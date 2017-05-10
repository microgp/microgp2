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
*   $Source: /home/squiller/tools/uGP/RCS/ugp.h,v $
* $Revision: 1.9 $
*     $Date: 2004/02/06 16:17:24 $
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

#define VERSION_NUMBER	"2.7.0 [beta]"
#define VERSION_NAME	"\"Dasyuroides byrnei\""
/* 
 * common name: Kowari 
 * 
 * Habitat : Sandy deserts of central Australia
 * Diet : Eats small vertibrates and invertibrates.
 * Size : reaches 320mm of which 140mm is the tail and weighs about 120grams.
 */

/* #define VERSION_NAME	"\"Peniocereus greggii\"" */
/* comon name for: Arizona Queen of the Night */

#define MAX_LINE 	1024
#define MAX_MACRO_SIZE	4096
#define TMP_SIZE 	1024

char           *ugpLinkingDate(void);
char           *ugpLinkingUser(void);
char           *ugpLinkingMachine(void);
char           *ugpLinkingFlags(void);

/* Berkeley FreeBSD */
int             setenv( register const char *name, register const char *value, int rewrite);
char           *getenv(const char *name);


#ifdef NEED_SOME_PROTOS
double          drand48(void);
double          erand48(unsigned short xi[3]);
long            lrand48(void);
long            nrand48(unsigned short xi[3]);
long            mrand48(void);
long            jrand48(unsigned short xi[3]);
void            srand48(long seedval);
unsigned short *seed48(unsigned short seed16v[3]);
void            lcong48(unsigned short param[7]);
int             getopt(int argc, char *const *argv, const char *optstring);
char           *strdup(const char *s1);
int             strcasecmp(const char *s1, const char *s2);
int             strncasecmp(const char *s1, const char *s2, int n);
#endif
