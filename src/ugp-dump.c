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
*   $Source: /home/squiller/tools/uGP/RCS/ugp-dump.c,v $
* $Revision: 1.1 $
*     $Date: 2004/03/29 16:17:11 $
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
#include <float.h>
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
#include "GPL.h"
#include "ugp.h"

#ifndef UGP_LINKED_BY
#define UGP_LINKED_BY "anonymous"
#endif

#ifndef UGP_LINKED_MACHINE
#define UGP_LINKED_MACHINE "unknown"
#endif

#ifndef UGP_FLAGS
#define UGP_FLAGS "unknown"
#endif

/*
 * INTERNAL PROTOS
 */
static void     ShowVersionInfoAndQuit(int retcode);
static void     ShowHelpAndQuit(int retcode);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/
char           *ProgramName;
static char    *opInstructionLibrary = "ugp.il";
static int      opVerboseLevel;
static char    *opFitnessFile = "program.s";

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/
char           *ugpLinkingDate(void)
{
    return __DATE__ " " __TIME__;
};

char           *ugpLinkingUser(void)
{
    return UGP_LINKED_BY;
}

char           *ugpLinkingMachine(void)
{
    return UGP_LINKED_MACHINE;
}

char           *ugpLinkingFlags(void)
{
    return UGP_FLAGS;
}

static void     ShowHelpAndQuit(int retcode)
{
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "USAGE: %s [-GVL:]", ProgramName);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "General:\n" "    --license, --help, --version");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Program verbosity:\n"
	       "    --debug, --verbose, --standard, --only-warnings, --silent");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Other options:\n"
	       "    --fitness-script-input %%s  [default: %s]\n"
	       "    --instruction-library %%s  [default: %s]"
	       , opFitnessFile, opInstructionLibrary);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    exit(retcode);
}


static void     ShowVersionInfoAndQuit(int retcode)
{
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Built on %s by %s", ugpLinkingMachine(), ugpLinkingUser());
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "CFLAGS=\"%s\"", ugpLinkingFlags());
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");
    exit(retcode);
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             main(int argc, char *argv[])
{
    char           *_FunctionName = "ugp-dump";
    char            c;
    int             i;
    char            tmp[TMP_SIZE];
    extern int      optind;
    extern char    *optarg;
    G_GENETIC       GEN;
    IL_LIBRARY     *IL;
    FILE           *F;
    int             option_index = 0;
    static struct option long_options[] = {
	/* verbose options */
	{"debug", no_argument, &opVerboseLevel, MSG_DEBUG | MSG_SHOWTIME},
	{"verbose", no_argument, &opVerboseLevel, MSG_VERBOSE | MSG_SHOWTIME},
	{"standard", no_argument, &opVerboseLevel, MSG_INFO},
	{"only-warnings", no_argument, &opVerboseLevel, MSG_WARNING},
	{"silent", no_argument, &opVerboseLevel, MSG_NONE | MSG_SHOWTIME},
	{"fitness-script-input", required_argument, 0, 'f'},

	/* help */
	{"gpl", no_argument, 0, 'G'},
	{"license", no_argument, 0, 'G'},
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},

	/* instruction library */
	{"instruction-library", required_argument, 0, 'L'},

	{0, 0, 0, 0}
    };

    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "ugp-dump v1.0");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "    A simple population dumper for MicroGP v" VERSION_NUMBER);
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "Copyright (c) 2004 by Giovanni Squillero <giovanni.squillero@polito.it>");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME,
	       "This is free software, and you are welcome to redistribute it under certain");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "conditions (use option \"--license\" for details).");
    msgMessage(MSG_FORCESHOW | MSG_NOTIME, "");

    opVerboseLevel = MSG_INFO;
    ProgramName = argv[0];

    while ((c = getopt_long(argc, argv, "?hGVL:", long_options, &option_index)) != -1) {
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
	case 'f':
	    opFitnessFile = CheckStrdup(optarg);
	    break;
	case 'G':
	    GPL(stderr);
	    exit(0);
	    break;
	case 'L':
	    opInstructionLibrary = optarg;
	    msgMessage(MSG_INFO, "Reading instruction library from file \"%s\"",
		       opInstructionLibrary);
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

    memset(&GEN, 0, sizeof(GEN));
    GEN.IL = IL;
    GEN.Mu = 0;
    GEN.Nu = 0;
    GEN.Lambda = 0;
    GEN.MaxGenerations = 0;
    GEN.NumFitnesses = 2;
    GEN.InitialPopulation = argv;
    GEN.FitnessFile = opFitnessFile;
    gInitializePopulation(&GEN);

    CheckTrue(GEN.Population.Size > 0);
    msgMessage(MSG_VERBOSE, "Loaded %d individual%s", GEN.Population.Size, GEN.Population.Size==1?"":"s");

    msgBarGraphStart(MSG_INFO, GEN.Population.Size, "Dumping");
    for(i=0; i<GEN.Population.Size; ++i) {
	msgBarGraph(i);
	sprintf(tmp, "d%04d_%s", i, GEN.FitnessFile);
	CheckTrue(F = fopen(tmp, "w"));
	grDumpGraph(GEN.Population.Individual[i].Graph, F, 0);
	fclose(F);
    }
    msgBarGraphEnd();
    return 0;
}
