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
*   $Source: /home/squiller/tools/uGP/RCS/Check.h,v $
* $Revision: 1.6 $
*     $Date: 2003/12/02 07:43:29 $
*
******************************************************************************
*                                                                            *
*  Copyright (c) 1994-2003 Giovanni Squillero                                *
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

#ifndef CHECK_HEADER_ALREADY_INCLUDED

#define CHECK_HEADER_ALREADY_INCLUDED Yes
/* 
 * CHECK MACROS. Really USEFUL
 */
#define CheckFalse(ex)                                                        \
        { long      _Kerouac;                                                 \
          if( (_Kerouac=(long) (ex)) ) {                                      \
                msgMessage(MSG_FORCESHOW, "");                                \
                msgMessage(MSG_ERROR,                                         \
                      "%s::check failed:"                                     \
                      " expression is true (0x%lx)\n"                         \
                      "           %s\n"                                       \
                      "           in %s:%d\n",                                \
                      _FunctionName, _Kerouac, #ex, __FILE__, __LINE__);      \
        } }

#define CheckTrue(ex)                                                         \
        { if((ex)==0) {                                                       \
                msgMessage(MSG_FORCESHOW, "");                                \
                msgMessage(MSG_ERROR,                                         \
                      "%s::check failed:"                                     \
                      " expression is false (zero)\n"                         \
                      "           %s\n"                                       \
                      "           in %s:%d\n",                                \
                      _FunctionName, #ex, __FILE__, __LINE__);                \
        } }

#ifdef GS_DEBUG
#define DCheckTrue(X)    CheckTrue(X)
#define DCheckFalse(X)   CheckFalse(X)
#else
#define DCheckTrue(X)    0
#define DCheckFalse(X)   0
#endif

#endif
