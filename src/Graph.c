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
*   $Source: /home/squiller/tools/uGP/RCS/Graph.c,v $
* $Revision: 1.12 $
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
#include <time.h>
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
int             igrCmpSubgraph(GR_SUBGRAPH * S1, GR_SUBGRAPH * S2);
int             igrCmpNode(GR_NODE * N1, GR_NODE * N2);
GR_NODE        *igrSeekHeadSafe(GR_NODE * N);
int             igrCmpNodeTopologies(GR_NODE * N1, GR_NODE * N2);
static void     igrDumpFreeMacro(IL_MACRO * M, FILE * FOut, int Options);
static int      igrCountSubgraphInSection(int Section, GR_GRAPH * F);
static int      igrStealSubgraph(GR_GRAPH * Thief, GR_SUBGRAPH * Sub);
static GR_SUBGRAPH *igrCreateRandomSubgraph(GR_GRAPH * F, IL_LIBRARY * IL, int sec);
static void     igrFixSubgraph(GR_SUBGRAPH * G, GR_GRAPH * Parent);
static char    *igrUniqueTag(void);
char           *grDebugGraphId(GR_GRAPH * G);
static GR_NODE *igrSeekFirstInnerReference(GR_NODE * T);
static GR_NODE *igrSeekLastInnerReference(GR_NODE * T);
static GR_NODE *igrSeekFirstOuterReference(GR_SUBGRAPH * S, GR_NODE * T);
static GR_NODE *igrSeekLastOuterReference(GR_SUBGRAPH * S, GR_NODE * T);
static void     igrDereferenceNode(GR_NODE * N);
static int      igrCountReferences(GR_SUBGRAPH * S, GR_NODE * T);
static void     igrCheckSubGraph(GR_SUBGRAPH * S);
static void     igrFreeSubgraph(GR_SUBGRAPH * F);
static void     igrFreeNode(GR_NODE * N);
GR_SUBGRAPH    *igrCreateEmptySubgraph(int nodes);
static void     igrLoadSubgraphParametersFromFile(GR_SUBGRAPH * T, IL_LIBRARY * IL,
						  HASH_TABLE * NodeNames, FILE * Fin);
static GR_SUBGRAPH *igrLoadSubgraphStructureFromFile(IL_LIBRARY * IL, HASH_TABLE * NodeNames,

						     FILE * Fin);
void            igrSaveSubgraphParametersToFile(GR_SUBGRAPH * T, FILE * F);
void            igrSaveSubgraphStructureToFile(GR_SUBGRAPH * T, FILE * F);
static void     igrDumpNode(GR_NODE * N, FILE * FOut, int Options);
static GR_SUBGRAPH *igrGetRandomSubgraph(GR_GRAPH * F, IL_LIBRARY * IL, int sec, int allowcreation);
static char    *igrLabelFormat(GR_NODE * N);
static char    *igrNodeName(GR_NODE * N);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

static long     GlobalNodeCount = 0;
static unsigned long int stat_TotGraphs = 0;
static unsigned long int stat_Clones = 0;
static unsigned long int stat_HashFailures = 0;


/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

static char    *igrNodeName(GR_NODE * N)
{
    static char     tmp[32];

    sprintf(tmp, "n%lu", N->id);
    return tmp;
}

char           *grDebugSubgraphId(GR_SUBGRAPH * T)
{
    static char     id[64];

    sprintf(id, "T:%p {id:%d, s:%d, n:%d}", T, T->id, T->Section, T->nNodes);
    return id;
}

char           *grDebugGraphId(GR_GRAPH * G)
{
    static char     id[64];

    sprintf(id, "G:%p {n:%d}", G, G->nSubgraphs);
    return id;
}


char           *grDebugNodeId(GR_NODE * N)
{
    static char     id[64];

    if (N) {
	if (N->id)
	    sprintf(id, "(n%lu/%p)", N->id, N);
	else
	    sprintf(id, "(***/%p)", N);
    } else {
	strcpy(id, "(null)");
    }
    return id;
}

int             grAssignNode(GR_NODE * Node, IL_MACRO * Macro)
{
    Node->Macro = Macro;
    if (!Node->Macro->nParameters)
	Node->ParameterValue = (GR_PARAMETER_VALUE *) NULL;
    else
	Node->ParameterValue =
	    (GR_PARAMETER_VALUE *) CheckCalloc(Node->Macro->nParameters,
					       sizeof(GR_PARAMETER_VALUE));
    msgMessage(MSG_DEBUG, "grAssignNode::Debug: ParameterValue: %p: [%d> %p, %p]",
	       &Node->ParameterValue, Node->Macro->nParameters, Node->ParameterValue,
	       Node->ParameterValue + Node->Macro->nParameters * sizeof(GR_PARAMETER_VALUE));

    return 0;
}

GR_NODE        *grCreateNode(GR_NODE * Prev)
{
    GR_NODE        *N;

    N = (GR_NODE *) CheckCalloc(1, sizeof(GR_NODE));
    N->id = ++GlobalNodeCount;
    if (Prev) {
	N->Succ = Prev->Succ;
	if (N->Succ)
	    N->Succ->Prev = N;
	Prev->Succ = N;
	N->Prev = Prev;
    }
    msgMessage(MSG_DEBUG, "grCreateNode::Debug: created %s", grDebugNodeId(N));
    return N;
}

int             grRandomizeNode(GR_NODE * Node)
{
    int             o;

    for (o = 0; o < Node->Macro->nParameters; ++o)
	grRandomizeNodeParameter(Node, o);

    return o;
}

int             grRandomizeNodeParameter(GR_NODE * Node, int op)
{
    GR_SUBGRAPH    *T;
    GR_GRAPH       *Graph;
    int             t;

    switch (Node->Macro->Parameter[op]->Type) {
    case PARAMETER_UNIQUE_TAG:
	break;
    case PARAMETER_UNKNOWN:
	msgMessage(MSG_ERROR,
		   "grRandomizeNodeParameter::Error: Illegal \"PARAMETER_UNKNOWN\" Unknown parameter type: %d",
		   Node->Macro->Parameter[op]->Type);
	break;
    case PARAMETER_HEX:
	if (!Node->ParameterValue[op].String)
	    Node->ParameterValue[op].String =
		CheckCalloc(sizeof(char), Node->Macro->Parameter[op]->Def.Digits + 1);

	for (t = 0; t < Node->Macro->Parameter[op]->Def.Digits; ++t)
	    Node->ParameterValue[op].String[t] = "0123456789ABCDEF"[lrand48() % 16];
	break;
    case PARAMETER_INTEGER:
	Node->ParameterValue[op].Integer = Node->Macro->Parameter[op]->Def.Integer.From +
	    lrand48() % (Node->Macro->Parameter[op]->Def.Integer.To -
			 Node->Macro->Parameter[op]->Def.Integer.From);
	break;
    case PARAMETER_CONSTANT:
	Node->ParameterValue[op].String =
	    Node->Macro->Parameter[op]->Def.Constant.Constant[lrand48() %
							      Node->Macro->Parameter[op]->
							      Def.Constant.nConstants];
	break;
    case PARAMETER_INNER_FORWARD_LABEL:
	/* first: dereference old target */
	igrDereferenceNode(Node->ParameterValue[op].Node);
	Node->ParameterValue[op].Node = grGetRandomSuccessor(Node);
	/* increasing target reference count */
	++Node->ParameterValue[op].Node->ReferenceCount;
	break;

    case PARAMETER_INNER_BACKWARD_LABEL:
	/* first: dereference old target */
	igrDereferenceNode(Node->ParameterValue[op].Node);
	Node->ParameterValue[op].Node = grGetRandomAncestor(Node);
	/* increasing target reference count */
	++Node->ParameterValue[op].Node->ReferenceCount;
	break;

    case PARAMETER_INNER_GENERIC_LABEL:
	/* first: dereference old target */
	igrDereferenceNode(Node->ParameterValue[op].Node);
	Node->ParameterValue[op].Node = grGetRandomSibling(Node);
	/* increasing target reference count */
	++Node->ParameterValue[op].Node->ReferenceCount;
	break;

    case PARAMETER_OUTER_LABEL:
	Graph = Node->Subgraph->Graph;
	/* first: dereference old target */
	igrDereferenceNode(Node->ParameterValue[op].Node);

	/*
	 * WHOAH! Big problem!!!!
	 * patch: never refer twice the same subgraph!
	 */
	T = igrCreateRandomSubgraph(Graph,
				    Node->Macro->Section->InstructionLibrary,
				    Node->Macro->Parameter[op]->Def.Section);
	/**
	T = igrGetRandomSubgraph(Graph, 
				 Node->Macro->Section->InstructionLibrary, 
				 Node->Macro->Parameter[op]->Def.Section, 
				 Node->Macro->Section->InstructionLibrary->Section[Node->Macro->Parameter[op]->Def.Section]->AllowMultiple); 
	**/
	Node->ParameterValue[op].Node = T->Head;
	/* increasing target reference count */
	++Node->ParameterValue[op].Node->ReferenceCount;
	break;

    default:
	msgMessage(MSG_ERROR, "grRandomizeNodeParameter::Error: Unknown parameter type: %d",
		   Node->Macro->Parameter[op]->Type);
    }
    return Node->Macro->Parameter[op]->Type;
}

GR_NODE        *grGetRandomInternalNode(GR_SUBGRAPH * T)
{
    char           *_FunctionName = "grGetRandomInternalNode";
    GR_NODE        *N;
    int             r;


    CheckTrue(T->nNodes > 2);
    r = lrand48() % (T->nNodes - 2);
    msgMessage(MSG_DEBUG, "grGetRandomInternalNode::Debug: chosen node %d", r + 1);
    for (N = T->Head; r >= 0; --r)
	N = N->Succ;
    return N;
}

GR_NODE        *grGetRandomNode(GR_SUBGRAPH * T)
{
    GR_NODE        *N;
    int             r;


    r = (lrand48() % T->nNodes) - 1;
    msgMessage(MSG_DEBUG, "grGetRandomInternalNode::Debug: chosen! node %d", r);
    for (N = T->Head; r >= 0; --r)
	N = N->Succ;
    return N;
}

static GR_SUBGRAPH *igrGetRandomSubgraph(GR_GRAPH * F, IL_LIBRARY * IL, int sec, int allowcreation)
{
    char           *_FunctionName = "igrGetRandomSubgraph";
    int            *candidate = NULL;
    int             ncandidates = 0;
    int             t;
    GR_SUBGRAPH    *T;

    msgMessage(MSG_DEBUG, "igrGetRandomSubgraph::Debug: looking for subgraphs of section %d", sec);
    for (t = 0; t < F->nSubgraphs; ++t) {
	if (F->Subgraph[t]->Section == sec) {
	    candidate = (int *)CheckRealloc(candidate, (ncandidates + 1) * sizeof(int));

	    candidate[ncandidates] = t;
	    ++ncandidates;
	}
    }
    if (!ncandidates || allowcreation) {
	msgMessage(MSG_DEBUG, "igrGetRandomSubgraph::Debug: allowing a new subgraph");
	candidate = (int *)CheckRealloc(candidate, (ncandidates + 1) * sizeof(int));

	candidate[ncandidates] = F->nSubgraphs;
	++ncandidates;
    }
    msgMessage(MSG_DEBUG, "igrGetRandomSubgraph::Debug: found %d candidate%s", ncandidates,
	       ncandidates == 1 ? "" : "s");
    t = lrand48() % ncandidates;
    msgMessage(MSG_DEBUG, "igrGetRandomSubgraph::Debug: chosen candidate %d (subgraph number %d)",
	       t, candidate[t]);

    if (candidate[t] == F->nSubgraphs) {
	T = grAddEmptySubgraph(F, IL, sec);
    } else {
	T = F->Subgraph[candidate[t]];
    }
    msgMessage(MSG_DEBUG, "igrGetRandomSubgraph::Debug: got %s", grDebugSubgraphId(T));
    CheckFree(candidate);

    CheckTrue(T);
    return T;
}

static GR_SUBGRAPH *igrCreateRandomSubgraph(GR_GRAPH * F, IL_LIBRARY * IL, int sec)
{
    char           *_FunctionName = "igrCreateRandomSubgraph";
    GR_SUBGRAPH    *T;

    T = grAddEmptySubgraph(F, IL, sec);
    CheckTrue(T);
    grAddRandomNodeToSubgraph(T, IL);
    msgMessage(MSG_DEBUG, "igrCreateRandomSubgraph::Debug: Creating subgraph %x", T);
    return T;
}

static char    *igrLabelFormat(GR_NODE * N)
{
    if (N->Macro->LabelFormat)
	return N->Macro->LabelFormat;
    if (N->Macro->Section->LabelFormat)
	return N->Macro->Section->LabelFormat;
    if (N->Macro->Section->InstructionLibrary->LabelFormat)
	return N->Macro->Section->InstructionLibrary->LabelFormat;
    return NULL;
}

static void     igrDumpNode(GR_NODE * N, FILE * FOut, int Options)
{
    char            m[MAX_MACRO_SIZE];
    char            key[] = { '\b', '\0' };
    char            var[8];
    char            tmp[TMP_SIZE];
    char           *actual;
    int             o;
    char           *e;
    IL_LIBRARY     *IL;

    /* get the instruction library pointer */
    IL = N->Macro->InstructionLibrary;

    if (N->ReferenceCount > 0)
	fprintf(FOut, igrLabelFormat(N), igrNodeName(N));

    if (Options & DUMP_OP_VERBOSE) {
	sprintf(tmp, "node %s", grDebugNodeId(N));
	fprintf(FOut, IL->CommentFormat, tmp);
    }

    if (!N->Macro->Text) {
	if (Options & DUMP_OP_VERBOSE)
	    sprintf(m, IL->CommentFormat, "(null)");
	else
	    strcpy(m, "");
    } else {
	strcpy(m, N->Macro->Text);
    }
    SearchReplace(m, "$$", key);

    while ((e = GetFirstTag(m))) {
	sprintf(tmp, "${%s}", e);
	msgMessage(MSG_DEBUG, "igrDumpNode::Debug: Replacing \"%s\" with \"%s\"", tmp,
		   getenv(e) ? getenv(e) : "(nil)");
	SearchReplace(m, tmp, getenv(e) ? getenv(e) : "(nil)");
    }

    for (o = N->Macro->nParameters - 1; o >= 0; --o) {
	/* valid macro is in m2! */
	sprintf(var, "$%d", o + 1);
	switch (N->Macro->Parameter[o]->Type) {
	case PARAMETER_UNIQUE_TAG:
	    actual = igrUniqueTag();
	    break;
	case PARAMETER_HEX:
	    actual = N->ParameterValue[o].String;
	    break;
	case PARAMETER_INTEGER:
	    sprintf(tmp, "%ld", N->ParameterValue[o].Integer);
	    actual = tmp;
	    break;
	case PARAMETER_CONSTANT:
	    actual = N->ParameterValue[o].String;
	    break;
	case PARAMETER_INNER_FORWARD_LABEL:
	case PARAMETER_INNER_BACKWARD_LABEL:
	case PARAMETER_INNER_GENERIC_LABEL:
	case PARAMETER_OUTER_LABEL:
	    strcpy(tmp, igrNodeName(N->ParameterValue[o].Node));
	    actual = tmp;
	    break;
	default:
	    actual = "*FOO*";
	    msgMessage(MSG_ERROR, "igrDumpNode::Error: Unknown parameter type: %d",
		       N->Macro->Parameter[o]->Type);
	}

	/* replace $n */
	SearchReplace(m, var, actual);
	/* replace $$ */
	SearchReplace(m, "$$", key);
    }
    /* standard replace */
    SearchReplace(m, key, "$");
    SearchReplace(m, "\\n", "\n");

    if (Options & DUMP_OP_TRANSLATE)
	SearchReplace(m, "+-", "-");
    fprintf(FOut, "%s", m);
}


void            grDumpSubgraph(GR_SUBGRAPH * T, FILE * FOut, int Options)
{
    GR_NODE        *N;
    char            tmp[TMP_SIZE];
    IL_LIBRARY     *IL;

    /* get the instruction library pointer */
    IL = T->Head->Macro->InstructionLibrary;

    if (!FOut)
	FOut = stderr;

    if (Options & DUMP_OP_VERBOSE) {
	sprintf(tmp, "subgraph %p", T);
	fprintf(FOut, IL->CommentFormat, tmp);
    }
    for (N = T->Head; N; N = N->Succ)
	igrDumpNode(N, FOut, Options);
    fprintf(FOut, "\n");
}

static GR_SUBGRAPH *igrLoadSubgraphStructureFromFile(IL_LIBRARY * IL, HASH_TABLE * NodeNames,
						     FILE * Fin)
{
    GR_SUBGRAPH    *T;
    GR_NODE        *N;
    int             n;
    int             msec, mid;

    fscanf(Fin, "%d", &n);
    T = igrCreateEmptySubgraph(n);
    fscanf(Fin, "%d", &T->Section);

    for (N = T->Head; N; N = N->Succ) {
	fscanf(Fin, "%*s %lu", &N->id);
	fscanf(Fin, "%d", &N->ReferenceCount);
	fscanf(Fin, "%d %d", &msec, &mid);
	if (mid == -1) {
	    N->Macro = IL->Section[msec]->Prologue;
	    msgMessage(MSG_DEBUG,
		       "igrLoadSubgraphStructureFromFile::Debug: node refers to sec %d prologue",
		       msec);
	} else if (mid == -2) {
	    N->Macro = IL->Section[msec]->Epilogue;
	    msgMessage(MSG_DEBUG,
		       "igrLoadSubgraphStructureFromFile::Debug: node refers to sec %d epilogue",
		       msec);
	} else {
	    N->Macro = IL->Section[msec]->Macro[mid];
	    msgMessage(MSG_DEBUG,
		       "igrLoadSubgraphStructureFromFile::Debug: node refers to sec %d macro %d",
		       msec, mid);
	}
	hPut(NodeNames, igrNodeName(N), N);
    }

    return T;
}


void            igrSaveSubgraphStructureToFile(GR_SUBGRAPH * T, FILE * F)
{
    GR_NODE        *N;

    fprintf(F, "%d\n", T->nNodes);
    fprintf(F, "%d\n", T->Section);
    for (N = T->Head; N; N = N->Succ) {
	fprintf(F, "%s %lu\n", igrNodeName(N), N->id);
	fprintf(F, "%d\n", N->ReferenceCount);
	fprintf(F, "%d %d\n", N->Macro->Section->id, N->Macro->id);
    }
}

GR_SUBGRAPH    *igrCreateEmptySubgraph(int nodes)
{
    char           *_FunctionName = "igrCreateEmptySubgraph";
    GR_SUBGRAPH    *T;
    GR_NODE        *Last, *Node;
    int             n;

    T = (GR_SUBGRAPH *) CheckCalloc(1, sizeof(GR_SUBGRAPH));
    T->nNodes = nodes;
    CheckTrue(T->nNodes >= 2);

    msgMessage(MSG_DEBUG, "igrCreateEmptySubgraph::Debug: building linked array of nodes");

    Last = (GR_NODE *) CheckCalloc(1, sizeof(GR_NODE));
    Last->Subgraph = T;
    T->Head = Last;
    for (n = 1; n < nodes; ++n) {
	Node = (GR_NODE *) CheckCalloc(1, sizeof(GR_NODE));
	Node->Subgraph = T;
	Node->Prev = Last;
	Last->Succ = Node;
	Last = Node;
    }
    T->Tail = Last;
    return T;
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/


void            grDebugNode(GR_NODE * N)
{
    if (!N) {
	msgMessage(MSG_INFO | MSG_NOTIME, "(null) node!");
    } else {
	msgMessage(MSG_INFO | MSG_NOTIME, "%p -> [ %p ] -> %p", N->Prev, N, N->Succ);
	msgMessage(MSG_INFO | MSG_NOTIME, "ReferenceCount: %d", N->ReferenceCount);
	msgMessage(MSG_INFO | MSG_NOTIME, "Macro: %p", N->Macro);
    }
}

void            grDumpGraph(GR_GRAPH * F, FILE * FOut, int Options)
{
    time_t          now;
    struct tm       loc;
    char            tmp[TMP_SIZE];
    char           *Mon[] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    IL_LIBRARY     *IL;
    int             t;
    int             s;

    /* get the instruction library pointer */
    IL = F->Subgraph[0]->Head->Macro->InstructionLibrary;

    time(&now);
    loc = *localtime(&now);

    if (!FOut)
	FOut = stderr;

    fprintf(FOut, IL->CommentFormat, "-*-asm-*-");
    fprintf(FOut, IL->CommentFormat,
	    "This program was automagically created by uGP (MicroGP v" VERSION_NUMBER ")");
    fprintf(FOut, IL->CommentFormat,
	    "Copyright (c) 2002-2004 by Giovanni Squillero <giovanni.squillero@polito.it>");
    fprintf(FOut, IL->CommentFormat, "");
    fprintf(FOut, "\n");

    igrDumpFreeMacro(IL->GlobalPrologue, FOut, Options);
    for (s = 0; s < IL->nSections; ++s) {
	if (igrCountSubgraphInSection(s, F) || IL->Section[s]->AlwaysDump) {
	    igrDumpFreeMacro(IL->Section[s]->GlobalPrologue, FOut, Options);
	    for (t = 0; t < F->nSubgraphs; ++t)
		if (F->Subgraph[t]->Section == s)
		    grDumpSubgraph(F->Subgraph[t], FOut, Options);
	    igrDumpFreeMacro(IL->Section[s]->GlobalEpilogue, FOut, Options);
	}
    }
    igrDumpFreeMacro(IL->GlobalEpilogue, FOut, Options);

    fprintf(FOut, "\n");
    sprintf(tmp, "*** this program: %s %2d %4d %02d:%02d:%02d",
	    Mon[loc.tm_mon], loc.tm_mday, 1900 + loc.tm_year, loc.tm_hour, loc.tm_min, loc.tm_sec);
    fprintf(FOut, IL->CommentFormat, tmp);
    sprintf(tmp, "*** MicroGP v" VERSION_NUMBER " " VERSION_NAME ": %s", ugpLinkingDate());
    fprintf(FOut, IL->CommentFormat, tmp);
}

static void     igrDumpFreeMacro(IL_MACRO * M, FILE * FOut, int Options)
{
    GR_NODE        *N;

    N = grCreateNode(NULL);
    grAssignNode(N, M);
    grRandomizeNode(N);
    igrDumpNode(N, FOut, Options);
    igrFreeNode(N);
}

static int      igrCountSubgraphInSection(int Section, GR_GRAPH * F)
{
    int             t;
    int             count = 0;

    for (t = 0; t < F->nSubgraphs; ++t)
	if (F->Subgraph[t]->Section == Section)
	    ++count;
    return count;
}


GR_SUBGRAPH    *grAddEmptySubgraph(GR_GRAPH * Graph, IL_LIBRARY * Lib, int sec)
{
    GR_SUBGRAPH    *T;

    T = (GR_SUBGRAPH *) CheckCalloc(1, sizeof(GR_SUBGRAPH));
    T->Section = sec;
    T->nNodes = 2;
    T->Graph = Graph;
    T->id = Graph->nSubgraphs;

    T->Head = grCreateNode(NULL);
    grAssignNode(T->Head, Lib->Section[sec]->Prologue);
    T->Head->Subgraph = T;

    T->Tail = grCreateNode(T->Head);
    grAssignNode(T->Tail, Lib->Section[sec]->Epilogue);
    T->Tail->Subgraph = T;

    Graph->Subgraph =
	(GR_SUBGRAPH **) CheckRealloc(Graph->Subgraph,
				      (Graph->nSubgraphs + 1) * sizeof(GR_SUBGRAPH *));
    Graph->Subgraph[Graph->nSubgraphs] = T;
    ++Graph->nSubgraphs;

    grRandomizeNode(T->Head);
    grRandomizeNode(T->Tail);

    msgMessage(MSG_DEBUG, "grAddEmptySubgraph::Debug: added subgraph %s to graph",
	       grDebugSubgraphId(T));
    return T;
}

int             grAddRandomNodeToSubgraph(GR_SUBGRAPH * T, IL_LIBRARY * Lib)
{
    GR_NODE        *P;
    GR_NODE        *N;
    int             m;

    if (T->nNodes == 2) {
	P = T->Head;
    } else {
	P = grGetRandomInternalNode(T);
    }
    N = grCreateNode(P);
    N->Subgraph = T;
    ++T->nNodes;
    msgMessage(MSG_DEBUG, "grAddRandomNodeToSubgraph::Debug: new node added (now: %d)", T->nNodes);

    /* pick a random macro */
    m = ilWeightedGetRandomMacro(Lib, T->Section);
    /* m = lrand48() % Lib->Section[T->Section]->nMacros; */
    msgMessage(MSG_DEBUG, "grAddRandomNodeToSubgraph::Debug: chosen macro number %d", m);
    grAssignNode(N, Lib->Section[T->Section]->Macro[m]);
    grRandomizeNode(N);

    return m;
}

int             grAddRandomNode(GR_GRAPH * F, IL_LIBRARY * Lib)
{
    int             t;

    t = lrand48() % F->nSubgraphs;
    grAddRandomNodeToSubgraph(F->Subgraph[t], Lib);
    return t;
}

void            grRemoveNode(GR_NODE * N)
{
    int             o;

    msgMessage(MSG_DEBUG, "grRemoveNode::Debug: unlinking node %s", grDebugNodeId(N));
    if (N->ReferenceCount > 0)
	msgMessage(MSG_DEBUG, "grRemoveNode::Debug: node refcount = %d", N->ReferenceCount);

    if (N->Prev)
	N->Prev->Succ = N->Succ;
    if (N->Succ)
	N->Succ->Prev = N->Prev;
    else
	msgMessage(MSG_DEBUG, "grRemoveNode::Debug: you are removing tail!");
    --N->Subgraph->nNodes;

    for (o = 0; o < N->Macro->nParameters; ++o)
	if (ilIsParameterLabel(N->Macro->Parameter[o]))
	    igrDereferenceNode(N->ParameterValue[o].Node);

    msgMessage(MSG_DEBUG, "grRemoveNode::Debug: zapping node");
    CheckFree(N->ParameterValue);
    CheckFree(N);
}

int             grRemoveRandomNode(GR_GRAPH * F)
{
    int             t;
    GR_NODE        *N;

    t = lrand48() % F->nSubgraphs;
    if (F->Subgraph[t]->nNodes == 2) {
	msgMessage(MSG_DEBUG, "grRemoveRandomNode::Debug: failed (empty subgraph)");
	return 0;
    }
    N = grGetRandomInternalNode(F->Subgraph[t]);
    if (N->ReferenceCount > 0) {
	msgMessage(MSG_DEBUG, "grRemoveRandomNode::Debug: failed (referenced node)");
	return 0;
    }
    msgMessage(MSG_DEBUG, "grRemoveNode::Debug: removing node from %s",
	       grDebugSubgraphId(N->Subgraph));
    grRemoveNode(N);
    return 1;
}

void            grRemoveSubgraph(GR_SUBGRAPH * T)
{
    char           *_FunctionName = "grRemoveSubgraph";
    GR_NODE        *N, *N2;
    GR_GRAPH       *F = T->Graph;
    int             t, t2;
    int             o;

    for (t2 = t = 0; t < F->nSubgraphs; ++t) {
	if (F->Subgraph[t] != T) {
	    F->Subgraph[t2] = F->Subgraph[t];
	    F->Subgraph[t2]->id = t2;
	    msgMessage(MSG_DEBUG, "grRemoveSubgraph::Debug: moved subgraph %d to %d", t, t2);
	    ++t2;
	}
    }
    CheckTrue(t2 < t);
    --F->nSubgraphs;
    F->Subgraph = (GR_SUBGRAPH **) CheckRealloc(F->Subgraph, F->nSubgraphs * sizeof(GR_SUBGRAPH *));

    /*
     * clear all inner labels to avoid problems dereferencing nodes
     */
    for (N = T->Head; N; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o)
	    if (ilIsParameterInnerLabel(N->Macro->Parameter[o]))
		N->ParameterValue[o].Node = NULL;
    }

    for (N = T->Head; N; N = N2) {
	msgMessage(MSG_DEBUG, "grRemoveSubgraph::Debug: zapping node %s", grDebugNodeId(N));
	N2 = N->Succ;
	grRemoveNode(N);
    }
}

void            grSaveGraphToFile(GR_GRAPH * F, FILE * Fout)
{
    int             t;
    unsigned long   random;

    random = lrand48();

    fprintf(Fout, "%lu\n", random);

    fprintf(Fout, "%d\n", F->nSubgraphs);
    for (t = 0; t < F->nSubgraphs; ++t)
	igrSaveSubgraphStructureToFile(F->Subgraph[t], Fout);
    for (t = 0; t < F->nSubgraphs; ++t)
	igrSaveSubgraphParametersToFile(F->Subgraph[t], Fout);
    fprintf(Fout, "%lu\n", random);
}

int             grLoadGraphFromFile(GR_GRAPH * F, IL_LIBRARY * IL, FILE * Fin)
{
    int             t;
    int             newsubgraphs;
    unsigned long   seed, seed2;
    GR_SUBGRAPH    *T;
    HASH_TABLE     *NodeNames;

    NodeNames = hInitHash(1549, HASH_KEY_CASE);
    fscanf(Fin, "%lu", &seed);
    msgMessage(MSG_DEBUG, "grLoadGraphFromFile::Debug: seed %lu", seed);
    fscanf(Fin, "%d", &newsubgraphs);
    for (t = 0; t < newsubgraphs; ++t) {
	T = igrLoadSubgraphStructureFromFile(IL, NodeNames, Fin);
	T->id = F->nSubgraphs;
	T->Graph = F;
	F->Subgraph =
	    (GR_SUBGRAPH **) CheckRealloc(F->Subgraph, (F->nSubgraphs + 1) * sizeof(GR_SUBGRAPH *));
	F->Subgraph[F->nSubgraphs] = T;
	++F->nSubgraphs;
	msgMessage(MSG_DEBUG, "grLoadGraphFromFile::Debug: added subgraph %d to graph", T->id);
    }
    for (t = 0; t < newsubgraphs; ++t)
	igrLoadSubgraphParametersFromFile(F->Subgraph[F->nSubgraphs - newsubgraphs + t], IL,
					  NodeNames, Fin);

    fscanf(Fin, "%lu", &seed2);
    if (seed != seed2)
	msgMessage(MSG_ERROR, "grLoadGraphFromFile::Error: File corrupted");

    hFreeHash(NodeNames);

    return newsubgraphs;
}

static void     igrLoadSubgraphParametersFromFile(GR_SUBGRAPH * T, IL_LIBRARY * IL,
						  HASH_TABLE * NodeNames, FILE * Fin)
{
    char           *_FunctionName = "igrLoadSubgraphParametersFromFile";
    int             nop;
    char            optype;
    int             o, t;
    GR_NODE        *N;
    char            tmp[TMP_SIZE];

    for (N = T->Head; N; N = N->Succ) {
	fscanf(Fin, "%d", &nop);
	if (nop)
	    N->ParameterValue = (GR_PARAMETER_VALUE *) CheckCalloc(nop, sizeof(GR_PARAMETER_VALUE));

	msgMessage(MSG_DEBUG,
		   "igrLoadSubgraphParametersFromFile::Debug: ParameterValue: %p: [%d> %p, %p]",
		   &N->ParameterValue, N->Macro->nParameters, N->ParameterValue,
		   N->ParameterValue + N->Macro->nParameters * sizeof(GR_PARAMETER_VALUE));

	for (o = 0; o < nop; ++o) {
	    fscanf(Fin, "%s", tmp);
	    optype = *tmp;
	    switch (optype) {
	    case 'h':		/* PARAMETER_HEX */
		fscanf(Fin, "%s", tmp);
		N->ParameterValue[o].String = strdup(tmp);
		CheckTrue(strlen(N->ParameterValue[o].String) ==
			  N->Macro->Parameter[o]->Def.Digits);
		msgMessage(MSG_DEBUG, "igrLoadSubgraphParametersFromFile::Debug: hex parameter %s",
			   N->ParameterValue[o].String);
		break;
	    case 'i':		/* PARAMETER_INTEGER */
		fscanf(Fin, "%ld", &N->ParameterValue[o].Integer);
		msgMessage(MSG_DEBUG,
			   "igrLoadSubgraphParametersFromFile::Debug: integer parameter %ld",
			   N->ParameterValue[o].Integer);
		break;
	    case 'k':		/* PARAMETER_CONSTANT */
		fscanf(Fin, "%s", tmp);
		for (t = 0; t < N->Macro->Parameter[o]->Def.Constant.nConstants; ++t)
		    if (!strcmp(tmp, N->Macro->Parameter[o]->Def.Constant.Constant[t]))
			N->ParameterValue[o].String =
			    N->Macro->Parameter[o]->Def.Constant.Constant[t];
		CheckTrue(N->ParameterValue[o].String);
		msgMessage(MSG_DEBUG,
			   "igrLoadSubgraphParametersFromFile::Debug: constant parameter \"%s\"",
			   N->ParameterValue[o].String);
		break;
	    case 'u':		/* PARAMETER_UNIQUE_TAG */
		msgMessage(MSG_DEBUG, "igrLoadSubgraphParametersFromFile::Debug: unique tag");
		break;
	    case 'l':		/* PARAMETER_*_LABEL */
		fscanf(Fin, "%s", tmp);
		igrNodeName(N->ParameterValue[o].Node = hGet(NodeNames, tmp));
		msgMessage(MSG_DEBUG,
			   "igrLoadSubgraphParametersFromFile::Debug: label parameter \"%s\"", tmp);
		break;
	    }
	}
    }

}

void            igrSaveSubgraphParametersToFile(GR_SUBGRAPH * T, FILE * F)
{
    GR_NODE        *N;
    int             o;

    for (N = T->Head; N; N = N->Succ) {
	fprintf(F, "%d\n", N->Macro->nParameters);
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    switch (N->Macro->Parameter[o]->Type) {
	    case PARAMETER_HEX:
		fprintf(F, "h %s\n", N->ParameterValue[o].String);
		break;
	    case PARAMETER_INTEGER:
		fprintf(F, "i %ld\n", N->ParameterValue[o].Integer);
		break;
	    case PARAMETER_CONSTANT:
		fprintf(F, "k %s\n", N->ParameterValue[o].String);
		break;
	    case PARAMETER_UNIQUE_TAG:
		fprintf(F, "u\n");
		break;
	    case PARAMETER_INNER_FORWARD_LABEL:
	    case PARAMETER_INNER_BACKWARD_LABEL:
	    case PARAMETER_INNER_GENERIC_LABEL:
	    case PARAMETER_OUTER_LABEL:
		fprintf(F, "l %s\n", igrNodeName(N->ParameterValue[o].Node));
		break;
	    default:
		msgMessage(MSG_ERROR, "igrSaveSubgraphParametersToFile: internal error");
	    }
	}
    }
}

GR_GRAPH       *grDuplicateGraph(GR_GRAPH * SrcGraph)
{
    GR_GRAPH       *DstGraph;
    GR_SUBGRAPH    *DstSubGraph;
    GR_NODE        *DstNode, *SrcNode;
    HASH_TABLE     *NodeNames;
    int             t;
    int             o;

#ifdef GS_DEBUG
    grCheckGraph(SrcGraph);
#endif

    /*
     * duplicate structure
     */
    NodeNames = hInitHash(1549, HASH_KEY_CASE);
    DstGraph = grGetNewGraph();
    DstGraph->nSubgraphs = SrcGraph->nSubgraphs;
    DstGraph->Subgraph = (GR_SUBGRAPH **) CheckCalloc(DstGraph->nSubgraphs, sizeof(GR_SUBGRAPH *));
    for (t = 0; t < SrcGraph->nSubgraphs; ++t) {
	/* create new subgraph */
	DstSubGraph = igrCreateEmptySubgraph(SrcGraph->Subgraph[t]->nNodes);
	msgMessage(MSG_DEBUG, "grDuplicateGraph::Debug: Copying %s",
		   grDebugSubgraphId(SrcGraph->Subgraph[t]));
	msgMessage(MSG_DEBUG, "grDuplicateGraph::Debug: ... to %s", grDebugSubgraphId(DstSubGraph));
	DstGraph->Subgraph[t] = DstSubGraph;
	DstSubGraph->Graph = DstGraph;
	DstSubGraph->id = SrcGraph->Subgraph[t]->id;
	DstSubGraph->Section = SrcGraph->Subgraph[t]->Section;
	/* copy structure */
	for (DstNode = DstSubGraph->Head, SrcNode = SrcGraph->Subgraph[t]->Head; SrcNode;
	     DstNode = DstNode->Succ, SrcNode = SrcNode->Succ) {
	    msgMessage(MSG_DEBUG, "grDuplicateGrap::Debug: copying %s", grDebugNodeId(SrcNode));
	    DstNode->id = ++GlobalNodeCount;
	    DstNode->ReferenceCount = SrcNode->ReferenceCount;
	    DstNode->Macro = SrcNode->Macro;
	    msgMessage(MSG_DEBUG, "grDuplicateGraph::Debug: ... to %s", grDebugNodeId(DstNode));
	    /* associate src name with dst pointer */
	    hPut(NodeNames, igrNodeName(SrcNode), DstNode);
	}
	msgMessage(MSG_DEBUG, "grDuplicateGraph::Debug: copied to %s",
		   grDebugSubgraphId(DstSubGraph));
    }

    for (t = 0; t < SrcGraph->nSubgraphs; ++t) {
	/* copy structure */
	for (DstNode = DstGraph->Subgraph[t]->Head, SrcNode = SrcGraph->Subgraph[t]->Head; SrcNode;
	     DstNode = DstNode->Succ, SrcNode = SrcNode->Succ) {
	    if (DstNode->Macro->nParameters)
		DstNode->ParameterValue =
		    (GR_PARAMETER_VALUE *) CheckCalloc(DstNode->Macro->nParameters,
						       sizeof(GR_PARAMETER_VALUE));
	    msgMessage(MSG_DEBUG, "grDuplicateGraph::Debug: ParameterValue: %p: [%d> %p, %p]",
		       &DstNode->ParameterValue,
		       DstNode->Macro->nParameters,
		       DstNode->ParameterValue,
		       DstNode->ParameterValue +
		       DstNode->Macro->nParameters * sizeof(GR_PARAMETER_VALUE));

	    for (o = 0; o < DstNode->Macro->nParameters; ++o) {
		switch (DstNode->Macro->Parameter[o]->Type) {
		case PARAMETER_HEX:
		    DstNode->ParameterValue[o].String = strdup(SrcNode->ParameterValue[o].String);
		    break;
		case PARAMETER_INTEGER:
		    DstNode->ParameterValue[o].Integer = SrcNode->ParameterValue[o].Integer;
		    break;
		case PARAMETER_CONSTANT:
		    DstNode->ParameterValue[o].String = SrcNode->ParameterValue[o].String;
		    break;
		case PARAMETER_INNER_FORWARD_LABEL:
		case PARAMETER_INNER_BACKWARD_LABEL:
		case PARAMETER_INNER_GENERIC_LABEL:
		case PARAMETER_OUTER_LABEL:
		    DstNode->ParameterValue[o].Node =
			hGet(NodeNames, igrNodeName(SrcNode->ParameterValue[o].Node));
		    break;
		case PARAMETER_UNIQUE_TAG:
		    break;
		default:
		    msgMessage(MSG_ERROR, "grDuplicateGraph::Error: internal error");
		}
	    }
	}
    }

    hFreeHash(NodeNames);

#ifdef GS_DEBUG
    grCheckGraph(DstGraph);
#endif
    return DstGraph;
}


static void     igrFreeNode(GR_NODE * N)
{
    msgMessage(MSG_DEBUG, "igrFreeNode::Debug: Zapping %s", grDebugNodeId(N));
    if (N->ParameterValue)
	CheckFree(N->ParameterValue);
    CheckFree(N);
}

static void     igrFreeSubgraph(GR_SUBGRAPH * T)
{
    GR_NODE        *N, *NN;
    int             t;

    msgMessage(MSG_DEBUG, "igrFreeSubgraph::Debug: Freeing %s", grDebugSubgraphId(T));
    t = 0;
    for (N = T->Head; N; N = NN) {
	msgMessage(MSG_DEBUG, "igrFreeSubgraph::Debug: Trying to free node %d/%d", t++, T->nNodes);
	NN = N->Succ;
	if (NN)
	    NN->Prev = NULL;
	igrFreeNode(N);
    }
    CheckFree(T);
}

void            grFreeGraph(GR_GRAPH * F)
{
    int             t;

    for (t = 0; t < F->nSubgraphs; ++t) {
	igrFreeSubgraph(F->Subgraph[t]);
    }
    CheckFree(F->Subgraph);
    CheckFree(F);

}

void            grCheckGraph(GR_GRAPH * G)
{
    char           *_FunctionName = "grCheckGraph";
    int             g;

    msgMessage(MSG_DEBUG, "grCheckGraph::Debug: Checking graph %s", grDebugGraphId(G));

    for (g = 0; g < G->nSubgraphs; ++g) {
	msgMessage(MSG_DEBUG, "Now checking subgraph %d (%p)", g, G->Subgraph[g]);
	CheckTrue(G->Subgraph[g]->Graph == G);
	igrCheckSubGraph(G->Subgraph[g]);
    }
    msgMessage(MSG_DEBUG, "grCheckGraph::Debug: Graph %s check successful", grDebugGraphId(G));
}

static void     igrCheckSubGraph(GR_SUBGRAPH * S)
{
    char           *_FunctionName = "igrCheckSubGraph";
    GR_NODE        *N;
    int             ref;
    int             count;
    int             found;
    int             s;
    int             o;

    CheckTrue(S->Head->Prev == NULL);

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Checking node number (%d)...", S->nNodes);
    for (count = 0, N = S->Head; N; N = N->Succ)
	++count;
    CheckTrue(S->nNodes == count);

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Checking reference count...");
    for (N = S->Head; N->Succ; N = N->Succ) {
	CheckTrue(grSeekHead(N) == igrSeekHeadSafe(N));
	ref = igrCountReferences(S, N);
	if (N == S->Head && N->ReferenceCount != ref) {
	    msgMessage(MSG_DEBUG,
		       "igrCheckSubGraph::Debug: Prologue %p has reference count %d, but %d internal ref",
		       N, N->ReferenceCount, ref);
	} else {
	    if(N->ReferenceCount != ref)
		CheckTrue(N->ReferenceCount == ref);
	}
	CheckTrue(N->Succ->Prev == N);
    }
    CheckTrue(N == S->Tail);

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Checking inner labels...");
    for (N = S->Head; N->Succ; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterInnerLabel(N->Macro->Parameter[o])) {
		CheckTrue(N->ParameterValue[o].Node);
		CheckTrue(N->ParameterValue[o].Node->Subgraph == S);
	    }
	}
    }

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Checking outer labels...");
    for (N = S->Head; N->Succ; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterOuterLabel(N->Macro->Parameter[o])) {
		CheckTrue(N->ParameterValue[o].Node);
		CheckTrue(N->ParameterValue[o].Node->Subgraph->Graph == S->Graph);
	    }
	}
    }

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Looking for undefined macro sections...");
    for (N = S->Head; N->Succ; N = N->Succ)
	CheckTrue(N->Macro->Section->id == S->Section);

    msgMessage(MSG_DEBUG, "igrCheckSubGraph::Checking subgraphs (%d)...", S->Graph->nSubgraphs);
    found = 0;
    for (s = 0; s < S->Graph->nSubgraphs; ++s) {
	if (S->Graph->Subgraph[s] == S)
	    found = 1;
    }
    if (!found)
	msgMessage(MSG_ERROR, "igrCheckSubGraph::Error: subgraph incoherent");
    else
	msgMessage(MSG_DEBUG, "igrCheckSubGraph::Check complete");
}

static int      igrCountReferences(GR_SUBGRAPH * S, GR_NODE * T)
{
    GR_NODE        *N;
    int             ref;
    int             o;

    msgMessage(MSG_DEBUG, "igrCountReferences::Counting references");
    for (ref = 0, N = S->Head; N; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterLabel(N->Macro->Parameter[o]) && !N->ParameterValue)
		msgMessage(MSG_WARNING, "igrCountReferences::Warning: delted label");
	    else if (ilIsParameterLabel(N->Macro->Parameter[o]) && N->ParameterValue[o].Node == T)
		++ref;
	}
    }
    msgMessage(MSG_DEBUG, "igrCountReferences::Found %d reference%s", ref, ref == 1 ? "" : "s");
    return ref;
}

static void     igrDereferenceNode(GR_NODE * N)
{
    if (!N) {
	msgMessage(MSG_DEBUG, "igrDereferenceNode::Debug: dereferencing NULL");
	return;
    }

    msgMessage(MSG_DEBUG, "igrDereferenceNode::Debug: dereferencing target %d", N->id);
    --N->ReferenceCount;
    if (N->ReferenceCount == 0	/* no more referenced */
	&& N->Prev == NULL	/* and it is a Head */
	&& N->Subgraph->id > 0) {	/* not first subgraph (always true?) */
	msgMessage(MSG_DEBUG, "igrDereferenceNode::Debug: removing unreferenced subgraph %d",
		   N->Subgraph->id);
	igrCheckSubGraph(N->Subgraph);
	grRemoveSubgraph(N->Subgraph);
    }
}

GR_NODE        *grSeekHead(GR_NODE * N)
{
    return N->Subgraph->Head;
}

GR_NODE        *igrSeekHeadSafe(GR_NODE * N)
{
    while (N->Prev)
	N = N->Prev;
    return N;
}

int             igrDoesN1SuccedN2(GR_NODE * N1, GR_NODE * N2)
{
    GR_NODE        *N;

    for (N = N2->Succ; N; N = N->Succ)
	if (N == N1)
	    return 1;
    return 0;
}

int             igrDoesN1PrecedeN2(GR_NODE * N1, GR_NODE * N2)
{
    GR_NODE        *N;

    for (N = N1->Succ; N; N = N->Succ)
	if (N == N2)
	    return 1;
    return 0;
}

GR_NODE        *grGetRandomSuccessor(GR_NODE * N)
{
    char           *_FunctionName = "grGetRandomSuccessor";
    GR_NODE        *N2;
    int             r;
    int             count;

    for (N2 = N->Succ, count = 0; N2; N2 = N2->Succ)
	++count;
    msgMessage(MSG_DEBUG, "grGetRandomSuccessor::Debug: node has %d successor%s", count,
	       count == 1 ? "" : "s");
    CheckTrue(count > 0);
    r = lrand48() % count;

    for (N2 = N->Succ; r > 0; --r)
	N2 = N2->Succ;
    return N2;
}

GR_NODE        *grGetRandomAncestor(GR_NODE * N)
{
    char           *_FunctionName = "grGetRandomAncestor";
    GR_NODE        *N2;
    int             r;
    int             count;

    for (N2 = N->Prev, count = 0; N2; N2 = N2->Prev)
	++count;
    msgMessage(MSG_DEBUG, "grGetRandomAncestor::Debug: node has %d ancestor%s", count,
	       count == 1 ? "" : "s");
    CheckTrue(count > 0);
    r = lrand48() % count;

    /* notez bien: may loop to itself... */
    for (N2 = N; r > 0; --r)
	N2 = N2->Prev;
    return N2;
}

GR_NODE        *grGetRandomSibling(GR_NODE * N)
{
    GR_NODE        *N2;
    GR_SUBGRAPH    *S;
    int             r;

    S = N->Subgraph;
    r = (lrand48() % (S->nNodes - 1)) - 1;
    msgMessage(MSG_DEBUG, "grGetRandomSibling::Debug: chosen node %d", r);

    for (N2 = S->Head->Succ; r >= 0; --r)
	N2 = N2->Succ;
    return N2;
}

int             grSliceSize(GR_SLICE * S)
{
    return grNodeDepth(S->Cut2) - grNodeDepth(S->Cut1);
}

int             grShrinkSlice(GR_SLICE * S, int size)
{
    while (grSliceSize(S) > size) {
	if (drand48() > 0.5)
	    S->Cut2 = S->Cut2->Prev;
	else
	    S->Cut1 = S->Cut1->Succ;
    }
    return 1;
}

int             grValidateSliceLight(GR_SLICE * S)
{
    char           *_FunctionName = "grValidateSliceLight";
    GR_NODE        *N;

    CheckTrue(grSeekHead(S->Cut1) == grSeekHead(S->Cut2));
    if (igrDoesN1SuccedN2(S->Cut1, S->Cut2)) {
	N = S->Cut1;
	S->Cut1 = S->Cut2;
	S->Cut2 = N;
    }
    return 1;
}

int             grValidateSlice(GR_SLICE * S)
{
    char           *_FunctionName = "grValidateSlice";
    GR_NODE        *N;
    GR_NODE        *Target;
    GR_NODE        *RefNode;
    int             o;

    CheckTrue(grSeekHead(S->Cut1) == grSeekHead(S->Cut2));
    if (igrDoesN1SuccedN2(S->Cut1, S->Cut2)) {
	N = S->Cut1;
	S->Cut1 = S->Cut2;
	S->Cut2 = N;
    }

    if (msgCheckLevel(MSG_DEBUG)) {
	printf("\n");
	for (N = grSeekHead(S->Cut1); N; N = N->Succ) {
	    printf("%s", grDebugNodeId(N));
	    if (S->Cut1 == N)
		printf(" <--Cut1");
	    if (S->Cut2 == N)
		printf(" <--Cut2");
	    printf("\n");
	}
    }

    /* Check if some node refers to the section... */
    for (N = S->Cut1; N != S->Cut2->Succ; N = N->Succ) {
	if (N->ReferenceCount > 0) {
	    RefNode = igrSeekFirstInnerReference(N);
	    if (!RefNode) {
		msgMessage(MSG_DEBUG, "grValidateSlice::Debug: Ignoring outer reference");
	    } else if (igrDoesN1PrecedeN2(S->Cut1, RefNode) && igrDoesN1PrecedeN2(RefNode, S->Cut2)) {
		msgMessage(MSG_DEBUG, "grValidateSlice::Debug: Referring node inside slice!");
	    } else if (igrDoesN1PrecedeN2(RefNode, S->Cut1)) {
		msgMessage(MSG_DEBUG,
			   "grValidateSlice::Debug: Referring node out of slice. Extending Cut1");
		S->Cut1 = RefNode;
		return grValidateSlice(S);
	    } else if (igrDoesN1PrecedeN2(S->Cut2, RefNode)) {
		msgMessage(MSG_DEBUG,
			   "grValidateSlice::Debug: Referring node out of slice. Extending Cut2");
		S->Cut2 = RefNode;
		return grValidateSlice(S);
	    }
	    RefNode = igrSeekLastInnerReference(N);
	    if (!RefNode) {
		msgMessage(MSG_DEBUG, "grValidateSlice::Debug: Ignoring outer reference");
	    } else if (igrDoesN1PrecedeN2(S->Cut1, RefNode) && igrDoesN1PrecedeN2(RefNode, S->Cut2)) {
		msgMessage(MSG_DEBUG, "grValidateSlice::Debug: Referring node inside slice!");
	    } else if (igrDoesN1PrecedeN2(RefNode, S->Cut1)) {
		msgMessage(MSG_DEBUG,
			   "grValidateSlice::Debug: Referring node out of slice. Extending Cut1");
		S->Cut1 = RefNode;
		return grValidateSlice(S);
	    } else if (igrDoesN1PrecedeN2(S->Cut2, RefNode)) {
		msgMessage(MSG_DEBUG,
			   "grValidateSlice::Debug: Referring node out of slice. Extending Cut2");
		S->Cut2 = RefNode;
		return grValidateSlice(S);
	    }
	}

    }

    /* REMOVED: And now do the same for outer labels (see r1.5)... */

    /* Check if some node refers of the section refer outside... */
    for (N = S->Cut1; N != S->Cut2->Succ; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterInnerLabel(N->Macro->Parameter[o])) {
		Target = N->ParameterValue[o].Node;

		if (igrDoesN1PrecedeN2(S->Cut1, Target) && igrDoesN1PrecedeN2(Target, S->Cut2)) {
		    msgMessage(MSG_DEBUG,
			       "grValidateSlice::Debug: Reference to a node inside slice!");
		} else if (igrDoesN1PrecedeN2(Target, S->Cut1)) {
		    msgMessage(MSG_DEBUG,
			       "grValidateSlice::Debug: Reference to a node out of slice. Extending Cut1");
		    S->Cut1 = Target;
		    return grValidateSlice(S);
		} else if (igrDoesN1PrecedeN2(S->Cut2, Target)) {
		    msgMessage(MSG_DEBUG,
			       "grValidateSlice::Debug: Reference to a node out of slice. Extending Cut2");
		    S->Cut2 = Target;
		    return grValidateSlice(S);
		}
	    }
	}
    }

    return 1;
}

static GR_NODE *igrSeekFirstInnerReference(GR_NODE * T)
{
    GR_NODE        *N;
    int             o;

    for (N = grSeekHead(T); N != NULL; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    /* inner (!) */
	    if (ilIsParameterInnerLabel(N->Macro->Parameter[o]) && N->ParameterValue[o].Node == T)
		return N;
	}
    }

    msgMessage(MSG_DEBUG, "igrSeekFirstInnerReference::Debug: Can't find inner reference!");
    return NULL;
}
static GR_NODE *igrSeekLastInnerReference(GR_NODE * T)
{
    GR_NODE        *N;
    GR_NODE        *F = NULL;
    int             o;

    for (N = grSeekHead(T); N != NULL; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    /* inner (!) */
	    if (ilIsParameterInnerLabel(N->Macro->Parameter[o]) && N->ParameterValue[o].Node == T)
		F = N;
	}
    }

    if (!F)
	msgMessage(MSG_DEBUG, "igrSeekLastInnerReference::Debug: Can't find inner reference!");
    return F;
}

GR_SUBGRAPH    *grGetRandomCompatibleSubgraph(GR_GRAPH * G, GR_SUBGRAPH * T)
{
    int             array[G->nSubgraphs];
    int             t;
    int             a;

    msgMessage(MSG_DEBUG,
	       "grGetRandomCompatibleSubgraph::Debug: Looking for a subgraph compatible with 0x%p (section: %d)",
	       T, T->Section);
    a = 0;
    for (t = 0; t < G->nSubgraphs; ++t)
	if (G->Subgraph[t]->Section == T->Section)
	    array[a++] = t;

    if (!a)
	return NULL;
    t = array[lrand48() % a];

    return G->Subgraph[t];
}


void            grSwapNodes(GR_NODE * N1, GR_NODE * N2)
{
    GR_SUBGRAPH    *SG1 = N1->Subgraph;
    GR_SUBGRAPH    *SG2 = N2->Subgraph;
    GR_NODE        *PrevN1 = N1->Prev;
    GR_NODE        *PrevN2 = N2->Prev;

    N1->Prev = PrevN2;
    if (N1->Prev)
	N1->Prev->Succ = N1;
    N2->Prev = PrevN1;
    if (N2->Prev)
	N2->Prev->Succ = N2;

    msgMessage(MSG_DEBUG, "grSwapNodes::Debug: Invalidating subgraphs %s and %s",
	       grDebugSubgraphId(SG1), grDebugSubgraphId(SG2));
    SG1->nNodes = SG2->nNodes = -1;
}

int             grSwapNodesSafe(GR_NODE * N1, GR_NODE * N2)
{
    GR_NODE         T1;
    GR_NODE         T2;
    int             p;
    int             dist;
    GR_NODE        *T;
    int             t;
    int 	    need_fix = 0;

    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Swapping nodes %p %p", N1, N2); 

    /*
     * avoid swapping Prologue/Epilogue with internal node
     */
    if(!!N1->Prev != !!N2->Prev)
	return 0;
    if(!!N1->Succ != !!N2->Succ)
	return 0;

    memcpy(&T1, N1, sizeof(GR_NODE));
    memcpy(&T2, N2, sizeof(GR_NODE));

    /* dereference referenced inner nodes */
    for (p = 0; p < N1->Macro->nParameters; ++p) {
	if (ilIsParameterInnerLabel(N1->Macro->Parameter[p])) {
	    igrDereferenceNode(N1->ParameterValue[p].Node);
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Param %d: decreased %p refcount to %d", p, N1->ParameterValue[p].Node, N1->ParameterValue[p].Node->ReferenceCount);
	    dist = grNodeDepth(N1->ParameterValue[p].Node) - grNodeDepth(N1);
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Param %d: translated %p to offset %+0d", p, N1->ParameterValue[p].Node, dist);
	    N1->ParameterValue[p].Integer = dist;
	}
    }
    for (p = 0; p < N2->Macro->nParameters; ++p) {
	if (ilIsParameterInnerLabel(N2->Macro->Parameter[p])) {
	    igrDereferenceNode(N2->ParameterValue[p].Node);
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Param %p: decreased %p refcount to %d", p, N2->ParameterValue[p].Node, N2->ParameterValue[p].Node->ReferenceCount);
	    dist = grNodeDepth(N2->ParameterValue[p].Node) - grNodeDepth(N2);
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Param %p: translated %p to offset %+0d", p, N2->ParameterValue[p].Node, dist);
	    N2->ParameterValue[p].Integer = dist;
	}
    }
    
    /* copy & repair */
    N1->Macro = T2.Macro;
    N1->ParameterValue = T2.ParameterValue;
    for (p = 0; p < N1->Macro->nParameters; ++p) {
	if (ilIsParameterInnerLabel(N1->Macro->Parameter[p])) {
	    dist = N1->ParameterValue[p].Integer;
	    if (dist > 0)
		for (T = N1, t = 0; t < dist; ++t)
		    T = T->Succ ? T->Succ : T;
	    else
		for (T = N1, t = 0; t < -dist; ++t)
		    T = T->Prev->Prev ? T->Prev : T;
	    N1->ParameterValue[p].Node = T;
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Translated offset %+0d to %p", dist, T);
	    ++T->ReferenceCount;
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Increased %p refcount to %d", T, T->ReferenceCount);
	} else if (ilIsParameterOuterLabel(N1->Macro->Parameter[p])) {
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Stealing subgraph %p", N1->ParameterValue[p].Node->Subgraph);
	    need_fix = 1;
	}
    }

    /* copy & repair */
    N2->Macro = T1.Macro;
    N2->ParameterValue = T1.ParameterValue;
    for (p = 0; p < N2->Macro->nParameters; ++p) {
	if (ilIsParameterInnerLabel(N2->Macro->Parameter[p])) {	    
	    dist = N2->ParameterValue[p].Integer;
	    if (dist > 0)
		for (T = N2, t = 0; t < dist; ++t)
		    T = T->Succ ? T->Succ : T;
	    else
		for (T = N2, t = 0; t < -dist; ++t)
		    T = T->Prev->Prev ? T->Prev : T;
	    N2->ParameterValue[p].Node = T;
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Translated offset %+0d to %p", dist, T);
	    ++T->ReferenceCount;
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Increased %p refcount to %d", T, T->ReferenceCount);
	} else if (ilIsParameterOuterLabel(N2->Macro->Parameter[p])) {
	    msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Stealing subgraph %p", N2->ParameterValue[p].Node->Subgraph);
	    need_fix = 1;
	}
    }

    if(need_fix) {
	msgMessage(MSG_DEBUG, "grSwapNodesSafe::Debug: Fixing subgraphs");
	grFixGraph(N1->Subgraph->Graph);
	grFixGraph(N2->Subgraph->Graph);
    } 

    return 1+need_fix;
}

static void     igrFixSubgraph(GR_SUBGRAPH * G, GR_GRAPH * Parent)
{
    GR_NODE        *N;
    int             count;

    G->Graph = Parent;
    count = 0;
    for (N = G->Head; N->Succ; N = N->Succ) {
	++count;
	N->Subgraph = G;
    }
    N->Subgraph = G;
    G->Tail = N;
    G->nNodes = count + 1;
}

void            grFixGraph(GR_GRAPH * G)
{
    int             s;
    int             o;
    GR_NODE        *N;

    /* 
     * stealing all connected subgraphs
     */
    for (N = G->Subgraph[0]->Head; N; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterOuterLabel(N->Macro->Parameter[o])) {
		igrStealSubgraph(G, N->ParameterValue[o].Node->Subgraph);
	    }
	}
    }

    /* 
     * then fix all stuff up 
     */
    for (s = 0; s < G->nSubgraphs; ++s) {
	igrFixSubgraph(G->Subgraph[s], G);
    }
}

static char    *igrUniqueTag(void)
{
    static long int count;
    static char     tag[32];

    sprintf(tag, "u%lu", ++count);
    return tag;
}

static GR_NODE *igrSeekFirstOuterReference(GR_SUBGRAPH * S, GR_NODE * T)
{
    GR_NODE        *N;
    int             o;

    for (N = S->Head; N != NULL; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterOuterLabel(N->Macro->Parameter[o]) && N->ParameterValue[o].Node == T)
		return N;
	}
    }

    msgMessage(MSG_INFO, "igrSeekFirstOuterReference::Info: Can't find inner reference!");
    return NULL;
}
static GR_NODE *igrSeekLastOuterReference(GR_SUBGRAPH * S, GR_NODE * T)
{
    GR_NODE        *N;
    GR_NODE        *F = NULL;
    int             o;

    for (N = S->Head; N != NULL; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterOuterLabel(N->Macro->Parameter[o]) && N->ParameterValue[o].Node == T)
		F = N;
	}
    }

    if (!F)
	msgMessage(MSG_INFO, "igrSeekLastOuterReference::Info: Can't find inner reference!");
    return F;
}

static int      igrStealSubgraph(GR_GRAPH * Thief, GR_SUBGRAPH * Sub)
{
    char           *_FunctionName = "igrStealSubgraph";
    int             t;
    int             s;
    int             o;
    int             tot;
    GR_NODE        *N;

    /*
     * remove subgraph Sub from its graph
     */
    for (s = 0, t = 0; t < Sub->Graph->nSubgraphs; ++t)
	if (Sub->Graph->Subgraph[t] != Sub)
	    Sub->Graph->Subgraph[s++] = Sub->Graph->Subgraph[t];
    CheckTrue(s == Sub->Graph->nSubgraphs - 1);
    --Sub->Graph->nSubgraphs;
    Sub->Graph->Subgraph =
	CheckRealloc(Sub->Graph->Subgraph, Sub->Graph->nSubgraphs * sizeof(GR_SUBGRAPH *));

    /*
     * add Sub to Thief's list
     */
    ++Thief->nSubgraphs;
    Thief->Subgraph = CheckRealloc(Thief->Subgraph, Thief->nSubgraphs * sizeof(GR_SUBGRAPH *));
    Thief->Subgraph[Thief->nSubgraphs - 1] = Sub;

    /*
     * then propagate
     */
    tot = 1;
    for (N = Sub->Head; N; N = N->Succ) {
	for (o = 0; o < N->Macro->nParameters; ++o) {
	    if (ilIsParameterOuterLabel(N->Macro->Parameter[o])) {
		tot += igrStealSubgraph(Thief, N->ParameterValue[o].Node->Subgraph);
	    }
	}
    }
    return tot;
}

int             grNodeDepth(GR_NODE * N)
{
    GR_NODE        *T;
    int             d;

    for (d = 0, T = grSeekHead(N); T != N; T = T->Succ)
	++d;
    return d;
}

int             igrCmpNodeTopologies(GR_NODE * N1, GR_NODE * N2)
{
    if (N1->Macro != N2->Macro) {
	msgMessage(MSG_DEBUG, "N1->Macro != N2->Macro");
	return 1;
    }
    if (N1->ReferenceCount != N2->ReferenceCount) {
	msgMessage(MSG_DEBUG, "N1->ReferenceCount != N2->ReferenceCount");
	return 3;
    }
    if (grNodeDepth(N1) != grNodeDepth(N2)) {
	msgMessage(MSG_DEBUG, "grNodeDepth(N1) != grNodeDepth(N2)");
	return 4;
    }

    return 0;
}

int             igrCmpNode(GR_NODE * N1, GR_NODE * N2)
{
    int             p;

    if (igrCmpNodeTopologies(N1, N2)) {
	msgMessage(MSG_DEBUG, "igrCmpNodeTopologies(N1, N2)");
	return -1;
    }

    for (p = 0; p < N1->Macro->nParameters; ++p) {
	switch (N1->Macro->Parameter[p]->Type) {
	case PARAMETER_UNIQUE_TAG:
	    break;
	case PARAMETER_CONSTANT:
	case PARAMETER_HEX:
	    if (strcmp(N1->ParameterValue[p].String, N2->ParameterValue[p].String)) {
		msgMessage(MSG_DEBUG,
			   "strcmp(N1->ParameterValue[p].String, N2->ParameterValue[p].String)");
		return 1 + p;
	    }
	    break;
	case PARAMETER_INTEGER:
	    if (N1->ParameterValue[p].Integer != N2->ParameterValue[p].Integer) {
		msgMessage(MSG_DEBUG,
			   "N1->ParameterValue[p].Integer != N2->ParameterValue[p].Integer");
		return 1 + p;
	    }
	    break;
	case PARAMETER_INNER_FORWARD_LABEL:
	case PARAMETER_INNER_BACKWARD_LABEL:
	case PARAMETER_INNER_GENERIC_LABEL:
	case PARAMETER_OUTER_LABEL:
	    if (igrCmpNodeTopologies(N1->ParameterValue[p].Node, N2->ParameterValue[p].Node)) {
		msgMessage(MSG_DEBUG,
			   "!igrCmpNodeTopologies(N1->ParameterValue[p].Node, N2->ParameterValue[p].Node)");
		return 1 + p;
	    }
	}
    }
    return 0;
}

int             igrCmpSubgraph(GR_SUBGRAPH * S1, GR_SUBGRAPH * S2)
{
    int             n;
    GR_NODE        *N1, *N2;

    if (S1->nNodes != S2->nNodes) {
	msgMessage(MSG_DEBUG, "S1->nNodes != S2->nNodes");
	return -1;
    }
    if (S1->Section != S2->Section) {
	msgMessage(MSG_DEBUG, "S1->Section != S2->Section");
	return -2;
    }

    N1 = S1->Head;
    N2 = S2->Head;
    for (n = 0; n < S1->nNodes; ++n) {
	if (igrCmpNode(N1, N2)) {
	    msgMessage(MSG_DEBUG, "igrCmpNode(N1, N2)");
	    return 1 + n;
	}
	N1 = N1->Succ;
	N2 = N2->Succ;
    }
    return 0;
}

int             grCmpGraph(GR_GRAPH * G1, GR_GRAPH * G2)
{
    int             s;

    if (G1->HashID && G2->HashID && G1->HashID != G2->HashID) {
	msgMessage(MSG_DEBUG, "grCmpGraph::Debug: G1->HashID != G2->HashID");
	return -1;
    }
    if (G1->nSubgraphs != G2->nSubgraphs) {
	msgMessage(MSG_VERBOSE,
		   "grCmpGraph::Debug: Same hashes but G1->nSubgraphs != G2->nSubgraphs");
	++stat_HashFailures;
	return -2;
    }
    for (s = 0; s < G1->nSubgraphs; ++s) {
	if (igrCmpSubgraph(G1->Subgraph[s], G2->Subgraph[s])) {
		msgMessage(MSG_VERBOSE,
			   "grCmpGraph::Debug: Same hashes but igrCmpSubgraph(G1->Subgraph[s], G2->Subgraph[s])) ");
		++stat_HashFailures;
		return 1 + s;
	}
    }
    ++stat_Clones;
    return 0;
}

long int        grCalculateHashID(GR_GRAPH * G)
{
    int             s;
    GR_NODE        *N;
    int             h, p;

    G->HashID = 0;
    G->HashID = hHashFunctionInt(G->nSubgraphs, G->HashID);
    for (s = 0; s < G->nSubgraphs; ++s) {
	G->HashID = hHashFunctionInt(G->Subgraph[s]->nNodes, G->HashID);
	for (N = G->Subgraph[s]->Head; N; N = N->Succ) {
	    /*
	     * Calculate node's HashID
	     */
	    N->HashID = hHashFunctionInt(N->ReferenceCount, 0L);
	    N->HashID = hHashFunctionInt(N->Macro->id, N->HashID);
	    for (p = 0; p < N->Macro->nParameters; ++p) {
		N->HashID = hHashFunctionInt(N->Macro->Parameter[p]->Type, N->HashID);
		switch (N->Macro->Parameter[p]->Type) {
		case PARAMETER_UNIQUE_TAG:
		    break;
		case PARAMETER_CONSTANT:
		case PARAMETER_HEX:
		    for (h = 0; N->ParameterValue[p].String[h]; ++h)
			N->HashID = hHashFunctionInt(N->ParameterValue[p].String[h], N->HashID);
		    break;
		case PARAMETER_INTEGER:
		    N->HashID = hHashFunctionInt(N->ParameterValue[p].Integer, N->HashID);
		    break;
		case PARAMETER_INNER_FORWARD_LABEL:
		case PARAMETER_INNER_BACKWARD_LABEL:
		case PARAMETER_INNER_GENERIC_LABEL:
		case PARAMETER_OUTER_LABEL:
		    N->HashID = hHashFunctionInt(grNodeDepth(N->ParameterValue[p].Node), N->HashID);
		}
	    }
	    /*
	     * Use node's HashID for Graph
	     */
	    G->HashID = hHashFunctionInt(N->HashID, G->HashID);
	}
    }
    msgMessage(MSG_DEBUG, "grCalculateHashID::Debug: Graph 0x%p HASH: 0x%08lx", G, G->HashID);
    return G->HashID;
}


void            grDisplayGraphStat(void)
{
    msgMessage(MSG_INFO | MSG_NOTIME, "");
    msgMessage(MSG_INFO, "gGraphStat::");
    msgMessage(MSG_INFO | MSG_NOTIME, "Total number of graphs: %lu", stat_TotGraphs);
    msgMessage(MSG_INFO | MSG_NOTIME, "Detected clones: %lu (hash failures: %lu)", stat_Clones,
	       stat_HashFailures);
}

GR_GRAPH       *grGetNewGraph(void)
{
    ++stat_TotGraphs;
    return CheckCalloc(1, sizeof(GR_GRAPH));
}
