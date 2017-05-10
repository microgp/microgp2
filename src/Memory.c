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
*   $Source: /home/squiller/tools/uGP/RCS/Memory.c,v $
* $Revision: 1.6 $
*     $Date: 2003/12/02 07:43:29 $
*
******************************************************************************
*                                                                            *
*  Copyright (c) 1999-2003 Giovanni Squillero                                *
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
#ifndef DMALLOC
/* yeuch! */
#  include <stdlib.h>
#endif
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef LINUX
#  include "gmacros.h"
#else
#  include <macros.h>
#  include <sys/wait.h>
#  include <sys/times.h>
#endif
#include <signal.h>

#ifdef DMALLOC
#  include "dmalloc.h"
#endif

#include "Messages.h"
#include "Memory.h"

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

void           *CheckRealloc(void *ptr, size_t size)
{
    register void  *r;

    if (!(r = realloc(ptr, size)))
	msgMessage(MSG_ERROR, "Memory::Error: Can't realloc %d byte%s", size, size == 1 ? "" : "s");
    return r;
}

void           *CheckMalloc(size_t size)
{
    register void  *r;

    if (!(r = malloc(size)))
	msgMessage(MSG_ERROR, "Memory::Error: Can't malloc %d byte%s", size, size == 1 ? "" : "s");
    return r;
}

void           *CheckCalloc(size_t nelem, size_t elsize)
{
    register void  *r;

    if (!(r = malloc(nelem * elsize)))
	msgMessage(MSG_ERROR, "Memory::Error: Can't calloc %d element%s of %d byte%s",
		   nelem, nelem == 1 ? "" : "s", elsize, elsize == 1 ? "" : "s");
    memset(r, 0, nelem * elsize);
    return r;
}

char           *CheckStrdup(const char *s)
{
    register char  *r;

    if (!(r = strdup(s)))
	msgMessage(MSG_ERROR, "Memory::Error: Can't strdup \"%s\"", s);
    return r;
}

void            _CheckFree(void *ptr)
{
    if (!ptr)
	msgMessage(MSG_DEBUG, "Memory: Attempting to free a NULL block");
    else
	free(ptr);
}

void            SafeMemCpy(void *dst, void *src, size_t size)
{
    register int    t;
    char           *s = src, *d = dst;

    if (!dst)
	msgMessage(MSG_ERROR, "Memory::Error: Can't memcpy to NULL");
    if (!src)
	msgMessage(MSG_ERROR, "Memory::Error: Can't memcpy from NULL");
    for (t = 0; t < size; ++t)
	d[t] = s[t];
}
