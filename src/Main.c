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
*   $Source: /home/squiller/tools/uGP/RCS/Main.c,v $
* $Revision: 1.10 $
*     $Date: 2004/02/27 09:56:37 $
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

#include "getopt.h"

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

#define SHORT_OPTIONS "GhHVv:L:R:S:g:i:m:l:s:I:M:A:N:cF:p:o:f:TK:P:"

/*
 * INTERNAL PROTOS
 */
void            MoreHelp(FILE * fout);
void            GPL(FILE * fout);
void            ShowHelpAndQuit(int retcode);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/
char           *ProgramName;

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/
static int      opParallelFitnessCalls = 1;
static int      opNumFitnesses = 2;
static int      opIdleLimit = -1;
static int      opCompleteEvaluation = 0;
static long     opRandomSeed = 0;
static char    *opInstructionLibrary = "ugp.il";
static int      opMaxGenerations = 100;
static int      opMu = 30;
static int      opLambda = 20;
static double   opSigma = 0.5;
static double   opInertia = 0.8;
static int      opInitSteps = 50;
static char    *opCsvFile = NULL;
static char    *opFitnessFile = "program.s";
static char    *opFitnessOut = "fitness.out";
static char    *opFitnessProgram = "fitness.sh";
static double   opFitnessLimit = -1;
static char   **opInitialPopulation;
static char    *opCrashRecoverDump = "ugp.population";
static int      opTranslatePlusMinus = 1;
static    int             opVerboseLevel;

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void            GPL(FILE * fout)
{
    fprintf(fout,
	    "This program is free software; you can redistribute it and/or modify it under\n"
	    "the terms of the GNU General Public License as published by the Free Software\n"
	    "Foundation; either version 2 of the License, or (at your option) any later\n"
	    "version.\n"
	    "\n"
	    "This program is distributed in the hope that it will be useful, but WITHOUT \n"
	    "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS \n"
	    "FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
	    "\n"
	    "You should have received a copy of the GNU General Public License along with \n"
	    "this program; if not, write to the Free Software Foundation, Inc., \n"
	    "59 Temple Place, Suite 330, Boston, MA 02111-1307 USA\n" "\n");
}

void            MoreHelp(FILE * fout)
{

    fprintf(fout,
	    "**** CSV STATISTICS ****\n"
	    "\n"
	    "Sigma: current strength of genetic operators.\n"
	    "\n"
	    "Mu: number of individuals in the current population.\n"
	    "\n"
	    "Lambda: numbers of new individuals generated applying genetic operators.\n"
	    "\n"
	    "OPx: likelihood of applying genetic operator OPx in arbitrary units.\n"
	    "\n"
	    "Best: fitness of the best individual up to the current one.\n"
	    "\n"
	    "Average (mu): average fitness of the current population (mu\n"
	    "individuals).\n"
	    "\n"
	    "Average (lambda): average fitness of the current offspring (lambda\n"
	    "individuals).\n"
	    "\n"
	    "On-Line: (De Jong's On-Line Performance) average fitness of all trials\n"
	    "up to the current one. Measures system ability to perform well in\n"
	    "on-line applications.\n"
	    "\n"
	    "Off-Line: (De Jong's Off-Line Performance) average of the fitness of\n"
	    "the best individuals up to the current one. Measure the convergence\n"
	    "and expected performance in off-line applications.\n"
	    "\n"
	    "**** EVOLUTIONARY STATUS ****\n"
	    "\n"
	    "SUCCESSES: Whoah (!), Success (+), Almost success (>)\n"
	    "FAILURES : Almost failure (<), Failure (-), Doh (?)\n"
	    "\n"
	    "**** AUTHOR INFO ****\n"
	    "\n"
	    "Dr. Giovanni Squillero\n"
	    "email: giovanni.squillero@polito.it\n"
	    "www  : http://www.cad.polito.it/staff/squillero/\n"
	    "smail: Politecnico di Torino - DAUIN\n"
	    "       Cso Duca degli Abruzzi 24\n" "       10129 Torino - ITALY\n" "\n");
}

void            ShowHelpAndQuit(int retcode)
{
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "USAGE: %s [-" SHORT_OPTIONS "]", ProgramName);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "General:\n"
	       "    --license, --help, --more-help, --version");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Program verbosity:\n"
	       "    --debug, --verbose, --standard, --only-warnings, --silent");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Library:\n"
	       "    --instruction-library %%s  [default: %s]", opInstructionLibrary);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Evolution:\n"
	       "    --random-seed %%l  [default: time since the Epoch]\n"
	       "    --csv-file %%s  [default: no csv file]\n"
	       "    --max-generation %%d  [default: %d]\n"
	       "    --initial-individual-size %%d  [default: %d]\n"
	       "    --mu %%d  [default: %d]\n"
	       "    --lambda %%d  [default: %d]\n"
	       "    --sigma %%f  [default: %0.4f]\n"
	       "    --self-adaptation-inertia %%f  [default: %0.3f]\n"
	       "    --steady-state-threshold %%f  [default: infinite]",
	       opMaxGenerations, opInitSteps, opMu, opLambda, opSigma, opInertia);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Fitness:\n"
	       "    --concurrent-fitness-calls %%d  [default: %d]\n"
	       "    --fitness-values %%d  [default: %d]\n"
	       "    --max-fitness %%f  [default: unknown]\n"
	       "    --fitness-script %%s  [default: %s]\n"
	       "    --fitness-script-input %%s  [default: %s]\n"
	       "    --fitness-script-output %%s  [default: %s]\n"
	       "    --dump-population-before-evaluation  [default: don't do it]\n"
	       "    --disable-smart-sign  [default: do translate \"+-\" to \"-\"]",
	       opParallelFitnessCalls, opNumFitnesses, opFitnessProgram, opFitnessFile, opFitnessOut);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Crash recovery:\n"
	       "    --population-file %%s  [default: %s]\n"
	       "    --load-population %%s  [no default, multiple ok]",
	       opCrashRecoverDump);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    exit(retcode);
}

void            ShowVersionInfoAndQuit(int retcode)
{
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Built on %s by %s", ugpLinkingMachine(), ugpLinkingUser());
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "CFLAGS=\"%s\"", ugpLinkingFlags());
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    exit(retcode);
}

int             main(int argc, char *argv[])
{
    char           *_FunctionName = "uGP::main";
    char            c;
    int             n;
    extern int      optind;
    extern char    *optarg;
    IL_LIBRARY     *IL;
    G_GENETIC       GEN;
    int             option_index = 0;
    static struct option long_options[] = {
	/* verbose options */
	{"debug", no_argument, &opVerboseLevel, MSG_DEBUG|MSG_SHOWTIME},
	{"verbose", no_argument, &opVerboseLevel, MSG_VERBOSE|MSG_SHOWTIME},
	{"standard", no_argument, &opVerboseLevel, MSG_INFO},
	{"only-warnings", no_argument, &opVerboseLevel, MSG_WARNING},	
	{"silent", no_argument, &opVerboseLevel, MSG_NONE|MSG_SHOWTIME},

	/* help */
	{"gpl", no_argument, 0, 'G'},
	{"license", no_argument, 0, 'G'},
	{"help", no_argument, 0, 'h'},
	{"more-help", no_argument, 0, 'H'},
	{"version", no_argument, 0, 'V'},
	
	/* instruction library */
	{"instruction-library", required_argument, 0, 'L'},

	/* evolution options */
	{"random-seed", required_argument, 0, 'R'},
	{"csv-file", required_argument, 0, 'C'},
	{"max-generations", required_argument, 0, 'g'},
	{"initial-size", required_argument, 0, 'i'},
	{"mu", required_argument, 0, 'm'},
	{"lambda", required_argument, 0, 'l'},
	{"sigma", required_argument, 0, 's'},
	{"self-adaptation-inertia", required_argument, 0, 'I'},
	{"steady-state-threshold", required_argument, 0, 'M'},

	/* fitness */
	{"concurrent-fitness-calls", required_argument, 0, 'A'},
	{"fitness-values", required_argument, 0, 'N'},
	{"dump-population-before-evaluation", no_argument, &opCompleteEvaluation, 1},
	{"max-fitness", required_argument, 0, 'F'},
	{"fitness-script", required_argument, 0, 'p'},
	{"fitness-script-output", required_argument, 0, 'o'},
	{"fitness-script-input", required_argument, 0, 'f'},
	{"disable-smart-sign", no_argument, &opTranslatePlusMinus, 0},
	
	/* crash recovery */
	{"population-file", required_argument, 0, 'K'},
	{"load-population", required_argument, 0, 'P'},

	{0, 0, 0, 0}
    };

    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "uGP (MicroGP v" VERSION_NUMBER " " VERSION_NAME ") built %s", ugpLinkingDate());
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "    An evolutionary self-adaptive turing-complete assembly program generator");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Copyright (c) 2002-2004 by Giovanni Squillero <giovanni.squillero@polito.it>");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "This is free software, and you are welcome to redistribute it under certain");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "conditions (use option \"--license\" for details).");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");

    opInitialPopulation = CheckMalloc(sizeof(char *));

    opInitialPopulation[0] = NULL;
    opVerboseLevel = MSG_INFO;
    ProgramName = argv[0];

    while ((c = getopt_long(argc, argv, "?" SHORT_OPTIONS, long_options, &option_index)) != -1) {
	switch (c) {
	case 0:
	    /* If this option set a flag, do nothing else now. */
	    if (long_options[option_index].flag != 0)
		break;
	    printf("option %s", long_options[option_index].name);
	    if (optarg)
		printf(" with arg %s", optarg);
	    printf("\n");
	    break;
	    
	case '?':
	case 'h':
	    ShowHelpAndQuit(0);
	    break;
	case 'H':
	    MoreHelp(stderr);
	    exit(0);
	    break;
	case 'V':
	    ShowVersionInfoAndQuit(0);
	    break;
	case 'v':
	    switch (*optarg) {
	    case 'd':
		opVerboseLevel = MSG_DEBUG | MSG_SHOWTIME;
		break;
	    case 'v':
		opVerboseLevel = MSG_VERBOSE | MSG_SHOWTIME;
		break;
	    case 'i':
		opVerboseLevel = MSG_INFO;
		break;
	    case 'w':
		opVerboseLevel = MSG_WARNING;
		break;
	    case 'n':
		opVerboseLevel = MSG_NONE | MSG_SHOWTIME;
		break;
	    default:
		msgMessage(MSG_FORCESHOW, "Illegal options -v%s", optarg);
		ShowHelpAndQuit(1);
	    }
	    break;
	case 'G':
	    GPL(stderr);
	    exit(0);
	    break;
	case 'f':
	    opFitnessFile = CheckStrdup(optarg);
	    break;
	case 'p':
	    opFitnessProgram = CheckStrdup(optarg);
	    break;
	case 'o':
	    opFitnessOut = CheckStrdup(optarg);
	    break;
	case 'F':
	    opFitnessLimit = atof(optarg);
	    break;
	case 'N':
	    opNumFitnesses = atoi(optarg);
	    break;
	case 'A':
	    opParallelFitnessCalls = atoi(optarg);
	    break;
	case 'R':
	    opRandomSeed = atol(optarg);
	    break;
	case 'T':
	    opTranslatePlusMinus = 0;
	    break;
	case 'L':
	    opInstructionLibrary = optarg;
	    msgMessage(MSG_INFO, "Reading instruction library from file \"%s\"",
		       opInstructionLibrary);
	    break;
	case 'i':
	    opInitSteps = atoi(optarg);
	    msgMessage(MSG_VERBOSE, "Initial creation steps set to %d", opInitSteps);
	    break;
	case 'g':
	    opMaxGenerations = atoi(optarg);
	    msgMessage(MSG_VERBOSE, "Max generations set to %d", opMaxGenerations);
	    break;
	case 'm':
	    opMu = atoi(optarg);
	    msgMessage(MSG_VERBOSE, "Mu set to %d", opMu);
	    break;
	case 'l':
	    opLambda = atoi(optarg);
	    msgMessage(MSG_VERBOSE, "Lambda set to %d", opLambda);
	    break;
	case 's':
	    opSigma = atof(optarg);
	    msgMessage(MSG_VERBOSE, "Sigma set to %f", opLambda);
	    break;
	case 'I':
	    opInertia = atof(optarg);
	    msgMessage(MSG_VERBOSE, "Self-adaptation inertia set to %f", opInertia);
	    break;
	case 'M':
	    opIdleLimit = atoi(optarg);
	    msgMessage(MSG_VERBOSE, "Idle limit set to %d", opIdleLimit);
	    break;
	case 'C':
	    opCsvFile = optarg;
	    msgMessage(MSG_VERBOSE, "Dumping evolution stats to \"%s\".", opCsvFile);
	    break;
	case 'c':
	    opCompleteEvaluation = 1;
	    msgMessage(MSG_VERBOSE, "Using complete evaluation at each generation");
	    break;
	case 'K':
	    opCrashRecoverDump = optarg;
	    msgMessage(MSG_VERBOSE, "Dumping population to \"%s\".", opCrashRecoverDump);
	    break;
	case 'P':
	    for (n = 0; opInitialPopulation[n]; ++n);
	    opInitialPopulation = CheckRealloc(opInitialPopulation, (n + 2) * sizeof(char *));

	    opInitialPopulation[n] = CheckStrdup(optarg);
	    opInitialPopulation[n + 1] = NULL;
	    msgMessage(MSG_VERBOSE, "Loading initial population from file \"%s\".",
		       opInitialPopulation[n]);
	    break;
	default:
	    ShowHelpAndQuit(1);
	}
    }
    argv += optind;
    argc -= optind;

    msgSetLevel(opVerboseLevel);

    CheckTrue(IL = ilReadInstructionLibrary(opInstructionLibrary));
    ilInfoLibrary(IL);    

    if (opRandomSeed) {
	msgMessage(MSG_INFO, "Random seed set to %ld", opRandomSeed);
	srand48(opRandomSeed);
    } else {
	msgMessage(MSG_INFO, "Random seed set to time(NULL)");
	srand48(time(NULL));
    }

    memset(&GEN, 0, sizeof(GEN));
    GEN.NumFitnesses = opNumFitnesses;
    GEN.IL = IL;
    GEN.Mu = opMu;
    GEN.Lambda = opLambda;
    GEN.Inertia = opInertia;
    GEN.IdleLimit = opIdleLimit;
    GEN.Sigma = opSigma;
    GEN.InitNodes = opInitSteps;
    GEN.MaxGenerations = opMaxGenerations;
    GEN.CompleteEvaluation = opCompleteEvaluation;
    GEN.FitnessFile = opFitnessFile;
    GEN.FitnessOut = opFitnessOut;
    GEN.FitnessProgram = opFitnessProgram;
    GEN.CrashRecoveryDump = opCrashRecoverDump;
    GEN.InitialPopulation = opInitialPopulation;
    GEN.FitnessLimit = opFitnessLimit;
    GEN.ParallelFitnessCalls = opParallelFitnessCalls;
 
    if (opCsvFile)
	GEN.CsvFile = CheckStrdup(opCsvFile);

    GEN.DumpOptions = 0;
    if (opTranslatePlusMinus)
	GEN.DumpOptions |= DUMP_OP_TRANSLATE;

    gEvolution(&GEN);
    ilFreeInstructionLibrary(IL);
    gFreeGEN(&GEN);

    return 0;
}
