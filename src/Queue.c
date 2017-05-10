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
*   $Source: /home/squiller/tools/uGP/RCS/Queue.c,v $
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
#include "String.h"
#include "InstructionLibrary.h"
#include "Graph.h"
#include "Genetic.h"
#include "Fitness.h"
#include "ugp.h"

/****************************************************************************/
/*              L O C A L   D A T A   S T R U C T U R E S                   */
/****************************************************************************/

typedef struct _I_QUEUE_ELEM {
    void           *Data;
    struct _I_QUEUE_ELEM *Next;
    struct _I_QUEUE_ELEM *Prev;
} I_QUEUE_ELEM;

typedef struct _I_QUEUE {
    I_QUEUE_ELEM   *First;
    I_QUEUE_ELEM   *Last;
} I_QUEUE;

/*
 * INTERNAL PROTOS
 */
void            iqRemoveElement(I_QUEUE * IL, I_QUEUE_ELEM * E);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

QUEUE           qInitQueue(void)
{
    I_QUEUE        *IL;

    IL = CheckCalloc(1, sizeof(I_QUEUE));
    return (QUEUE) IL;
}

int             qQueueEmpty(QUEUE S)
{
    I_QUEUE        *IL;

    IL = (I_QUEUE *) S;
    return !IL->First;
}

int             qEnqueueElement(QUEUE S, void *element)
{
    I_QUEUE        *IL;
    I_QUEUE_ELEM   *E;

    IL = (I_QUEUE *) S;
    E = CheckMalloc(sizeof(I_QUEUE_ELEM));
    E->Data = element;
    E->Prev = NULL;
    if (IL->First)
	IL->First->Prev = E;
    else
	IL->Last = E;
    E->Next = IL->First;
    IL->First = E;

    return 1;
}

void           *qDequeueElement(QUEUE S)
{
    I_QUEUE        *IL;
    void           *element;

    IL = (I_QUEUE *) S;
    element = IL->Last->Data;
    iqRemoveElement(IL, IL->Last);

    return element;
}

void            qFreeQueue(QUEUE S)
{
    I_QUEUE        *IL;

    IL = (I_QUEUE *) S;
    while (IL->First)
	iqRemoveElement(IL, IL->First);
    CheckFree(IL);
}

void            iqRemoveElement(I_QUEUE * IL, I_QUEUE_ELEM * E)
{
    I_QUEUE_ELEM   *P = E->Prev;
    I_QUEUE_ELEM   *N = E->Next;

    CheckFree(E);
    if (P)
	P->Next = N;
    else
	IL->First = N;

    if (N)
	N->Prev = P;
    else
	IL->Last = P;
}
