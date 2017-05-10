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
*   $Source: /home/squiller/tools/uGP/RCS/Genetic.h,v $
* $Revision: 1.9 $
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

#define CSV_SEPARATOR ","

/* early declaration... */
typedef struct _G_GENETIC G_GENETIC;

enum {
    EVOLUTIONARY_UNKNOWN = 0,
    EVOLUTIONARY_DISASTER = 1,
    EVOLUTIONARY_COMPLETE_FAILURE = 2,
    EVOLUTIONARY_PARTIAL_FAILURE = 3,
    EVOLUTIONARY_PARTIAL_SUCCESS = 10,
    EVOLUTIONARY_COMPLETE_SUCCESS = 11,
    EVOLUTIONARY_MIRACLE = 12
};

typedef struct _G_INDIVIDUAL {
    long            id;
    int             Birth;
    char           *Info;
    GR_GRAPH       *Graph;
    double         *Fitness;
    long           *Ancestor;
    int             LastOperatorUsed;
} G_INDIVIDUAL;

typedef struct _G_POPULATION {
    int             Size;
    int             Space;
    G_INDIVIDUAL   *Individual;
} G_POPULATION;

typedef struct _G_OPERATOR {
    int             (*Function) (G_GENETIC * GEN);
    char           *Description;
    char            Symbol;
    int             pCurrent;
    int             pMax, pMin;
    int            *statActivation;
    int            *statSuccess;
} G_OPERATOR;

struct _G_GENETIC {
    int             NumFitnesses;
    int             Generations;
    int             MaxGenerations;
    int             Mu;
    int             Lambda;
    G_POPULATION    Population;
    int             nOperators;
    G_OPERATOR     *Operator;
    double          Sigma;
    int             InitNodes;
    double          Inertia;
    int             DumpOptions;
    int             MaximizingP;
    int             CompleteEvaluation;
    int             IdleLimit;
    int             ParallelFitnessCalls;

    char           *CrashRecoveryDump;
    char          **InitialPopulation;
    char           *CsvFile;
    char           *FitnessFile;
    char           *FitnessProgram;
    char           *FitnessOut;
    double          FitnessLimit;

    IL_LIBRARY     *IL;

    long int        statIndividuals;
    int            *statSuccess;

    double         *statBestFit;
    double         *statAvgFitnessMu;
    double         *statAvgFitnessLambda;
    double          statOnLine;
    int             statOnLine_num;
    double          statOffLine;
    int             statOffLine_num;
};

int             gAddIndividualToPopulation(G_POPULATION * P, G_INDIVIDUAL * I);
int             gCompare(const void *x1, const void *x2);
int             gDeclareOperator(G_GENETIC * GEN, int (*Function) (G_GENETIC *), char *description,
				 char symbol, int p_curr, int p_min, int p_max);
int             gEvolution(G_GENETIC * GEN);
int             gTournament(G_INDIVIDUAL * Candidate, int nCandidates, int tau);
void            gFreeGEN(G_GENETIC * GEN);
void            gFreeIndividual(G_INDIVIDUAL * I);
void            gInitializeIndividual(G_GENETIC * GEN, G_INDIVIDUAL * I);
G_INDIVIDUAL   *gDuplicateIndividual(G_GENETIC * GEN, G_INDIVIDUAL * I);
int             gAddAncestor(G_INDIVIDUAL * I, long ancestor);

#ifdef NEED_SOME_PROTOS
double          drand48(void);
double          rint(double x);
#endif
