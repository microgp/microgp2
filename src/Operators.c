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
*   $Source: /home/squiller/tools/uGP/RCS/Operators.c,v $
* $Revision: 1.12 $
*     $Date: 2003/12/02 13:29:16 $
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
#include <math.h>
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

#include "Heapsort.h"
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

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

int             opAddMutation(G_GENETIC * GEN)
{
    G_INDIVIDUAL   *New;
    GR_GRAPH       *F;
    int             parent;
    int             force;
    int             t;

    parent = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    New = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent]);
    gAddAncestor(New, GEN->Population.Individual[parent].id);
    F = New->Graph;

    force = 1;
    while (force || drand48() < GEN->Sigma) {
	force = 0;

	t = lrand48() % F->nSubgraphs;
	grAddRandomNodeToSubgraph(F->Subgraph[t], GEN->IL);
	msgMessage(MSG_DEBUG, "opAdd::Note: added node to %s", grDebugSubgraphId(F->Subgraph[t]));
    }

    gAddIndividualToPopulation(&GEN->Population, New);
    gFreeIndividual(New);
    return 1;
}

int             opSubMutation(G_GENETIC * GEN)
{
    G_INDIVIDUAL   *New;
    GR_GRAPH       *F;
    GR_NODE        *N;
    int             parent;
    int             force;
    int             t;

    parent = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    New = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent]);
    gAddAncestor(New, GEN->Population.Individual[parent].id);
    F = New->Graph;

    force = 1;
    while (force || drand48() < GEN->Sigma) {
	force = 0;

	t = lrand48() % F->nSubgraphs;
	if (F->Subgraph[t]->nNodes == 2) {
	    msgMessage(MSG_DEBUG, "opSub::Note: failed (empty subgraph)");
	    continue;
	}
	N = grGetRandomInternalNode(F->Subgraph[t]);
	if (N->ReferenceCount > 0) {
	    msgMessage(MSG_DEBUG, "opSub::Note: failed (referenced node)");
	    continue;
	}
	msgMessage(MSG_DEBUG, "opSub::Note: removing node %d from %s", N->id,
		   grDebugSubgraphId(N->Subgraph));
	grRemoveNode(N);
    }

    gAddIndividualToPopulation(&GEN->Population, New);
    gFreeIndividual(New);
    return 1;
}

int             opModMutation(G_GENETIC * GEN)
{
    G_INDIVIDUAL   *New;
    GR_GRAPH       *F;
    GR_NODE        *N;
    int             parent;
    int             force;
    int             t;

    parent = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    New = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent]);
    gAddAncestor(New, GEN->Population.Individual[parent].id);
    F = New->Graph;

    force = 1;
    while (force || drand48() < GEN->Sigma) {
	force = 0;

	t = lrand48() % F->nSubgraphs;
	N = grGetRandomNode(F->Subgraph[t]);
	msgMessage(MSG_DEBUG, "opMod::Note: modifying node %d in %s", N->id,
		   grDebugSubgraphId(N->Subgraph));
	if (!N->Macro->nParameters)
	    continue;

	grRandomizeNodeParameter(N, lrand48() % N->Macro->nParameters);
    }

    gAddIndividualToPopulation(&GEN->Population, New);
    gFreeIndividual(New);
    return 1;
}

int             opMixMutation(G_GENETIC * GEN)
{
    G_INDIVIDUAL   *New;
    GR_GRAPH       *F;
    GR_NODE        *N;
    int             parent;
    int             force;
    int             t;

    parent = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    New = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent]);
    gAddAncestor(New, GEN->Population.Individual[parent].id);
    F = New->Graph;

    force = 1;
    while (force || drand48() < GEN->Sigma) {
	force = 0;

	t = lrand48() % F->nSubgraphs;
	if (drand48() < 0.5) {
	    if (F->Subgraph[t]->nNodes == 2) {
		msgMessage(MSG_DEBUG, "opMix::Note: failed (empty subgraph)");
		continue;
	    }
	    N = grGetRandomInternalNode(F->Subgraph[t]);
	    if (N->ReferenceCount > 0) {
		msgMessage(MSG_DEBUG, "opMix::Note: failed (referenced node)");
		continue;
	    }
	    msgMessage(MSG_DEBUG, "opMix::Note: removing node %d from %s", N->id,
		       grDebugSubgraphId(N->Subgraph));
	    grRemoveNode(N);
	} else {
	    grAddRandomNodeToSubgraph(F->Subgraph[t], GEN->IL);
	    msgMessage(MSG_DEBUG, "opMix::Note: added node to %s",
		       grDebugSubgraphId(F->Subgraph[t]));
	}
    }

    gAddIndividualToPopulation(&GEN->Population, New);
    gFreeIndividual(New);
    return 1;
}

int             opStd2PointXOver(G_GENETIC * GEN)
{
    G_INDIVIDUAL   *Ind1;
    int             parent1;
    GR_SLICE        slice1;
    GR_SUBGRAPH    *Subgraph1;

    G_INDIVIDUAL   *Ind2;
    int             parent2;
    GR_SLICE        slice2;
    GR_SUBGRAPH    *Subgraph2;

    int             t;
    int             result = 0;

    /* copy parent 1 */
    parent1 = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    Ind1 = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent1]);
    if (msgCheckLevel(MSG_DEBUG)) {
	FILE           *F;

	F = fopen("parent1.deb", "w");
	grDumpGraph(Ind1->Graph, F, 0);
	grDumpGraph(Ind1->Graph, F, 1);
	fclose(F);
    }
    /* copy parent 2 */
    parent2 = gTournament(GEN->Population.Individual, GEN->Mu, 2);
    Ind2 = gDuplicateIndividual(GEN, &GEN->Population.Individual[parent2]);
    if (msgCheckLevel(MSG_DEBUG)) {
	FILE           *F;

	F = fopen("parent2.deb", "w");
	grDumpGraph(Ind2->Graph, F, 0);
	grDumpGraph(Ind2->Graph, F, 1);
	fclose(F);
    }
    gAddAncestor(Ind1, GEN->Population.Individual[parent1].id);
    gAddAncestor(Ind1, GEN->Population.Individual[parent2].id);
    gAddAncestor(Ind2, GEN->Population.Individual[parent1].id);
    gAddAncestor(Ind2, GEN->Population.Individual[parent2].id);

    /* get a random slice from parent 1 */
    t = lrand48() % Ind1->Graph->nSubgraphs;
    msgMessage(MSG_DEBUG,
	       "opStd2PointXOver::Debug: Selected subgraph %d from individual 1 (section: %d)", t,
	       Ind1->Graph->Subgraph[t]->Section);
    Subgraph1 = Ind1->Graph->Subgraph[t];
    slice1.Cut1 = grGetRandomNode(Subgraph1);
    slice1.Cut2 = grGetRandomNode(Subgraph1);

    grValidateSlice(&slice1);
    if (slice1.Cut1->Prev == NULL && slice1.Cut2->Succ == NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Can't slice the whole subgraph");
	goto cleanup;		/* can't slice the whole subgraph! */
    }

    /* get a compatible slice from parent 2 */
    Subgraph2 = grGetRandomCompatibleSubgraph(Ind2->Graph, slice1.Cut1->Subgraph);
    if (!Subgraph2) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Can't find a compatible subgraph");
	goto cleanup;
    }

    if (slice1.Cut1->Prev == NULL) {
	slice2.Cut1 = Subgraph2->Head;
	slice2.Cut2 = grGetRandomNode(Subgraph2);
    } else if (slice1.Cut2->Succ == NULL) {
	slice2.Cut1 = grGetRandomNode(Subgraph2);
	slice2.Cut2 = Subgraph2->Tail;
    } else {
	slice2.Cut1 = grGetRandomNode(Subgraph2);
	slice2.Cut2 = grGetRandomNode(Subgraph2);
    }

    grValidateSlice(&slice2);

    if (slice2.Cut1->Prev == NULL && slice2.Cut2->Succ == NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Incompatible slices");
	goto cleanup;		/* can't slice the whole subgraph! */
    }
    if (slice1.Cut1->Prev == NULL && slice2.Cut1->Prev != NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Incompatible slices");
	goto cleanup;		/* incompatible */
    }
    if (slice2.Cut1->Prev == NULL && slice1.Cut1->Prev != NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Incompatible slices");
	goto cleanup;		/* incompatible */
    }
    if (slice1.Cut2->Succ == NULL && slice2.Cut2->Succ != NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Incompatible slices");
	goto cleanup;		/* incompatible */
    }
    if (slice2.Cut2->Succ == NULL && slice1.Cut2->Succ != NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Incompatible slices");
	goto cleanup;		/* incompatible */
    }

    msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Got compatible slices");

    /* swap slices */
    if (slice1.Cut1->Prev == NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Swapping first half");
	grSwapNodes(slice1.Cut2->Succ, slice2.Cut2->Succ);
    } else if (slice1.Cut2->Succ == NULL) {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Swapping second half");
	grSwapNodes(slice1.Cut1, slice2.Cut1);
    } else {
	msgMessage(MSG_DEBUG, "opStd2PointXOver::Debug: Swapping internal slice");
	grSwapNodes(slice1.Cut1, slice2.Cut1);
	grSwapNodes(slice1.Cut2->Succ, slice2.Cut2->Succ);
    }

    grFixGraph(Ind1->Graph);
    grFixGraph(Ind2->Graph);

    gAddIndividualToPopulation(&GEN->Population, Ind1);
    gAddIndividualToPopulation(&GEN->Population, Ind2);
    result = 1;
  cleanup:
    gFreeIndividual(Ind1);
    gFreeIndividual(Ind2);
    return result;
}
