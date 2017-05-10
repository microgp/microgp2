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
*   $Source: /home/squiller/tools/uGP/RCS/String.c,v $
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

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

char           *SearchReplace(char *string, const char *search, const char *replace)
{
    char           *s;
    char            tmp1[MAX_MACRO_SIZE];
    char            tmp2[MAX_MACRO_SIZE];
    char           *pos;

    strcpy(tmp1, string);
    pos = tmp1;
    while ((s = strstr(pos, search))) {
	strcpy(tmp2, s);
	strcpy(s, replace);
	strcat(s, tmp2 + strlen(search));
	pos = s + strlen(replace);
    }

    /* yeuch!
     * msgMessage(MSG_DEBUG, "SearchReplace::Debug: \"%s\" =~ s|%s|%s|g => \"%s\"", string, search, replace, tmp1);
     */

    strcpy(string, tmp1);
    return string;
}

char           *Chop(char *line)
{
    int             t;

    for (t = strlen(line) - 1; t >= 0 && isspace((int)line[t]); --t)
	if (isspace((int)line[t]))
	    line[t] = '\0';
    msgMessage(MSG_DEBUG, "Line chopped to \"%s\"", line);
    return line;
}

char           *GetFirstTag(char *string)
{
    static char     tmp[TMP_SIZE];
    char           *c;
    int             state;
    int             tmp_pos;

    c = string;
    state = 0;
    while (*c) {
	switch (state) {
	case 0:
	    if (*c == '$')
		state = 1;
	    break;
	case 1:
	    if (*c == '{')
		state = 2;
	    else
		state = 1;
	    break;
	case 2:
	    if (isalpha(*c) || *c == '_') {
		tmp_pos = 0;
		tmp[tmp_pos++] = *c;
		state = 3;
	    } else {
		state = 1;
	    }
	    break;
	case 3:
	    if (*c == '}') {
		tmp[tmp_pos] = '\0';
		return tmp;
	    } else if (isalnum(*c) || *c == '_') {
		tmp[tmp_pos++] = *c;
	    } else {
		state = 1;
	    }
	    break;
	}
	++c;
    }
    return NULL;
}

void            ENV(char *s)
{
    msgMessage(MSG_INFO, "${%s} = \"%s\"", s, getenv(s) ? getenv(s) : "(null)");
}

char           *GetExtension(char *filename)
{
    char           *ext = ".s";
    int             t;

    for (t = 0; filename[t]; ++t)
	if (filename[t] == '.')
	    ext = &filename[t];
    return ext;
}
