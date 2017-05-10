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
*   $Source: /home/squiller/tools/uGP/RCS/Stack.c,v $
* $Revision: 1.6 $
*     $Date: 2003/12/02 07:43:29 $
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
#include "Stack.h"
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
/*              L O C A L   D A T A   S T R U C T U R E S                   */
/****************************************************************************/

typedef struct _I_STACK {
    int             Size;
    int             AllocatedSpace;
    void          **Table;
} I_STACK;

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

STACK          *sInitStack(void)
{
    I_STACK        *S;

    S = CheckCalloc(1, sizeof(I_STACK));
    return (STACK *) S;
}

int             sStackEmpty(STACK * S)
{
    I_STACK        *IS;

    IS = (I_STACK *) S;
    return !IS->Size;
}

int             sPushElement(STACK * S, void *element, int unique)
{
    int             t;
    I_STACK        *IS;

    IS = (I_STACK *) S;
    if (unique) {
	for (t = 0; t < IS->Size; ++t)
	    if (IS->Table[t] == element)
		return 0;
    }
    if (IS->Size == IS->AllocatedSpace) {
	IS->Table = CheckRealloc(IS->Table, (IS->AllocatedSpace + 1) * sizeof(void *));

	++IS->AllocatedSpace;
    }
    IS->Table[IS->Size] = element;
    ++IS->Size;
    return 1;
}

void           *sPopElement(STACK * S)
{
    char           *_FunctionName = "sPopElement";
    I_STACK        *IS;

    IS = (I_STACK *) S;
    CheckTrue(IS->Size > 0);
    return IS->Table[--IS->Size];
}

void            sFreeStack(STACK * S)
{
    I_STACK        *IS;

    IS = (I_STACK *) S;
    CheckFree(IS->Table);
    CheckFree(IS);
}
