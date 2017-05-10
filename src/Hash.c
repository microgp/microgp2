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
*   $Source: /home/squiller/tools/uGP/RCS/Hash.c,v $
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

/****************************************************************************/
/*              L O C A L   D A T A   S T R U C T U R E S                   */
/****************************************************************************/

typedef struct _I_HASH_ELEM {
    unsigned char  *Key;
    void           *Data;
    struct _I_HASH_ELEM *Next;
} I_HASH_ELEM;

typedef struct _I_HASH {
    int             CaseSensitive;
    int             Size;
    I_HASH_ELEM   **Table;
} I_HASH;

/*
 * INTERNAL PROTOS
 */
static void     ihInitZobristLookupTab(void);
static void     ihZapHashElem(I_HASH_ELEM * E);
static unsigned int ihZHashFuncCaseSensitive(const unsigned char *key);
static unsigned int ihZHashFuncCaseInSensitive(const unsigned char *key);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/


/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

/* The mixing step */
#define mix(a,b,c) \
{ \
    a=a-b;  a=a-c;  a=a^(c>>13); \
    b=b-c;  b=b-a;  b=b^(a<<8);  \
    c=c-a;  c=c-b;  c=c^(b>>13); \
    a=a-b;  a=a-c;  a=a^(c>>12); \
    b=b-c;  b=b-a;  b=b^(a<<16); \
    c=c-a;  c=c-b;  c=c^(b>>5);  \
    a=a-b;  a=a-c;  a=a^(c>>3);  \
    b=b-c;  b=b-a;  b=b^(a<<10); \
    c=c-a;  c=c-b;  c=c^(b>>15); \
}

#define ZMAXBYTES	64
static unsigned int ZobristLookupTab[ZMAXBYTES][256];

static void     ihInitZobristLookupTab(void)
{
    int             t, u;

    for (t = 0; t < ZMAXBYTES; ++t)
	for (u = 0; u < 256; ++u)
	    ZobristLookupTab[t][u] = lrand48();
    while (ZobristLookupTab[0][0] == 0l)
	ZobristLookupTab[0][0] = lrand48();
    msgMessage(MSG_DEBUG, "Hash::Debug: Zobrist LookUpTable initialized (zmax=%d)", ZMAXBYTES);
}

static unsigned int ihZHashFuncCaseSensitive(const unsigned char *key)
{
    unsigned int    hash = 0;
    int             i;

    for (i = 0; key[i]; ++i)
	hash ^= ZobristLookupTab[i % ZMAXBYTES][(int)key[i]];
    return hash;
}

static unsigned int ihZHashFuncCaseInSensitive(const unsigned char *key)
{
    unsigned int    hash = 0;
    int             i;

    for (i = 0; key[i]; ++i)
	hash ^= ZobristLookupTab[i % ZMAXBYTES][(int)tolower(key[i])];
    return hash;
}

static void     ihZapHashElem(I_HASH_ELEM * E)
{
    I_HASH_ELEM    *E2;

    if (!E)
	return;
    msgMessage(MSG_DEBUG, "Hash::Debug: Zapping { \"%s\", %p }", E->Key, E->Data);
    E2 = E->Next;
    free(E->Key);
    free(E);
    if (E2)
	ihZapHashElem(E2);
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/

int             hCheckKey(HASH_TABLE H, const unsigned char *key)
{
    int             h;
    I_HASH         *IH = (I_HASH *) H;
    I_HASH_ELEM    *E;

    if (IH->CaseSensitive)
	h = ihZHashFuncCaseSensitive(key) % IH->Size;
    else
	h = ihZHashFuncCaseInSensitive(key) % IH->Size;

    for (E = IH->Table[h]; E; E = E->Next)
	if ((IH->CaseSensitive && !strcmp(key, E->Key))
	    || (!IH->CaseSensitive && !strcasecmp(key, E->Key))) {
	    msgMessage(MSG_DEBUG, "Hash::Debug: hCheckKey(%p, \"%s\") yields 1", IH, key);
	    return 1;
	}
    msgMessage(MSG_DEBUG, "Hash::Debug: hCheckKey(%p, \"%s\") yields 0", IH, key);
    return 0;
}

HASH_TABLE      hInitHash(int size, int casesensitive)
{
    I_HASH         *IH;

    if (ZobristLookupTab[0][0] == 0l)
	ihInitZobristLookupTab();

    IH = (I_HASH *) CheckMalloc(sizeof(I_HASH));
    IH->Size = size;
    IH->CaseSensitive = casesensitive;
    (I_HASH **) IH->Table = (I_HASH_ELEM **) CheckCalloc(IH->Size, sizeof(I_HASH_ELEM *));
    msgMessage(MSG_DEBUG, "Hash::Debug: New HashTable %p initialized (dim=%d)", IH, IH->Size);

    return (HASH_TABLE) IH;
}

void            hFreeHash(HASH_TABLE H)
{
    int             t;
    I_HASH         *IH = (I_HASH *) H;

    for (t = 0; t < IH->Size; ++t)
	ihZapHashElem(IH->Table[t]);
    CheckFree(IH->Table);
    CheckFree(IH);
    return;
}

int             hPut(HASH_TABLE H, const unsigned char *key, void *data)
{
    int             h;
    int             r;
    I_HASH         *IH = (I_HASH *) H;
    I_HASH_ELEM    *E = (I_HASH_ELEM *) CheckCalloc(1, sizeof(I_HASH_ELEM));

    if (IH->CaseSensitive)
	h = ihZHashFuncCaseSensitive(key) % IH->Size;
    else
	h = ihZHashFuncCaseInSensitive(key) % IH->Size;

    if (IH->Table[h])
	r = 1;
    else
	r = 0;

    E->Key = CheckStrdup(key);
    E->Data = data;
    E->Next = IH->Table[h];
    IH->Table[h] = E;

    msgMessage(MSG_DEBUG, "Hash::Debug: Added element { \"%s\", %p } to hash %p [r=%d, h=%d]",
	       key, data, IH, r, h);

    return r;
}

void           *hGet(HASH_TABLE H, const unsigned char *key)
{
    int             h;
    I_HASH         *IH = (I_HASH *) H;
    I_HASH_ELEM    *E;

    if (IH->CaseSensitive)
	h = ihZHashFuncCaseSensitive(key) % IH->Size;
    else
	h = ihZHashFuncCaseInSensitive(key) % IH->Size;

    for (E = IH->Table[h]; E; E = E->Next)
	if ((IH->CaseSensitive && !strcmp(key, E->Key))
	    || (!IH->CaseSensitive && !strcasecmp(key, E->Key))) {
	    msgMessage(MSG_DEBUG, "Hash::Debug: hGet(%p, \"%s\") yields { \"%s\", %p }",
		       IH, key, E->Key, E->Data);
	    return E->Data;
	}
    msgMessage(MSG_ERROR, "Hash: hGet(%p, \"%s\") failed", IH, key);
    return NULL;
}

/*
 * Probably some ideas in this code have been originally developed 
 * by Robert Jenkins and Thomas Wang. 
 * More probably, I'm using them in the wrong ways...
 */
long int        hHashFunctionInt2(long int key)
{
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    return key;
}

long int        hHashFunctionPointer(void *key, long int link)
{
    return link ^ (((long int)key >> 3) * 2654435761ul);
}

long int        hHashFunctionMem(void *mem, size_t length, long int link)
{
    long int        a = 2654435761ul;
    long int        b = 2654435761ul;
    long int        c = link;
    size_t          len = length;
    unsigned char  *k = mem;

    /*
     * handle most of the key 
     */
    while (len >= 12) {
	a = a + (k[0] + ((long int)k[1] << 8) + ((long int)k[2] << 16) + ((long int)k[3] << 24));
	b = b + (k[4] + ((long int)k[5] << 8) + ((long int)k[6] << 16) + ((long int)k[7] << 24));
	c = c + (k[8] + ((long int)k[9] << 8) + ((long int)k[10] << 16) + ((long int)k[11] << 24));
	mix(a, b, c);
	k = k + 12;
	len = len - 12;
    }

    /* 
     * handle the last 11 bytes
     */
    c = c + length;
    switch (len) {
    case 11:
	c = c + ((long int)k[10] << 24);
    case 10:
	c = c + ((long int)k[9] << 16);
    case 9:
	c = c + ((long int)k[8] << 8);
    case 8:
	b = b + ((long int)k[7] << 24);
    case 7:
	b = b + ((long int)k[6] << 16);
    case 6:
	b = b + ((long int)k[5] << 8);
    case 5:
	b = b + k[4];
    case 4:
	a = a + ((long int)k[3] << 24);
    case 3:
	a = a + ((long int)k[2] << 16);
    case 2:
	a = a + ((long int)k[1] << 8);
    case 1:
	a = a + k[0];
    case 0:
	;
    }
    mix(a, b, c);

    return c;
}

long int        hHashFunctionInt(long int key, long int link)
{
    return hHashFunctionMem(&key, sizeof(long int), link);
}
