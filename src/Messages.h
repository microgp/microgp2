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
*   $Source: /home/squiller/tools/uGP/RCS/Messages.h,v $
* $Revision: 1.6 $
*     $Date: 2003/12/02 07:43:29 $
*
******************************************************************************
*                                                                            *
*  Copyright (c) 2000-2003 Giovanni Squillero                                *
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

#define MSG_ABORT 	0x1000

#define MSG_OPTIONMASK	0x000f
#define MSG_SHOWTIME 	0x0001
#define MSG_NOTIME 	0x0004
#define MSG_EXACT 	0x0008

#define MSG_DEBUG 	0x0010
#define MSG_VERBOSE 	0x0020
#define MSG_INFO 	0x0040
#define MSG_NONE 	0x0100
#define MSG_WARNING 	0x0400
#define MSG_FORCESHOW 	0x0800

#define MSG_ERROR 	(MSG_ABORT | MSG_FORCESHOW | MSG_SHOWTIME)

void            msgBarGraphStart(int level, int max, const char *template, ...);
void            msgBarGraphEnd(void);
void            msgBarGraph(int current);
void            msgBarGraphSpecial(char symbol);

int             msgCheckLevel(int level);
int             msgSetLevel(int level);
int             msgMessage(int level, const char *template, ...);
