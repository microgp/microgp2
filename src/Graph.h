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
*   $Source: /home/squiller/tools/uGP/RCS/Graph.h,v $
* $Revision: 1.7 $
*     $Date: 2003/12/02 07:43:29 $
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

typedef union {
    long            Integer;
    char           *Constant;
    struct _GR_NODE *Node;
} GR_PARAMETER_VALUE;

typedef struct _GR_NODE {
    long            id;
    int             ReferenceCount;
    IL_MACRO       *Macro;
    struct _GR_NODE *Succ;
    struct _GR_NODE *Prev;
    GR_PARAMETER_VALUE *ParameterValue;
    /* back */
    struct _GR_SUBGRAPH *Subgraph;
} GR_NODE;

typedef struct _GR_SUBGRAPH {
    int             id;
    int             nNodes;
    GR_NODE        *Head;
    GR_NODE        *Tail;
    int             Section;
    /* back */
    struct _GR_GRAPH *Graph;
} GR_SUBGRAPH;

typedef struct _GR_GRAPH {
    int             nSubgraphs;
    GR_SUBGRAPH   **Subgraph;
} GR_GRAPH;

typedef struct _GR_SLICE {
    GR_NODE        *Cut1;
    GR_NODE        *Cut2;
} GR_SLICE;

#define DUMP_OP_VERBOSE 	0x0001
#define DUMP_OP_TRANSLATE 	0x0002

char           *grDebugNodeId(GR_NODE * N);
char           *grDebugSubgraphId(GR_SUBGRAPH * T);
GR_GRAPH       *grDuplicateGraph(GR_GRAPH * SrcF);
GR_NODE        *grGetRandomNode(GR_SUBGRAPH * T);
GR_NODE        *grGetRandomInternalNode(GR_SUBGRAPH * T);
GR_SUBGRAPH    *grAddEmptySubgraph(GR_GRAPH * Graph, IL_LIBRARY * Lib, int sec);
int             grAddRandomNode(GR_GRAPH * F, IL_LIBRARY * Lib);
int             grAddRandomNodeToSubgraph(GR_SUBGRAPH * T, IL_LIBRARY * Lib);
int             grLoadGraphFromFile(GR_GRAPH * F, IL_LIBRARY * IL, FILE * Fin);
int             grRandomizeNode(GR_NODE * Node);
int             grRemoveRandomNode(GR_GRAPH * F);
void            grDebugNode(GR_NODE * N);
void            grDumpGraph(GR_GRAPH * F, FILE * FOut, int Options);
void            grFreeGraph(GR_GRAPH * F);
void            grRemoveNode(GR_NODE * N);
void            grRemoveSubgraph(GR_SUBGRAPH * T);
void            grSaveGraphToFile(GR_GRAPH * F, FILE * Fout);
void            grDumpSubgraph(GR_SUBGRAPH * T, FILE * FOut, int Options);
void            grCheckGraph(GR_GRAPH * G);

GR_NODE        *grGetRandomSuccessor(GR_NODE * N);
GR_NODE        *grGetRandomAncestor(GR_NODE * N);
GR_NODE        *grGetRandomSibling(GR_NODE * N);

int             grValidateSlice(GR_SLICE * S);
GR_SUBGRAPH    *grGetRandomCompatibleSubgraph(GR_GRAPH * G, GR_SUBGRAPH * T);
GR_NODE        *grSeekHead(GR_NODE * N);
void            grSwapNodes(GR_NODE * N1, GR_NODE * N2);
void            grFixGraph(GR_GRAPH * G);
GR_NODE        *grCreateNode(GR_NODE * Prev);
int             grAssignNode(GR_NODE * Node, IL_MACRO * Macro);
int             grRandomizeNodeParameter(GR_NODE * Node, int op);
