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
*   $Source: /home/squiller/tools/uGP/RCS/Genetic.c,v $
* $Revision: 1.13 $
*     $Date: 2004/02/26 15:20:43 $
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
#include <signal.h>
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
#include "Operators.h"
#include "ugp.h"

/*
 * INTERNAL PROTOS
 */
int             igEvalOffsprint(G_INDIVIDUAL * I);
G_INDIVIDUAL   *igGetIndividualFromId(long id);
int             igGetOperatorIndex(int (*func) (G_GENETIC *));
double          igAutoAdapt(double oldval, double newval);
static int      igCompareSort(const void *x1, const void *x2);
static int      igSelectOperator(void);
static int      igUpdateProbabilities(void);
static void     igDumpCsv(void);
static void     igDumpPopulation(G_GENETIC * GEN);
static void     igFreeOperator(G_OPERATOR * O);
static void     igInitializePopulation(G_GENETIC * GEN);
static void     igResizePopulation(G_POPULATION * P, int newSize);
static void     igSigHandler(int sig);
void            igSaveIndividualToFile(G_INDIVIDUAL * I, FILE * Fout);
void            igSavePopulationToFile(G_POPULATION * P, FILE * Fout);
int             igLoadPopulationFromFile(G_POPULATION * P, FILE * Fin);
G_INDIVIDUAL   *igLoadIndividualFromFile(FILE * Fin);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

static G_GENETIC *LocalGEN;
static int      _SIGINT_CAUGHT = 0;

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

/*
 * REVERSE igCompare and consider age (for Heapsort)
 */
static int      igCompareSort(const void *x1, const void *x2)
{
    G_INDIVIDUAL   *I1 = (G_INDIVIDUAL *) x1;
    G_INDIVIDUAL   *I2 = (G_INDIVIDUAL *) x2;

    if (gCompare(x1, x2) != 0) {
	return gCompare(x2, x1);	/* SWAPPED!!!! */
    } else {
	return I2->Birth - I1->Birth;
    }
}

int             gDeclareOperator(G_GENETIC * GEN,
				 int (*Function) (G_GENETIC *), char *description,
				 char symbol, int p_curr, int p_min, int p_max)
{
    GEN->Operator = CheckRealloc(GEN->Operator, (GEN->nOperators + 1) * sizeof(G_OPERATOR));
    GEN->Operator[GEN->nOperators].Function = Function;
    GEN->Operator[GEN->nOperators].Description = CheckStrdup(description);
    GEN->Operator[GEN->nOperators].Symbol = symbol;
    GEN->Operator[GEN->nOperators].pCurrent = p_curr;
    GEN->Operator[GEN->nOperators].pMax = p_max;
    GEN->Operator[GEN->nOperators].pMin = p_min;

    GEN->Operator[GEN->nOperators].statActivation = NULL;
    GEN->Operator[GEN->nOperators].statSuccess = NULL;

    msgMessage(MSG_DEBUG, "gDeclareOperator::Note: Added operator \"%s\" (%c)", description,
	       symbol);

    ++GEN->nOperators;
    return GEN->nOperators;
}

static void     igDumpCsv(void)
{
    char           *_FunctionName = "igDumpCsv";
    FILE           *CSV;
    int             o;

    if (!LocalGEN->CsvFile)
	return;

    if (!LocalGEN->Generations) {
	CheckTrue(CSV = fopen(LocalGEN->CsvFile, "w"));
	fprintf(CSV,
		"GEN" CSV_SEPARATOR "Sigma" CSV_SEPARATOR "Mu" CSV_SEPARATOR "Lambda"
		CSV_SEPARATOR);
	for (o = 0; o < LocalGEN->nOperators; ++o) {
	    fprintf(CSV, "%s" CSV_SEPARATOR, LocalGEN->Operator[o].Description);
	    fprintf(CSV, "%s!" CSV_SEPARATOR, LocalGEN->Operator[o].Description);
	}
	fprintf(CSV, "Best" CSV_SEPARATOR);
	fprintf(CSV, "Avg (mu)" CSV_SEPARATOR);
	fprintf(CSV, "Avg (lambda)" CSV_SEPARATOR);
	fprintf(CSV, "On-Line" CSV_SEPARATOR);
	fprintf(CSV, "Off-Line" CSV_SEPARATOR);

	fprintf(CSV, "\n");
	fclose(CSV);
    }

    CheckTrue(CSV = fopen(LocalGEN->CsvFile, "a"));
    fprintf(CSV, "%d" CSV_SEPARATOR "%f" CSV_SEPARATOR "%d" CSV_SEPARATOR "%d" CSV_SEPARATOR,
	    LocalGEN->Generations, LocalGEN->Sigma, LocalGEN->Mu, LocalGEN->Lambda);
    for (o = 0; o < LocalGEN->nOperators; ++o) {
	fprintf(CSV, "%d" CSV_SEPARATOR,
		LocalGEN->Operator[o].statActivation ? LocalGEN->
		Operator[o].statActivation[LocalGEN->Generations] : 0);
	fprintf(CSV, "%d" CSV_SEPARATOR,
		LocalGEN->Operator[o].statSuccess ? LocalGEN->Operator[o].
		statSuccess[LocalGEN->Generations] : 0);
    }

    fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->statBestFit[LocalGEN->Generations]);
    fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->statAvgFitnessMu[LocalGEN->Generations]);
    fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->statAvgFitnessLambda[LocalGEN->Generations]);
    fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->statOnLine / LocalGEN->statOnLine_num);
    fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->statOffLine / LocalGEN->statOffLine_num);

    for (o = 0; o < LocalGEN->Mu + LocalGEN->Lambda; ++o)
	fprintf(CSV, "%f" CSV_SEPARATOR, LocalGEN->Population.Individual[o].Fitness[0]);


    fprintf(CSV, "\n");
    fclose(CSV);
}

static void     igDumpPopulation(G_GENETIC * GEN)
{
    char           *_FunctionName = "igDumpPopulation";
    int             i;
    int             s;
    int             f;
    char            tmp[TMP_SIZE];
    char            tmp2[TMP_SIZE];

    msgMessage(MSG_VERBOSE | MSG_NOTIME, "");
    for (i = 0; i < GEN->Population.Size; ++i) {
	if (i == GEN->Mu)
	    msgMessage(MSG_VERBOSE | MSG_NOTIME, "");

	sprintf(tmp, "%d", GEN->Population.Individual[i].Graph->Subgraph[0]->nNodes);
	for (s = 1; s < GEN->Population.Individual[i].Graph->nSubgraphs; ++s)
	    sprintf(&tmp[strlen(tmp)], "/%d",
		    GEN->Population.Individual[i].Graph->Subgraph[s]->nNodes);
	CheckTrue(strlen(tmp) < TMP_SIZE);

	strcpy(tmp2, " ");
	for (f = 0; f < GEN->NumFitnesses; ++f)
	    sprintf(&tmp2[strlen(tmp2)], "%0.2f ", GEN->Population.Individual[i].Fitness[f]);
	CheckTrue(strlen(tmp2) < TMP_SIZE);

	msgMessage(MSG_VERBOSE | MSG_NOTIME,
		   "%02d) Individual %ld, b:%d, sg:%s, f = {%s\"%s\" }", i + 1,
		   GEN->Population.Individual[i].id,
		   GEN->Population.Individual[i].Birth, tmp, tmp2,
		   GEN->Population.Individual[i].Info);
    }
    msgMessage(MSG_VERBOSE | MSG_NOTIME, "");

    if (GEN->Population.Size > GEN->Mu) {
	msgMessage(MSG_EXACT | MSG_INFO,
		   "POP: [ %0.3f -- %0.3f || %0.3f -- %0.3f ]",
		   *GEN->Population.Individual[0].Fitness,
		   *GEN->Population.Individual[GEN->Mu - 1].Fitness,
		   *GEN->Population.Individual[GEN->Mu].Fitness,
		   *GEN->Population.Individual[GEN->Population.Size - 1].Fitness);
    } else {
	msgMessage(MSG_EXACT | MSG_INFO,
		   "POP: [ %0.3f -- %0.3f || na/na -- na/na ]",
		   *GEN->Population.Individual[0].Fitness,
		   *GEN->Population.Individual[GEN->Mu - 1].Fitness);
    }

    sprintf(tmp, "%d", GEN->Population.Individual[0].Graph->Subgraph[0]->nNodes);
    for (s = 1; s < GEN->Population.Individual[0].Graph->nSubgraphs; ++s)
	sprintf(&tmp[strlen(tmp)], "/%d", GEN->Population.Individual[0].Graph->Subgraph[s]->nNodes);

    strcpy(tmp2, " ");
    for (f = 0; f < GEN->NumFitnesses; ++f)
	sprintf(&tmp2[strlen(tmp2)], "%0.2f ", GEN->Population.Individual[0].Fitness[f]);
    CheckTrue(strlen(tmp2) < TMP_SIZE);

    msgMessage(MSG_EXACT | MSG_INFO,
	       "Best: %ld, b: %d, sg: %s, f = {%s \"%s\" }",
	       GEN->Population.Individual[0].id,
	       GEN->Population.Individual[0].Birth,
	       tmp, tmp2,
	       GEN->Population.Individual[0].Info ? GEN->Population.Individual[0].Info : "(null)");
}

int             gTournament(G_INDIVIDUAL * Candidate, int nCandidates, int tau)
{
    int             champion;
    int             t;
    int             r;

    champion = lrand48() % nCandidates;
    for (t = 1; t < tau; ++t) {
	r = lrand48() % nCandidates;
	if (*Candidate[r].Fitness > *Candidate[champion].Fitness)
	    champion = r;
    }
    return champion;
}

static void     igInitializePopulation(G_GENETIC * GEN)
{
    char           *_FunctionName = "igInitializePopulation";
    int             p, f;
    int             t;
    double          totfit1, totfit2;
    FILE           *F;
    char            tmp[TMP_SIZE];
    G_INDIVIDUAL   **I;

    for (t = 0; GEN->InitialPopulation[t]; ++t) {
	msgMessage(MSG_INFO, "Reading population from file \"%s\"", GEN->InitialPopulation[t]);
	CheckTrue(F = fopen(GEN->InitialPopulation[t], "r"));
	igLoadPopulationFromFile(&GEN->Population, F);
	fclose(F);
    }
    p = GEN->Population.Size;

    /* fillup population (if needed) */
    if (GEN->Mu > GEN->Population.Size) {
	igResizePopulation(&GEN->Population, GEN->Mu);
	for (; p < GEN->Mu; ++p) {
	    gInitializeIndividual(GEN, &GEN->Population.Individual[p]);

	    grAddEmptySubgraph(GEN->Population.Individual[p].Graph, LocalGEN->IL, 0);
	    for (t = 0; t < GEN->InitNodes; ++t) {
/* 		grAddRandomNode(GEN->Population.Individual[p].Graph, LocalGEN->IL); */
		grAddRandomNodeToSubgraph(*GEN->Population.Individual[p].Graph->Subgraph, LocalGEN->IL);
	    }
	    /*
	     * randomize local and global prologue/epilogue
	     */
	    grRandomizeNode(GEN->Population.Individual[p].Graph->Subgraph[0]->Head);
	    grRandomizeNode(GEN->Population.Individual[p].Graph->Subgraph[0]->Tail);
	}
    }

    msgBarGraphStart(MSG_EXACT | MSG_INFO, GEN->Population.Size, "Evaluating initial population");
    I = CheckCalloc(1+GEN->ParallelFitnessCalls, sizeof(G_INDIVIDUAL *));
    for (p = 0; p < GEN->Population.Size; p += GEN->ParallelFitnessCalls) {
	for(f=0; f < GEN->ParallelFitnessCalls && f+p < GEN->Population.Size; ++f) {
	    I[f] = &GEN->Population.Individual[f+p];
	    msgBarGraph(p);
	}
	fitEvaluate(GEN, I);
    }
    msgBarGraphEnd();

    /* sort and discard worst individuals */
    heapsort(GEN->Population.Individual, GEN->Population.Size,
	     sizeof(G_INDIVIDUAL), &igCompareSort);
    igResizePopulation(&GEN->Population, GEN->Mu);

    GEN->statBestFit = CheckCalloc(1, sizeof(double));
    GEN->statAvgFitnessMu = CheckCalloc(1, sizeof(double));
    GEN->statAvgFitnessLambda = CheckCalloc(1, sizeof(double));

    for (totfit1 = 0.0, t = 0; t < min(GEN->Mu, GEN->Population.Size); ++t)
	totfit1 += *GEN->Population.Individual[t].Fitness;
    for (totfit2 = 0.0; t < min(GEN->Mu + GEN->Lambda, GEN->Population.Size); ++t)
	totfit2 += *GEN->Population.Individual[t].Fitness;

    GEN->statAvgFitnessMu[0] = totfit1 / GEN->Mu;
    GEN->statAvgFitnessLambda[0] = totfit2 / GEN->Lambda;
    GEN->statBestFit[0] = *GEN->Population.Individual[0].Fitness;
    GEN->statOnLine += totfit2;
    GEN->statOnLine_num += GEN->Lambda;
    GEN->statOffLine += *GEN->Population.Individual[0].Fitness;
    GEN->statOffLine_num += 1;

    igDumpCsv();

    /* 
     * write bestprog to file 
     */
    sprintf(tmp, "best_%s", GEN->FitnessFile);
    CheckTrue(F = fopen(tmp, "w"));

    grDumpGraph(GEN->Population.Individual[0].Graph, F, GEN->DumpOptions);
    fclose(F);
}

static void     igPerformEvolution(G_GENETIC * GEN)
{
    char           *_FunctionName = "igPerformEvolution";
    int             op, off, f;
    FILE           *B;
    double          totfit1, totfit2;
    int             t;
    int             status;
    char            tmp[TMP_SIZE];
    int             IdleCount;
    int             ContinueEvolution;
    int             ind;
    G_INDIVIDUAL  **I;

    double          __OldBest = 0;

    GEN->Generations = 0;
    IdleCount = 0;

    igDumpCsv();
    /* 
     * crash recovery
     */
    msgMessage(MSG_VERBOSE, "igPerformEvolution::Preliminary crash recovery");
    sprintf(tmp, "%s.%d", GEN->CrashRecoveryDump, GEN->Generations);
    CheckTrue(B = fopen(tmp, "w"));
    igSavePopulationToFile(&GEN->Population, B);
    fclose(B);

    ContinueEvolution = 1;
    if (GEN->MaxGenerations <= 0)
	ContinueEvolution = 0;

    while (ContinueEvolution) {

	__OldBest = *GEN->Population.Individual[0].Fitness;

	++GEN->Generations;

	msgMessage(MSG_VERBOSE, "");
	msgMessage(MSG_VERBOSE, "igPerformEvolution::Generation %d:", GEN->Generations);

	/*
	 * malloc statz
	 */
	GEN->statSuccess = CheckRealloc(GEN->statSuccess, sizeof(int) * (GEN->Generations + 1));
	GEN->statAvgFitnessMu =
	    CheckRealloc(GEN->statAvgFitnessMu, sizeof(double) * (GEN->Generations + 1));
	GEN->statAvgFitnessLambda =
	    CheckRealloc(GEN->statAvgFitnessLambda, sizeof(double) * (GEN->Generations + 1));

	GEN->statBestFit = CheckRealloc(GEN->statBestFit, sizeof(double) * (GEN->Generations + 1));
	GEN->statAvgFitnessMu =
	    CheckRealloc(GEN->statAvgFitnessMu, sizeof(double) * (GEN->Generations + 1));
	GEN->statAvgFitnessLambda =
	    CheckRealloc(GEN->statAvgFitnessLambda, sizeof(double) * (GEN->Generations + 1));

	/* 
	 * clear statz! 
	 */
	GEN->statSuccess[GEN->Generations] = 0;
	for (op = 0; op < GEN->nOperators; ++op) {
	    GEN->Operator[op].statActivation =
		CheckRealloc(GEN->Operator[op].statActivation,

			     sizeof(int) * (GEN->Generations + 1));
	    GEN->Operator[op].statSuccess =
		CheckRealloc(GEN->Operator[op].statSuccess, sizeof(int) * (GEN->Generations + 1));

	    GEN->Operator[op].statActivation[GEN->Generations] = 0;
	    GEN->Operator[op].statSuccess[GEN->Generations] = 0;
	}

	/* discard worst Lambda individuals */
	igResizePopulation(&GEN->Population, GEN->Mu);
	msgMessage(MSG_INFO, "");
	msgMessage(MSG_INFO, "*** Generation %d", GEN->Generations);

	msgBarGraphStart(MSG_EXACT | MSG_INFO, GEN->Lambda, "Generation:");
	for (off = 0; off < GEN->Lambda; ++off) {
	    op = igSelectOperator();
	    status = (GEN->Operator[op].Function) (GEN);
	    if (status) {
		msgBarGraphSpecial(tolower(GEN->Operator[op].Symbol));
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Operator %s (success)",
			   GEN->Operator[op].Description);
	    } else {
		msgBarGraphSpecial(toupper(GEN->Operator[op].Symbol));
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Operator %s (failure)",
			   GEN->Operator[op].Description);
	    }
	    /*
	     * Set the 'LastOperatorUsed' field to all new Individuals 
	     * (ie. where LastOperatorUsed == -1)
	     */
	    for (ind = 0; ind < GEN->Population.Size; ++ind)
		if (GEN->Population.Individual[ind].LastOperatorUsed == -1)
		    GEN->Population.Individual[ind].LastOperatorUsed = op;
	}
	msgBarGraphEnd();

	/* Now evaluate the population... */
	I = CheckCalloc(1+GEN->ParallelFitnessCalls, 
			sizeof(G_INDIVIDUAL *));

	msgBarGraphStart(MSG_EXACT | MSG_INFO, GEN->Lambda, "Evaluation:");
	for (ind = GEN->Mu; ind < GEN->Population.Size; ind += GEN->ParallelFitnessCalls) {
	    for(f=0; f < GEN->ParallelFitnessCalls 
		    && ind+f < GEN->Population.Size; ++f) {
		I[f] = &GEN->Population.Individual[ind+f];
		msgBarGraphSpecial('$');
	    }
	    fitEvaluate(GEN, I);
	}
	msgBarGraphEnd();
	

	msgBarGraphStart(MSG_EXACT | MSG_INFO, GEN->Lambda, "Analysis  :");
	for (ind = GEN->Mu; ind < GEN->Population.Size; ++ind) {
	    status = igEvalOffsprint(&GEN->Population.Individual[ind]);
	    op = GEN->Population.Individual[ind].LastOperatorUsed;

	    /* operator statz */
	    ++GEN->Operator[op].statActivation[GEN->Generations];
	    if (status >= EVOLUTIONARY_PARTIAL_SUCCESS) {
		++GEN->statSuccess[GEN->Generations];
		++GEN->Operator[op].statSuccess[GEN->Generations];
	    }

	    /* print correct symbol */
	    if (status == EVOLUTIONARY_MIRACLE) {
		msgBarGraphSpecial('!');
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Whoa!!!");
		IdleCount = 0;
	    } else if (status == EVOLUTIONARY_COMPLETE_SUCCESS) {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Success");
		msgBarGraphSpecial('+');
		IdleCount = 0;
	    } else if (status == EVOLUTIONARY_PARTIAL_SUCCESS) {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Almost success");
		msgBarGraphSpecial('>');
	    } else if (status == EVOLUTIONARY_PARTIAL_FAILURE) {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Almost failure");
		msgBarGraphSpecial('<');
		++IdleCount;
	    } else if (status == EVOLUTIONARY_COMPLETE_FAILURE) {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Failure");
		msgBarGraphSpecial('-');
		++IdleCount;
	    } else if (status == EVOLUTIONARY_DISASTER) {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Yeuch!!!");
		msgBarGraphSpecial('?');
		++IdleCount;
	    } else {
		msgMessage(MSG_VERBOSE, "igPerformEvolution::Note: Unknown result");
		msgBarGraphSpecial('#');
	    }

	    if (_SIGINT_CAUGHT) {
		signal(SIGINT, &igSigHandler);
		_SIGINT_CAUGHT = 0;
		msgMessage(MSG_FORCESHOW, "Caught SIGINT...");
		exit(32);
	    }
	}
	msgBarGraphEnd();

	/* 
	 * sort new population 
	 */
	CheckTrue(__OldBest == *GEN->Population.Individual[0].Fitness);
	heapsort(GEN->Population.Individual,
		 GEN->Population.Size, sizeof(G_INDIVIDUAL), &igCompareSort);
	igDumpPopulation(GEN);
	CheckTrue(__OldBest <= *GEN->Population.Individual[0].Fitness);

	/*
	 * update statz
	 */
	for (totfit1 = 0.0, t = 0; t < GEN->Mu; ++t)
	    totfit1 += *GEN->Population.Individual[t].Fitness;
	for (totfit2 = 0.0; t < GEN->Population.Size; ++t)
	    totfit2 += *GEN->Population.Individual[t].Fitness;

	GEN->statAvgFitnessMu[GEN->Generations] = totfit1 / GEN->Mu;
	GEN->statAvgFitnessLambda[GEN->Generations] = totfit2 / GEN->Lambda;
	GEN->statBestFit[GEN->Generations] = *GEN->Population.Individual[0].Fitness;
	GEN->statOnLine += totfit2;
	GEN->statOnLine_num += GEN->Lambda;
	GEN->statOffLine += *GEN->Population.Individual[0].Fitness;
	GEN->statOffLine_num += 1;

	igUpdateProbabilities();
	igDumpCsv();

	/* 
	 * write bestprog to file 
	 */
	sprintf(tmp, "best_%s", GEN->FitnessFile);
	CheckTrue(B = fopen(tmp, "w"));
	grDumpGraph(GEN->Population.Individual[0].Graph, B, GEN->DumpOptions);
	fclose(B);

	/* 
	 * crash recovery
	 */
	sprintf(tmp, "%s.%d", GEN->CrashRecoveryDump, GEN->Generations);
	CheckTrue(B = fopen(tmp, "w"));
	igSavePopulationToFile(&GEN->Population, B);
	fclose(B);
	sprintf(tmp, "%s.%d", GEN->CrashRecoveryDump, GEN->Generations - 1);
	unlink(tmp);

	/*
	 * continue?
	 */
	if (GEN->Generations >= GEN->MaxGenerations) {
	    msgMessage(MSG_INFO, "Maximum number of generations reached");
	    ContinueEvolution = 0;
	} else if (GEN->IdleLimit > 0 && IdleCount > GEN->IdleLimit) {
	    msgMessage(MSG_INFO, "Steady state detected (idle=%d)", IdleCount);
	    ContinueEvolution = 0;
	} else if (GEN->FitnessLimit > 0
		   && *GEN->Population.Individual[0].Fitness >= GEN->FitnessLimit) {
	    msgMessage(MSG_INFO, "Fitness treshold reached (fit=%f)",
		       GEN->Population.Individual[0].Fitness);
	    ContinueEvolution = 0;
	}
    }
}


static int      igSelectOperator(void)
{
    char           *_FunctionName = "igSelectOperator";
    int             sum;
    int             o;
    int             choice;

    for (sum = 0, o = 0; o < LocalGEN->nOperators; ++o)
	sum += LocalGEN->Operator[o].pCurrent;
    choice = lrand48() % sum;
    for (o = 0; choice >= 0; ++o)
	choice -= LocalGEN->Operator[o].pCurrent;
    --o;
    CheckTrue(o < LocalGEN->nOperators);
    return o;
}

static void     igFreeOperator(G_OPERATOR * O)
{
    CheckFree(O->Description);
    CheckFree(O->statActivation);
    CheckFree(O->statSuccess);
}

double          igAutoAdapt(double oldval, double newval)
{
    if (LocalGEN->Inertia <= 0)
	return newval;
    if (LocalGEN->Inertia >= 1)
	return oldval;
    return oldval * LocalGEN->Inertia + newval * (1 - LocalGEN->Inertia);
}

static void     igSigHandler(int sig)
{
    ++_SIGINT_CAUGHT;
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             gEvolution(G_GENETIC * GEN)
{
    char 	warrior[TMP_SIZE];
    int w;

    LocalGEN = GEN;

    gDeclareOperator(GEN, opStd2PointXOver, "XVR2", 'x', 200, 10, 1000);
/*     gDeclareOperator(GEN, opSwapNodeXOver, "???", 'X', 100, 10, 1000);  */
    gDeclareOperator(GEN, opSubMutation, "Sub", 's', 100, 10, 1000);
    gDeclareOperator(GEN, opAddMutation, "Add", 'a', 200, 10, 1000);
    gDeclareOperator(GEN, opModMutation, "Mod", 'm', 100, 10, 1000);
    gDeclareOperator(GEN, opMixMutation, "MIX", 'i', 100, 10, 1000);

    msgMessage(MSG_INFO, "Evolving through a (%d+%d) strategy.  Auto-adaptation inertia: %0.3f.",
	       GEN->Mu, GEN->Lambda, GEN->Inertia);
    if (GEN->IdleLimit > 0) {
	msgMessage(MSG_INFO,
		   "%d initial random node%s.  %d generation%s.  Steady state detection: max idle = %d.",
		   GEN->InitNodes, GEN->InitNodes == 1 ? "" : "s", GEN->MaxGenerations,
		   GEN->MaxGenerations == 1 ? "" : "s", GEN->IdleLimit);
    } else {
	msgMessage(MSG_INFO,
		   "%d initial random node%s.  %d generation%s.  No steady state detection.",
		   GEN->InitNodes, GEN->InitNodes == 1 ? "" : "s", GEN->MaxGenerations,
		   GEN->MaxGenerations == 1 ? "" : "s");
    }
    if (GEN->CsvFile)
	msgMessage(MSG_INFO, "Dumping evolutioon stats to \"%s\".", GEN->CsvFile);
    if(GEN->ParallelFitnessCalls==1) {
	msgMessage(MSG_INFO, "Fitness call: \"%s w000_%s\" -> \"%s\" (%d value%s)",
		   GEN->FitnessProgram, GEN->FitnessFile,
		   GEN->FitnessOut, GEN->NumFitnesses, GEN->NumFitnesses == 1 ? "" : "s");
    } else {
	warrior[0] = 0;
	for (w = 0; w < GEN->ParallelFitnessCalls; ++w)
	    sprintf(&warrior[strlen(warrior)], " w%03d_%s", w, GEN->FitnessFile);
	msgMessage(MSG_INFO, "Fitness call: \"%s%s\" -> \"%s\" (%dx%d values)",
		   GEN->FitnessProgram, warrior,
		   GEN->FitnessOut, 
		   GEN->ParallelFitnessCalls, 
		   GEN->NumFitnesses);
    }



    msgMessage(MSG_INFO, "");

/*     OldSigInt = signal(SIGINT, &igSigHandler); */
    _SIGINT_CAUGHT = 0;
    igInitializePopulation(GEN);
    igPerformEvolution(GEN);
/*     signal(SIGINT, OldSigInt); */

    return GEN->Generations;
}

/*
 * Returns > 0 when x1 is *BETTER* than x2
 * Returns < 0 when x1 is *WORSE* than x2
 */
int             gCompare(const void *x1, const void *x2)
{
    G_INDIVIDUAL   *I1 = (G_INDIVIDUAL *) x1;
    G_INDIVIDUAL   *I2 = (G_INDIVIDUAL *) x2;
    int             f;

    for (f = 0; f < LocalGEN->NumFitnesses; ++f) {
	if (I1->Fitness[f] > I2->Fitness[f]) {
	    return 1;
	} else if (I1->Fitness[f] < I2->Fitness[f]) {
	    return -1;
	}
    }
    return 0;
}

void            gFreeGEN(G_GENETIC * GEN)
{
    int             p;

    /*
     * free everything
     */
    for (p = 0; p < GEN->Population.Size; ++p) {
	gFreeIndividual(&GEN->Population.Individual[p]);
    }
    CheckFree(GEN->Population.Individual);
    for (p = 0; p < GEN->nOperators; ++p)
	igFreeOperator(&GEN->Operator[p]);
    CheckFree(GEN->Operator);
    CheckFree(GEN->statSuccess);

    CheckFree(GEN->statBestFit);
    CheckFree(GEN->statAvgFitnessMu);
    CheckFree(GEN->statAvgFitnessLambda);

    CheckFree(GEN->CsvFile);
}

static int      igUpdateProbabilities(void)
{
    int             i;
    double          tempf;
    int             tempi;
    char            tmp[256];

    /* SIGMA */
    sprintf(tmp, "Success rate: %0.2f%%, ",
	    100.0 * LocalGEN->statSuccess[LocalGEN->Generations] / LocalGEN->Lambda);
    tempf = min(1.0 * LocalGEN->statSuccess[LocalGEN->Generations] / LocalGEN->Lambda, 0.95);
    tempf = igAutoAdapt(LocalGEN->Sigma, tempf);
    msgMessage(MSG_VERBOSE, "Success rate %.2f%% (sigma: %0.3f -> %0.3f)",
	       100.0 * LocalGEN->statSuccess[LocalGEN->Generations] / LocalGEN->Lambda,
	       LocalGEN->Sigma, tempf);
    sprintf(&tmp[strlen(tmp)], "sigma: %0.3f%+0.3f ", LocalGEN->Sigma, tempf - LocalGEN->Sigma);
    LocalGEN->Sigma = tempf;

    /* MU */
    tempi = LocalGEN->Mu;
    sprintf(&tmp[strlen(tmp)], "mu: %d%+d, ", LocalGEN->Mu, tempi - LocalGEN->Mu);
    msgMessage(MSG_VERBOSE, "Success rate %.2f%% (mu: %d -> %d)",
	       100.0 * LocalGEN->statSuccess[LocalGEN->Generations] / LocalGEN->Lambda,
	       LocalGEN->Mu, tempi);
    LocalGEN->Mu = tempi;

    /* LAMBDA */
    tempi = LocalGEN->Lambda;
    sprintf(&tmp[strlen(tmp)], "lambda: %d%+d ", LocalGEN->Lambda, tempi - LocalGEN->Lambda);
    msgMessage(MSG_VERBOSE, "Success rate %.2f%% (lambda: %d -> %d)",
	       100.0 * LocalGEN->statSuccess[LocalGEN->Generations] / LocalGEN->Lambda,
	       LocalGEN->Lambda, tempi);
    LocalGEN->Lambda = tempi;

    msgMessage(MSG_EXACT | MSG_INFO, "%s", tmp);

    /* OPS */
    *tmp = 0;
    for (i = 0; i < LocalGEN->nOperators; ++i) {
	tempi = LocalGEN->Operator[i].pCurrent;
	if (LocalGEN->Operator[i].statActivation[LocalGEN->Generations] > 0)
	    tempi = rint(igAutoAdapt
			 (tempi,
			  1000.0 * LocalGEN->Operator[i].statSuccess[LocalGEN->Generations] /
			  LocalGEN->Operator[i].statActivation[LocalGEN->Generations]));
	else
	    tempi = rint(igAutoAdapt(tempi, LocalGEN->Operator[i].pMin));

	if (tempi > LocalGEN->Operator[i].pMax)
	    tempi = LocalGEN->Operator[i].pMax;
	if (tempi < LocalGEN->Operator[i].pMin)
	    tempi = LocalGEN->Operator[i].pMin;
	msgMessage(MSG_VERBOSE, "Operator %d \"%s\": %d/%d : %d -> %d", i,
		   LocalGEN->Operator[i].Description,
		   LocalGEN->Operator[i].statActivation[LocalGEN->Generations],
		   LocalGEN->Operator[i].statSuccess[LocalGEN->Generations],
		   LocalGEN->Operator[i].pCurrent, tempi);
	sprintf(&tmp[strlen(tmp)], "%c:%d%+d ",
		LocalGEN->Operator[i].Symbol,
		LocalGEN->Operator[i].pCurrent, tempi - LocalGEN->Operator[i].pCurrent);
	LocalGEN->Operator[i].pCurrent = tempi;
    }
    msgMessage(MSG_EXACT | MSG_INFO, "OPS: [ %s]", tmp);

    return 1;
}

static void     igResizePopulation(G_POPULATION * P, int newSize)
{
    int             p;

    P->Size = newSize;

    if (P->Size == P->Space) {
	/* do nuthin' */
    } else if (P->Size > P->Space) {
	P->Individual = CheckRealloc(P->Individual, P->Size * sizeof(G_INDIVIDUAL));
	for (p = P->Space; p < P->Size; ++p)
	    memset(&P->Individual[p], 0, sizeof(G_INDIVIDUAL));
	P->Space = P->Size;
    } else {
	msgMessage(MSG_DEBUG, "igResizePopulation::Debug: Population is shrinking");
	for (p = P->Size; p < P->Space; ++p) {
	    msgMessage(MSG_DEBUG, "igResizePopulation::Debug: Removing individual %d (id: %d)", p,
		       P->Individual[p].id);
	    if (P->Individual[p].id)
		gFreeIndividual(&P->Individual[p]);
	}
    }
}

int             gAddIndividualToPopulation(G_POPULATION * P, G_INDIVIDUAL * I)
{
    int             a;

#ifdef GS_DEBUG
    grCheckGraph(I->Graph);
#endif
    msgMessage(MSG_DEBUG, "gAddIndividualToPopulation::Debug: P->Size will be %d", P->Size + 1);
    igResizePopulation(P, P->Size + 1);
    memcpy(&P->Individual[P->Size - 1], I, sizeof(G_INDIVIDUAL));
    P->Individual[P->Size - 1].Ancestor = CheckMalloc(sizeof(long));

    *P->Individual[P->Size - 1].Ancestor = -1;
    P->Individual[P->Size - 1].Graph = grDuplicateGraph(I->Graph);

    a = -1;
    while (I->Ancestor[++a] != -1)
	gAddAncestor(&P->Individual[P->Size - 1], I->Ancestor[a]);
    if (I->Info)
	P->Individual[P->Size - 1].Info = CheckStrdup(I->Info);

    return P->Size;
}

void            gFreeIndividual(G_INDIVIDUAL * I)
{
    if (I->Graph)
	grFreeGraph(I->Graph);
    if (I->Info)
	CheckFree(I->Info);
    if (I->Ancestor)
	CheckFree(I->Ancestor);
    I->id = 0;
}

void            gInitializeIndividual(G_GENETIC * GEN, G_INDIVIDUAL * I)
{
    I->id = ++GEN->statIndividuals;
    I->Birth = GEN->Generations;
    I->Fitness = (double *)CheckCalloc(LocalGEN->NumFitnesses, sizeof(double));

    I->Info = NULL;
    I->Graph = (GR_GRAPH *) CheckCalloc(1, sizeof(GR_GRAPH));

    I->LastOperatorUsed = -1;
    I->Ancestor = CheckMalloc(sizeof(long));

    *I->Ancestor = -1;
}

void            igSavePopulationToFile(G_POPULATION * P, FILE * Fout)
{
    int             p;
    unsigned long   random = lrand48();

    fprintf(Fout, "%lu\n", random);
    fprintf(Fout, "%d\n", P->Size);
    fprintf(Fout, "%d\n", P->Space);
    for (p = 0; p < P->Size; ++p)
	igSaveIndividualToFile(&P->Individual[p], Fout);
    fprintf(Fout, "%lu\n", random);
}

int             igLoadPopulationFromFile(G_POPULATION * P, FILE * Fin)
{
    G_INDIVIDUAL   *I;
    unsigned long   seed, seed2;
    int             p;
    int             size, space;

    fscanf(Fin, "%lu", &seed);
    msgMessage(MSG_DEBUG, "igLoadPopulationFromFile::Debug: seed %lu", seed);
    fscanf(Fin, "%d", &size);
    fscanf(Fin, "%d", &space);
    for (p = 0; p < size; ++p) {
	I = igLoadIndividualFromFile(Fin);
	I->id = ++LocalGEN->statIndividuals;
	gAddIndividualToPopulation(P, I);
	gFreeIndividual(I);
    }
    fscanf(Fin, "%lu", &seed2);
    if (seed != seed2)
	msgMessage(MSG_ERROR, "igLoadPopulationFromFile::Error: File corrupted");
    msgMessage(MSG_DEBUG, "igLoadPopulationFromFile::Debug: Population read correctly");
    return P->Size;
}

void            igSaveIndividualToFile(G_INDIVIDUAL * I, FILE * Fout)
{
    unsigned long   random = lrand48();
    int             f;

    fprintf(Fout, "%lu\n", random);

    fprintf(Fout, "%lu\n", I->id);
    fprintf(Fout, "%d\n", I->Birth);
    fprintf(Fout, "%s\n", I->Info);
    for (f = 0; f < LocalGEN->NumFitnesses; ++f)
	fprintf(Fout, "%f ", I->Fitness[f]);
    fprintf(Fout, "\n");
    grSaveGraphToFile(I->Graph, Fout);
    fprintf(Fout, "%lu\n", random);
}

G_INDIVIDUAL   *igLoadIndividualFromFile(FILE * Fin)
{
    G_INDIVIDUAL   *I;
    unsigned long   seed, seed2;
    char            info[TMP_SIZE];
    int             f;

    I = CheckCalloc(1, sizeof(G_INDIVIDUAL));
    gInitializeIndividual(LocalGEN, I);

    fscanf(Fin, "%lu", &seed);
    msgMessage(MSG_DEBUG, "igLoadIndividualFromFile::Debug: seed %lu", seed);
    fscanf(Fin, "%lu", &I->id);
    fscanf(Fin, "%d", &I->Birth);
    fscanf(Fin, "%s", info);
    I->Info = CheckStrdup(info);
    for (f = 0; f < LocalGEN->NumFitnesses; ++f)
	fscanf(Fin, "%lf", &I->Fitness[f]);
    grLoadGraphFromFile(I->Graph, LocalGEN->IL, Fin);
    fscanf(Fin, "%lu", &seed2);
    if (seed != seed2)
	msgMessage(MSG_ERROR, "igLoadIndividualFromFile::Error: File corrupted");
    msgMessage(MSG_DEBUG, "igLoadIndividualFromFile::Debug: Population read correctly");
    return I;
}

G_INDIVIDUAL   *gDuplicateIndividual(G_GENETIC * GEN, G_INDIVIDUAL * I)
{
    G_INDIVIDUAL   *New;

    New = CheckCalloc(1, sizeof(G_INDIVIDUAL));
    gInitializeIndividual(GEN, New);
    New->Graph = grDuplicateGraph(I->Graph);

    return New;
}

G_INDIVIDUAL   *igGetIndividualFromId(long id)
{
    int             i;

    for (i = 0; i < LocalGEN->Mu; ++i)
	if (LocalGEN->Population.Individual[i].id == id)
	    return &LocalGEN->Population.Individual[i];
    return NULL;
}

int             gAddAncestor(G_INDIVIDUAL * I, long ancestor)
{
    int             a;

    msgMessage(MSG_DEBUG, "gAddAncestor::Debug: Adding ancestor %lu to individual %lu", ancestor,
	       I->id);
    if (!I->Ancestor) {
	msgMessage(MSG_WARNING, "gAddAncestor::Warning: Fixing NULL ancestor list");
	I->Ancestor = CheckMalloc(sizeof(long));

	*I->Ancestor = -1;
    }

    for (a = 0; I->Ancestor[a] != -1; ++a)
	/* counting ancestors */ ;
    I->Ancestor = CheckRealloc(I->Ancestor, (a + 2) * sizeof(long));

    I->Ancestor[a] = ancestor;
    I->Ancestor[a + 1] = -1;

    return a + 1;
}

int             igGetOperatorIndex(int (*func) (G_GENETIC *))
{
    int             o;

    for (o = 0; o < LocalGEN->nOperators; ++o)
	if (LocalGEN->Operator[o].Function == func)
	    return o;
    msgMessage(MSG_ERROR, "igGetOperatorIndex::Error: Unknown operator 0x%p", func);
    return -1;
}

#define _F_BETTER(I1, I2) (gCompare(I1, I2) > 0)
#define _F_NEW_BEST(IND)  _F_BETTER(IND, &GEN->Population.Individual[0])

int             igEvalOffsprint(G_INDIVIDUAL * I)
{
    int             ancestors;
    int             improvement;
    int             worsening;


    if (gCompare(I, LocalGEN->Population.Individual) > 0) {
	return EVOLUTIONARY_MIRACLE;
    } else if (gCompare(I, &LocalGEN->Population.Individual[LocalGEN->Mu - 1]) < 0) {
	return EVOLUTIONARY_DISASTER;
    }

    improvement = worsening = 0;
    for (ancestors = 0; I->Ancestor[ancestors] != -1; ++ancestors) {
	msgMessage(MSG_DEBUG,
		   "igEvalOffsprint::Debug: Comparing individual %lu with ancestor %d (%lu)", I->id,
		   ancestors, I->Ancestor[ancestors]);
	if (gCompare(I, igGetIndividualFromId(I->Ancestor[ancestors])) > 0)
	    ++improvement;
	else if (gCompare(I, igGetIndividualFromId(I->Ancestor[ancestors])) < 0)
	    ++worsening;
    }

    if (improvement == ancestors)
	return EVOLUTIONARY_COMPLETE_SUCCESS;
    else if (worsening == ancestors)
	return EVOLUTIONARY_COMPLETE_FAILURE;
    else if (improvement > 0 && !worsening)
	return EVOLUTIONARY_PARTIAL_SUCCESS;
    else
	return EVOLUTIONARY_PARTIAL_FAILURE;
}
