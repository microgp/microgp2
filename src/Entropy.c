
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
*   $Source: /home/squiller/tools/uGP/RCS/Entropy.c,v $
* $Revision: 1.1 $
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

/*
 * INTERNAL PROTOS
 */
static void     ienAddSymbol(long int symbol);
static void     ienFreeSymbolTable(void);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

#define SYMBOL_HASH_SIZE 7237

typedef struct _SYMBOL_ELEM {
    long int        Symbol;
    int             Count;
    struct _SYMBOL_ELEM *Next;
} SYMBOL_ELEM;

static SYMBOL_ELEM *SymbolMap[SYMBOL_HASH_SIZE];
static long int numSymbols;
static long int numUniqueSymbols;

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

static void     ienFreeSymbolTable(void)
{
    int             t;
    SYMBOL_ELEM    *T;

    numSymbols = numUniqueSymbols = 0;
    for (t = 0; t < SYMBOL_HASH_SIZE; ++t) {
	while (SymbolMap[t]) {
	    T = SymbolMap[t]->Next;
	    CheckFree(SymbolMap[t]);
	    SymbolMap[t] = T;
	}
    }
}

static void     ienAddSymbol(long int symbol)
{
    int             h;
    SYMBOL_ELEM    *S;

    ++numSymbols;
    h = hHashFunctionInt2(symbol) % SYMBOL_HASH_SIZE;
    S = SymbolMap[h];
    while (S && S->Symbol != symbol)
	S = S->Next;

    if (S) {
	++S->Count;
    } else {
	S = CheckMalloc(sizeof(SYMBOL_ELEM));
	S->Symbol = symbol;
	S->Count = 1;
	S->Next = SymbolMap[h];
	SymbolMap[h] = S;
	++numUniqueSymbols;
    }
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

double          enCalculateEntropy(G_GENETIC * GEN)
{
    char           *_FunctionName = "enCalculateEntropy";
    double          entropy;
    int             i;
    GR_NODE        *N;
    GR_GRAPH       *G;
    int             s;
    SYMBOL_ELEM    *S;
    double          p;

    ienFreeSymbolTable();
    for (i = 0; i < GEN->Mu; ++i) {
	G = GEN->Population.Individual[i].Graph;
	for (s = 0; s < G->nSubgraphs; ++s) {
	    for (N = G->Subgraph[s]->Head; N; N = N->Succ) {
		CheckTrue(N->HashID);
		ienAddSymbol(N->HashID);
	    }
	}
    }

    entropy = 0.0l;
    for (s = 0; s < SYMBOL_HASH_SIZE; ++s) {
	for (S = SymbolMap[s]; S; S = S->Next) {
	    p = 1.0 * S->Count / numSymbols;
	    entropy += p * log(p);
	}
    }

    return -entropy;
}
