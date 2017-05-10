/********************************************************************-*-c-*-*\
*               *                                                            *
*    #####      *  Giovanni Squillero, Ph.D.                                 *
*   ######      *  Politecnico di Torino - Dip. Automatica e Informatica     *
*   ###   \     *  Cso Duca degli Abruzzi 24 / I-10129 TORINO / ITALY        *
*    ##G  c\    *                                                            *
*    #     _\   *  Tel: +39-011564.7092  /  Fax: +39-011564.7099             *
*    |   _/     *  email: giovanni.squillero@polito.it                       *
*    |  _/      *  www  : http://www.cad.polito.it/staff/squillero/          *
*               *                                                            *
******************************************************************************
*
*   $Source: /home/squiller/tools/uGP/RCS/Fitness.c,v $
* $Revision: 1.11 $
*     $Date: 2004/03/29 16:17:11 $
*
******************************************************************************
*                                                                            *
*  Copyright (c) 2002-2003 Giovanni Squillero                                *
*                                                                            *
*  This  program  is   free  software;   you can  redistribute   it  and/or  *
*  modify  it under   the terms  of  the  GNU General   Public License   as  *
*  published by   the Free  Software  Foundation; either version  2 of  the  *
*  License,  or (at your option) any later version.                          *
*                                                                            *
*  This program  is distributed in the  hope   that it will be useful,  but  *
*  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty    of  *
*  MERCHANTABILITY  or  FITNESS FOR   A  PARTICULAR PURPOSE.   See  the GNU  *
*  General   PublicLicensefor  more details                                  *
*                                                                            *
\****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef LINUX
#include "gmacros.h"
#else
#include <macros.h>
#endif

#include "Check.h"
#include "Memory.h"
#include "Messages.h"
#include "Queue.h"
#include "Hash.h"
#include "String.h"
#include "InstructionLibrary.h"
#include "Graph.h"
#include "Genetic.h"
#include "Fitness.h"
#include "ugp.h"

#define CLONED_OUT "Cloned-Out"
#define TO_DO_LATER "To-do"

/*
 * INTERNAL PROTOS
 */
static G_INDIVIDUAL *ifitFoundValidClone(G_INDIVIDUAL * I, G_INDIVIDUAL ** iL);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/


static G_INDIVIDUAL *ifitFoundValidClone(G_INDIVIDUAL * I, G_INDIVIDUAL ** iL)
{
    G_INDIVIDUAL   *Clone;
    int             i;

    Clone = gGetFirstValidClone(I);
    if (Clone && Clone != I)
	return Clone;

    for (i = 0; iL[i]; ++i)
	if (!grCmpGraph(I->Graph, iL[i]->Graph))
	    return iL[i];

    return NULL;
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             fitEvaluate(G_GENETIC * GEN, G_INDIVIDUAL ** iList)
{
    char           *_FunctionName = "fitEvaluate";
    QUEUE           EvaluationBatch;
    G_INDIVIDUAL   *Ind;
    char            tmp[TMP_SIZE];
    char            candidate[TMP_SIZE];
    char            candidate2[TMP_SIZE];
    G_INDIVIDUAL   *Clone;
    FILE           *P;
    FILE           *F;
    int             p;
    int             f;
    int             i, j;
    int             batch, proc, tot_proc;
    static char     env[2][TMP_SIZE];
    int             done;

    /*
     * set up environment
     */
    EvaluationBatch = qInitQueue();
    for (i = 0; iList[i]; ++i) {
	CheckFalse(iList[i]->Info);
	grCheckGraph(iList[i]->Graph);
    }

    sprintf(env[0], "%u", GEN->Generations);
    setenv("UGP_GENERATION", env[0], 1);
    sprintf(env[1], "best_%s", GEN->FitnessFile);
    setenv("UGP_BEST_PROGRAM", env[1], 1);

    /*
     * dump whole population?
     */
    if (GEN->CompleteEvaluation) {
	for (p = 0; p < GEN->Mu; ++p) {
	    sprintf(tmp, "pop_individual_%d%s", p, GetExtension(GEN->FitnessFile));
	    CheckTrue(P = fopen(tmp, "w"));
	    grDumpGraph(GEN->Population.Individual[p].Graph, P, 0);
	    fclose(P);
	}
    }

    /*
     * prune todo list
     */
    if (GEN->CloneScaling == 0.0 || GEN->DeterministicFitness) {
	for (i = 0; iList[i]; ++i) {
	    if (iList[i]->Info)
		continue;
	    /* find clones in population */
	    if(Clone = gGetFirstValidClone(iList[i])) {
		msgMessage(MSG_VERBOSE, "Ind %d/%p is a clone of %d/%p (population)",
			   iList[i]->id, iList[i], Clone->id, Clone);
		iList[i]->Info = TO_DO_LATER;
		continue;
	    }
	    /* find clones in todo list */
	    for (j = 0; j < i; ++j) {
		if (!grCmpGraph(iList[i]->Graph, iList[j]->Graph)) {
		    msgMessage(MSG_VERBOSE, "Ind %d/%p is a clone of %d/%p (iList)",		    
			       iList[i]->id, iList[i], iList[j]->id, iList[j]);
		    iList[i]->Info = TO_DO_LATER;
		    continue;
		}
	    }
	}
    }

    /*
     * let's evaluate
     */
    done = 0;
    msgBarGraphStart(MSG_EXACT | MSG_INFO, GEN->Lambda, "Evaluation:");
    for (tot_proc = 0; iList[tot_proc]; tot_proc += proc) {
	*candidate = *candidate2 = 0;
	for (proc = 0, batch = 0; iList[tot_proc + proc] && batch < GEN->ParallelFitnessCalls;
	     ++proc) {
	    Ind = iList[tot_proc + proc];
	    if (Ind->Info) {
		msgBarGraphSpecial('-');
		msgMessage(MSG_VERBOSE, "Skipping ind %s", gDescribeIndividual(Ind));
	    } else {
		msgBarGraphSpecial('$');
		msgMessage(MSG_VERBOSE, "Scheduling ind %s", gDescribeIndividual(Ind));
		qEnqueueElement(EvaluationBatch, Ind);
		sprintf(tmp, "p%03d_%s", batch, GEN->FitnessFile);
		CheckTrue(P = fopen(tmp, "w"));
		grDumpGraph(Ind->Graph, P, GEN->DumpOptions);
		fclose(P);
		strcat(candidate, " ");
		strcat(candidate, tmp);
		strcat(candidate2, ":");
		strcat(candidate2, tmp);
		++batch;
	    }
	}

	if (qQueueEmpty(EvaluationBatch))
	    continue;
	setenv("UGP_PROGRAM", candidate2 + 1, 1);
	sprintf(tmp, "%s%s", GEN->FitnessProgram, candidate);
	CheckTrue(strlen(tmp) < TMP_SIZE);
	unlink(GEN->FitnessOut);
	system(tmp);

	CheckTrue(F = fopen(GEN->FitnessOut, "r"));
	while (!qQueueEmpty(EvaluationBatch)) {
	    Ind = qDequeueElement(EvaluationBatch);
	    for (f = 0; f < GEN->NumFitnesses; ++f)
		CheckTrue(fscanf(F, "%lf", &Ind->Fitness[f]) == 1);
	    CheckTrue(fscanf(F, "%s", tmp) == 1);
	    Ind->Info = CheckStrdup(tmp);
	    msgMessage(MSG_VERBOSE,
		       "... ind %ld/%p f = %s", Ind->id, Ind, gDescribeIndividualFitness(Ind));
	}
	fclose(F);
    }
    msgBarGraphEnd();

    /*
     * fix-up clones
     */
    for (i = 0; iList[i]; ++i) {
	if (iList[i]->Info != TO_DO_LATER)
	    continue;
	Clone = ifitFoundValidClone(iList[i], iList);
	CheckTrue(Clone);
	CheckTrue(Clone->Info != TO_DO_LATER);
	for (f = 0; f < GEN->NumFitnesses; ++f)
	    iList[i]->Fitness[f] = Clone->Fitness[f];
	iList[i]->Info = CheckStrdup(Clone->Info);
	msgMessage(MSG_VERBOSE | MSG_NOTIME,
		   "Individual %s\n\tfitness set to: %s",
		   gDescribeIndividual(iList[i]),
		   gDescribeIndividualFitness(iList[i]));
    }

    /* 
     * remove whole pop
     */
    if (GEN->CompleteEvaluation) {
	for (p = 0; p < GEN->Mu; ++p) {
	    sprintf(tmp, "pop_individual_%d%s", p, GetExtension(GEN->FitnessFile));
	    unlink(tmp);
	}
    }

    /*
     * that's all folks...
     */
    return 1;
}
