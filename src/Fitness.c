/********************************************************************-*-c-*-*\
*               *                                                            *
*    #####      *  Giovanni Squillero, Ph.D.                                 *
*   ######      *  Politecnico di Torino - Dip. Automatica e Informatica     *
*   ###   \     *  Cso Duca degli Abruzzi 24 / I-10129 TORINO / ITALY        *
*    ##G  c\    *                                                            *
*    #     _\   *  Tel: +39-011564.7186  /  Fax: +39-011564.7099             *
*    |   _/     *  email: giovanni.squillero@polito.it                       *
*    |  _/      *  www  : http://www.cad.polito.it/staff/squillero/          *
*               *                                                            *
******************************************************************************
*
*   $Source: /home/squiller/tools/uGP/RCS/Fitness.c,v $
* $Revision: 1.8 $
*     $Date: 2004/02/26 15:27:39 $
*
******************************************************************************
*                                                                            *
*  Copyright (c) 2002-2006 Giovanni Squillero                                *
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
#include "Hash.h"
#include "String.h"
#include "InstructionLibrary.h"
#include "Graph.h"
#include "Genetic.h"
#include "Fitness.h"
#include "ugp.h"

/*
 * INTERNAL PROTOS
 */
static char    *GetExtension(char *filename);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             fitEvaluate(G_GENETIC * GEN, G_INDIVIDUAL ** iList)
{
    char           *_FunctionName = "fitEvaluate";
    char            tmp[TMP_SIZE];
    char            warrior[TMP_SIZE];
    FILE           *P;
    FILE           *F;
    int             p;
    int             f;
    int             i;
    static char     env[2][TMP_SIZE];

    sprintf(env[0], "%u", GEN->Generations);
    setenv("UGP_GENERATION", env[0], 1);

    sprintf(env[1], "best_%s", GEN->FitnessFile);
    setenv("UGP_BEST_PROGRAM", env[2], 1);

    setenv("UGP_PROGRAM", GEN->FitnessFile, 1);

    if (GEN->CompleteEvaluation) {
	for (p = 0; p < GEN->Mu; ++p) {
	    sprintf(tmp, "pop_individual_%d%s", p, GetExtension(GEN->FitnessFile));
	    CheckTrue(P = fopen(tmp, "w"));
	    grDumpGraph(GEN->Population.Individual[p].Graph, P, 0);
	    fclose(P);
	}
    }

    unlink(GEN->FitnessOut);

    warrior[0] = 0;
    for (i = 0; iList[i]; ++i) {
	grCheckGraph(iList[i]->Graph);
	sprintf(tmp, "w%03d_%s", i, GEN->FitnessFile);
	CheckTrue(P = fopen(tmp, "w"));
	grDumpGraph(iList[i]->Graph, P, GEN->DumpOptions);
	fclose(P);
	strcat(warrior, " ");
	strcat(warrior, tmp);
    }
    sprintf(tmp, "%s%s", GEN->FitnessProgram, warrior);
    system(tmp);

    CheckTrue(F = fopen(GEN->FitnessOut, "r"));
    for (i = 0; iList[i]; ++i) {
	for (f = 0; f < GEN->NumFitnesses; ++f)
	    CheckTrue(fscanf(F, "%lf", &iList[i]->Fitness[f]) == 1);
	CheckTrue(fscanf(F, "%s", tmp) == 1);
	iList[i]->Info = CheckStrdup(tmp);
    }
    fclose(F);

    if (GEN->CompleteEvaluation) {
	for (p = 0; p < GEN->Mu; ++p) {
	    sprintf(tmp, "pop_individual_%d%s", p, GetExtension(GEN->FitnessFile));
	    unlink(tmp);
	}
    }

    for (i = 0; iList[i]; ++i) {
	strcpy(tmp, " ");
	for (f = 0; f < GEN->NumFitnesses; ++f)
	    sprintf(&tmp[strlen(tmp)], "%0.3f ", iList[i]->Fitness[f]);
	CheckTrue(strlen(tmp) < TMP_SIZE);
	msgMessage(MSG_VERBOSE, "Individual %ld/%p -> {%s\"%s\" }",
		   iList[i]->id, iList[i], tmp, iList[i]->Info);
    }
    return 1;
}

static char    *GetExtension(char *filename)
{
    char           *ext = ".s";
    int             t;

    for (t = 0; filename[t]; ++t)
	if (filename[t] == '.')
	    ext = &filename[t];
    return ext;
}
