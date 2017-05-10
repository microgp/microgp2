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
*   $Source: /home/squiller/tools/uGP/RCS/Memory.h,v $
* $Revision: 1.7 $
*     $Date: 2004/03/12 16:19:03 $
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

void           *CheckCalloc(size_t nelem, size_t elsize);
void           *CheckMalloc(size_t size);
void           *CheckRealloc(void *ptr, size_t size);
char           *CheckStrdup(const char *s);

void            SafeMemCpy(void *dst, void *src, size_t size);

#define 	CheckFree(X) 	(_CheckFree(X), X=NULL)
void            _CheckFree(void *ptr);

#ifdef NEED_SOME_PROTOS
char           *strdup(const char *s1);
#endif
