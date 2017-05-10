/********************************************************************-*-c-*-*\
*               *                                                            *
*    #####      * Giovanni Squillero, Ph.D.                                  *
*   ######      * Politecnico di Torino - Dip. Automatica e Informatica      *
*   ###   \     * Cso Duca degli Abruzzi 24 / I-10129 TORINO / ITALY         *
*    ##G  c\    *                                                            *
*    #     _\   * Tel: +39-011564.7186  /  Fax: +39-011564.7099              *
*    |   _/     * email: giovanni.squillero@polito.it                        *
*    |  _/      * www  : http://www.cad.polito.it/~squiller/                 *
*               *                                                            *
******************************************************************************
*
*   $Source: /home/squiller/tools/uGP/RCS/ugp.c,v $
* $Revision: 1.6 $
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

char *__progname = "MicroGP v" VERSION_NUMBER;

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
