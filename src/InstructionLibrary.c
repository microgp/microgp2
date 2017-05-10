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
*   $Source: /home/squiller/tools/uGP/RCS/InstructionLibrary.c,v $
* $Revision: 1.8 $
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
static void     iilFreeParameter(IL_PARAMETER * O);
static void     iilFreeMacro(IL_MACRO * M);
static void     iilFreeILSection(IL_SECTION * S);
static void     iilCookFormat(char *label);
static int      iilAddSectionToCurrentIL(const char *tag);
void            iilInfoSection(IL_SECTION * S);
static IL_PARAMETER *iilReadParameter(const char *opstring);
static IL_MACRO *iilReadMacro(FILE * Fin);
static char    *iilReadNextLine(FILE * Fin);
static int      iilTokenize(const char *line);

/****************************************************************************/
/*                          V A R I A B L E S                               */
/****************************************************************************/

static STACK   *ilParameterList;
static IL_LIBRARY *ilCurrentLibrary;
static HASH_TABLE *ilSystem;
static HASH_TABLE *ilUser;
static IL_MACRO EmptyPrologue = {
    -1,				/* int id */
    0,				/* int Probability */
    NULL,			/* char *LabelFormat */
    NULL,			/* char *RegExp */
    NULL,			/* char *Text */
    0,				/* int  nParameters */
    NULL,			/* IL_PARAMETER **Parameter */
    NULL			/* struct _IL_SECTION *Section */
};
static IL_MACRO EmptyEpilogue = {
    -2,				/* int id */
    0,				/* int Probability */
    NULL,			/* char *LabelFormat */
    NULL,			/* char *RegExp */
    NULL,			/* char *Text */
    0,				/* int  nParameters */
    NULL,			/* IL_PARAMETER **Parameter */
    NULL			/* struct _IL_SECTION *Section */
};

/****************************************************************************/
/*                I N T E R N A L     F U N C T I O N S                     */
/****************************************************************************/

static char    *iilReadNextLine(FILE * Fin)
{
    static char     line[MAX_LINE];
    char           *l;
    int             p;

    do {	
	l = fgets(line, MAX_LINE, Fin);
	if(l) {
	    for(p=strlen(l)-1; p>=0 && (isspace((int)line[p]) || line[p]=='\n'); --p)
		line[p] = '\n', line[p+1] = '\0';
	}
    } while (l && ((line[0] == ';' && line[1] == '-') || line[0] == '\n'));
    return l;
}


static void     iilCookFormat(char *label)
{
    char            ret[] = { '\n', '\0' };

    SearchReplace(label, "\\n", ret);
}

static int      iilTokenize(const char *line)
{
    if (*line != '.')
	return TOKEN_DATA;
    else if (!strncasecmp(line, TSTRING_SECTION, strlen(TSTRING_SECTION)))
	return TOKEN_SECTION;
    else if (!strncasecmp(line, TSTRING_TYPE, strlen(TSTRING_TYPE)))
	return TOKEN_TYPE;
    else if (!strncasecmp(line, TSTRING_MACRO, strlen(TSTRING_MACRO)))
	return TOKEN_MACRO;
    else if (!strncasecmp(line, TSTRING_ENDMACRO, strlen(TSTRING_ENDMACRO)))
	return TOKEN_ENDMACRO;
    else if (!strncasecmp(line, TSTRING_PROLOGUE, strlen(TSTRING_PROLOGUE)))
	return TOKEN_PROLOGUE;
    else if (!strncasecmp(line, TSTRING_ENDPROLOGUE, strlen(TSTRING_ENDPROLOGUE)))
	return TOKEN_ENDPROLOGUE;
    else if (!strncasecmp(line, TSTRING_GLOBALPROLOGUE, strlen(TSTRING_GLOBALPROLOGUE)))
	return TOKEN_GLOBALPROLOGUE;
    else if (!strncasecmp(line, TSTRING_ENDGLOBALPROLOGUE, strlen(TSTRING_ENDGLOBALPROLOGUE)))
	return TOKEN_ENDGLOBALPROLOGUE;
    else if (!strncasecmp(line, TSTRING_EPILOGUE, strlen(TSTRING_EPILOGUE)))
	return TOKEN_EPILOGUE;
    else if (!strncasecmp(line, TSTRING_ENDEPILOGUE, strlen(TSTRING_ENDEPILOGUE)))
	return TOKEN_ENDEPILOGUE;
    else if (!strncasecmp(line, TSTRING_GLOBALEPILOGUE, strlen(TSTRING_GLOBALEPILOGUE)))
	return TOKEN_GLOBALEPILOGUE;
    else if (!strncasecmp(line, TSTRING_ENDGLOBALEPILOGUE, strlen(TSTRING_ENDGLOBALEPILOGUE)))
	return TOKEN_ENDGLOBALEPILOGUE;
    else if (!strncasecmp(line, TSTRING_PROBABILITY, strlen(TSTRING_PROBABILITY)))
	return TOKEN_PROBABILITY;
    else if (!strncasecmp(line, TSTRING_PARAMETER, strlen(TSTRING_PARAMETER)))
	return TOKEN_PARAMETER;
    else if (!strncasecmp(line, TSTRING_LABELFORMAT, strlen(TSTRING_LABELFORMAT)))
	return TOKEN_LABELFORMAT;
    else if (!strncasecmp(line, TSTRING_REGEXP, strlen(TSTRING_REGEXP)))
	return TOKEN_REGEXP;
    else if (!strncasecmp(line, TSTRING_COMMENTFORMAT, strlen(TSTRING_COMMENTFORMAT)))
	return TOKEN_COMMENTFORMAT;
    else if (!strncasecmp(line, TSTRING_ALLOWMULTIPLE, strlen(TSTRING_ALLOWMULTIPLE)))
	return TOKEN_ALLOWMULTIPLE;
    else if (!strncasecmp(line, TSTRING_ALWAYSDUMP, strlen(TSTRING_ALWAYSDUMP)))
	return TOKEN_ALWAYSDUMP;
    else
	msgMessage(MSG_ERROR, "Syntax error (unknown token) in line: \"%s\"", line);
    return TOKEN_ILLEGAL;
}


static IL_MACRO *iilReadMacro(FILE * Fin)
{
    char           *_FunctionName = "iilReadMacro";
    IL_MACRO       *ilMacro;
    char           *l;
    char            Text[MAX_MACRO_SIZE] = "";
    char            tmp[TMP_SIZE];

    msgMessage(MSG_DEBUG, "iilReadMacro::Debug: entering iilReadMacro()");
    ilMacro = (IL_MACRO *) CheckCalloc(1, sizeof(IL_MACRO));
    ilMacro->Probability = 1;
    while ((l = iilReadNextLine(Fin))) {
	Chop(l);
	switch (iilTokenize(l)) {
	case TOKEN_PROBABILITY:
	    CheckTrue(sscanf(l, TSTRING_PROBABILITY " %d", &ilMacro->Probability) == 1);
	    CheckTrue(ilMacro->Probability >= 0);
	    msgMessage(MSG_DEBUG, "iilReadMacro::Debug: macro Probability set to: %d",
		       ilMacro->Probability);
	    break;
	case TOKEN_LABELFORMAT:
	    CheckTrue(sscanf(l, TSTRING_LABELFORMAT " \"%[^\"]\"", tmp) == 1);
	    iilCookFormat(tmp);
	    ilMacro->LabelFormat = CheckStrdup(tmp);
	    msgMessage(MSG_DEBUG, "iilReadMacro::Debug: macro LabelFormat set to: \"%s\"",
		       ilMacro->LabelFormat);
	    break;
	case TOKEN_REGEXP:
	    ilMacro->RegExp = CheckStrdup(&l[strlen(TSTRING_REGEXP)+1]);
	    msgMessage(MSG_DEBUG, "iilReadMacro::Debug: macro regexp set to: \"%s\"",
		       ilMacro->RegExp);
	    break;
	case TOKEN_DATA:
	    strcat(Text, l);
	    strcat(Text, "\n");
	    break;
	case TOKEN_ENDMACRO:
	case TOKEN_ENDPROLOGUE:
	case TOKEN_ENDGLOBALPROLOGUE:
	case TOKEN_ENDEPILOGUE:
	case TOKEN_ENDGLOBALEPILOGUE:
	    msgMessage(MSG_DEBUG, "iilReadMacro::Debug: iilreadmacro() completed");
	    ilMacro->Text = CheckStrdup(Text);
	    return ilMacro;
	    break;
	case TOKEN_PARAMETER:
	    ilMacro->Parameter = (IL_PARAMETER **) CheckRealloc(ilMacro->Parameter,
								(ilMacro->nParameters +
								 1) * sizeof(IL_PARAMETER *));
	    ilMacro->Parameter[ilMacro->nParameters] =
		iilReadParameter(&l[sizeof(TSTRING_PARAMETER)]);
	    ++ilMacro->nParameters;
	    break;
	default:
	    msgMessage(MSG_ERROR, "Syntax error (token %d not allowed here) in line: \"%s\"",
		       iilTokenize(l), l);
	    break;
	}
    }
    msgMessage(MSG_ERROR, "Syntax error (EOF)");
    return NULL;
}

static IL_PARAMETER *iilReadParameter(const char *opstring)
{
    char           *_FunctionName = "ilReadParameter";
    IL_PARAMETER   *O;
    char            tmp[TMP_SIZE];
    int             t;

    O = (IL_PARAMETER *) CheckCalloc(1, sizeof(IL_PARAMETER));
    CheckTrue(sscanf(opstring, "%s", tmp) == 1);
    CheckTrue(hCheckKey(ilSystem, tmp));
    O->Type = (int)hGet(ilSystem, tmp);
    switch (O->Type) {
    case PARAMETER_UNIQUE_TAG:
	break;
    case PARAMETER_SAVED_TYPE:
	CheckTrue(sscanf(opstring, "%*s %s", tmp));
	CheckTrue(hCheckKey(ilUser, tmp));
	CheckFree(O);		/* useless... */
	O = (IL_PARAMETER *) hGet(ilUser, tmp);
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: type \"%s\"", tmp);
	break;
    case PARAMETER_HEX:
	CheckTrue(sscanf(opstring, "%*s %d", &O->Def.Digits) == 1);
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: hex parameter [%ld]",
		   O->Def.Digits);
	break;
    case PARAMETER_INTEGER:
	CheckTrue(sscanf(opstring, "%*s %ld %ld", &O->Def.Integer.From, &O->Def.Integer.To) == 2);
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: integer parameter [%ld %ld]",
		   O->Def.Integer.From, O->Def.Integer.To);
	break;
    case PARAMETER_CONSTANT:
	for (t = 0; opstring[t]; ++t)
	    if (opstring[t] == ' ' || opstring[t] == '\t')
		break;
	++t;
	do {
	    msgMessage(MSG_DEBUG, "ilReadParameter::Debug: scanning [%s]", &opstring[t]);
	    CheckTrue(sscanf(&opstring[t], "%s", tmp) == 1);
	    O->Def.Constant.Constant = (char **)CheckRealloc(O->Def.Constant.Constant,
							     (1 +
							      O->Def.Constant.nConstants) *

							     sizeof(char *));

	    O->Def.Constant.Constant[O->Def.Constant.nConstants] = CheckStrdup(tmp);
	    msgMessage(MSG_DEBUG, "ilReadParameter::Debug: new constant %d \"%s\"",
		       O->Def.Constant.nConstants,
		       O->Def.Constant.Constant[O->Def.Constant.nConstants]);
	    ++O->Def.Constant.nConstants;
	    for (++t; opstring[t] && opstring[t] != ' ' && opstring[t] != '\t'; ++t);
	    if (opstring[t])
		++t;
	} while (opstring[t]);
	break;

    case PARAMETER_INNER_FORWARD_LABEL:
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: inner forward label parameter");
	O->Def.Section = -1;
	break;

    case PARAMETER_INNER_BACKWARD_LABEL:
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: inner backward label parameter");
	O->Def.Section = -1;
	break;

    case PARAMETER_INNER_GENERIC_LABEL:
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: inner generic label parameter");
	O->Def.Section = -1;
	break;

    case PARAMETER_OUTER_LABEL:
	CheckTrue(sscanf(opstring, "%*s %s", tmp) == 1);
	msgMessage(MSG_DEBUG, "ilReadParameter::Debug: outer label parameter to section \"%s\"",
		   tmp);
	if (!hCheckKey(ilUser, tmp))
	    iilAddSectionToCurrentIL(tmp);
	O->Def.Section = (int)hGet(ilUser, tmp);
	break;

    default:
	msgMessage(MSG_ERROR, "Syntax error (unknown parameter type) in line: \"%s\"", opstring);
    }

    return O;
}

void            iilInfoSection(IL_SECTION * S)
{
    msgMessage(MSG_INFO, "section \"%s\": %d macro%s",
	       S->Tag, S->nMacros, S->nMacros == 1 ? "" : "s");
}


static int      iilAddSectionToCurrentIL(const char *tag)
{
    IL_SECTION     *Sec;

    hPut(ilUser, tag, (void *)ilCurrentLibrary->nSections);

    Sec = (IL_SECTION *) CheckCalloc(1, sizeof(IL_SECTION));
    Sec->Tag = CheckStrdup(tag);

    /***
    Sec->Prologue = (IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
    memcpy(Sec->Prologue, &EmptyPrologue, sizeof(IL_MACRO));
    Sec->Epilogue = (IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
    memcpy(Sec->Epilogue, &EmptyEpilogue, sizeof(IL_MACRO));
    ***/

    Sec->InstructionLibrary = ilCurrentLibrary;
    Sec->id = ilCurrentLibrary->nSections;

    ilCurrentLibrary->Section = (IL_SECTION **) CheckRealloc(ilCurrentLibrary->Section,
							     (1 +
							      ilCurrentLibrary->nSections) *
							     sizeof(IL_SECTION *));
    ilCurrentLibrary->Section[ilCurrentLibrary->nSections] = Sec;
    ++ilCurrentLibrary->nSections;
    return ilCurrentLibrary->nSections;
}

/****************************************************************************/
/*                          F U N C T I O N S                               */
/****************************************************************************/


void            ilInfoLibrary(IL_LIBRARY * IL)
{
    int             t;

    msgMessage(MSG_INFO, "");
    msgMessage(MSG_INFO, "Instructions library contains %d section%s",
	       IL->nSections, IL->nSections == 1 ? "" : "s");
    for (t = 0; t < IL->nSections; ++t) {
	iilInfoSection(IL->Section[t]);
    }
}

IL_LIBRARY     *ilReadInstructionLibrary(char *FileName)
{
    char           *_FunctionName = "ilReadInstructionLibrary";
    FILE           *F;
    char           *l;
    char            tmp[TMP_SIZE];
    int             CurrentSection;
    int             num;
    IL_PARAMETER   *op;
    char           *typename;

    ilCurrentLibrary = (IL_LIBRARY *) CheckCalloc(1, sizeof(IL_LIBRARY));;

    /*
     * Init parser tokens
     */
    ilSystem = hInitHash(71, HASH_KEY_NOCASE);
    ilUser = hInitHash(277, HASH_KEY_NOCASE);
    ilParameterList = sInitStack();
    hPut(ilSystem, "HEX", (void *)PARAMETER_HEX);
    hPut(ilSystem, "INTEGER", (void *)PARAMETER_INTEGER);
    hPut(ilSystem, "CONSTANT", (void *)PARAMETER_CONSTANT);
    hPut(ilSystem, "INNER_FORWARD_LABEL", (void *)PARAMETER_INNER_FORWARD_LABEL);
    hPut(ilSystem, "INNER_BACKWARD_LABEL", (void *)PARAMETER_INNER_BACKWARD_LABEL);
    hPut(ilSystem, "INNER_GENERIC_LABEL", (void *)PARAMETER_INNER_GENERIC_LABEL);
    hPut(ilSystem, "FORWARD_LABEL", (void *)PARAMETER_INNER_FORWARD_LABEL);
    hPut(ilSystem, "BACKWARD_LABEL", (void *)PARAMETER_INNER_BACKWARD_LABEL);
    hPut(ilSystem, "GENERIC_LABEL", (void *)PARAMETER_INNER_GENERIC_LABEL);
    hPut(ilSystem, "OUTER_LABEL", (void *)PARAMETER_OUTER_LABEL);
    hPut(ilSystem, "INNERFORWARDLABEL", (void *)PARAMETER_INNER_FORWARD_LABEL);
    hPut(ilSystem, "INNERBACKWARDLABEL", (void *)PARAMETER_INNER_BACKWARD_LABEL);
    hPut(ilSystem, "INNERGENERICLABEL", (void *)PARAMETER_INNER_GENERIC_LABEL);
    hPut(ilSystem, "FORWARDLABEL", (void *)PARAMETER_INNER_FORWARD_LABEL);
    hPut(ilSystem, "BACKWARDLABEL", (void *)PARAMETER_INNER_BACKWARD_LABEL);
    hPut(ilSystem, "GENERICLABEL", (void *)PARAMETER_INNER_GENERIC_LABEL);
    hPut(ilSystem, "OUTERLABEL", (void *)PARAMETER_OUTER_LABEL);
    hPut(ilSystem, "UNIQUE_TAG", (void *)PARAMETER_UNIQUE_TAG);
    hPut(ilSystem, "UNIQUETAG", (void *)PARAMETER_UNIQUE_TAG);
    hPut(ilSystem, "TYPE", (void *)PARAMETER_SAVED_TYPE);

    /*
     * some defauylts
     */
    ilCurrentLibrary->CommentFormat = CheckStrdup(DEFAULT_COMMENT_FORMAT);
    ilCurrentLibrary->LabelFormat = CheckStrdup(DEFAULT_COMMENT_FORMAT);

    ilCurrentLibrary->GlobalPrologue = (IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
    memcpy(ilCurrentLibrary->GlobalPrologue, &EmptyPrologue, sizeof(IL_MACRO));
    ilCurrentLibrary->GlobalEpilogue = (IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
    memcpy(ilCurrentLibrary->GlobalEpilogue, &EmptyEpilogue, sizeof(IL_MACRO));

    CheckTrue(F = fopen(FileName, "r"));
    CurrentSection = -1;
    while ((l = iilReadNextLine(F))) {
	Chop(l);
	switch (iilTokenize(l)) {

	case TOKEN_COMMENTFORMAT:
	    CheckTrue(CurrentSection == -1);
	    CheckTrue(sscanf(l, TSTRING_COMMENTFORMAT " \"%[^\"]\"", tmp) == 1);
	    iilCookFormat(tmp);
	    CheckFree(ilCurrentLibrary->CommentFormat);
	    ilCurrentLibrary->CommentFormat = CheckStrdup(tmp);
	    msgMessage(MSG_DEBUG, "ilReadInstructionLibrary::Debug: CommentFormat set to: \"%s\"",
		       ilCurrentLibrary->CommentFormat);
	    break;

	case TOKEN_LABELFORMAT:
	    CheckTrue(sscanf(l, TSTRING_LABELFORMAT " \"%[^\"]\"", tmp) == 1);
	    iilCookFormat(tmp);
	    if (CurrentSection == -1) {
		CheckFree(ilCurrentLibrary->LabelFormat);
		ilCurrentLibrary->LabelFormat = CheckStrdup(tmp);
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: default LabelFormat set to: \"%s\"",
			   ilCurrentLibrary->LabelFormat);
	    } else {
		CheckFree(ilCurrentLibrary->Section[CurrentSection]->LabelFormat);
		ilCurrentLibrary->Section[CurrentSection]->LabelFormat = CheckStrdup(tmp);
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: section LabelFormat set to: \"%s\"",
			   ilCurrentLibrary->Section[CurrentSection]->LabelFormat);
	    }
	    break;

	case TOKEN_TYPE:
	    CheckTrue(CurrentSection == -1);
	    CheckTrue(sscanf(l, TSTRING_TYPE " %s", tmp) == 1);
	    typename = CheckStrdup(tmp);
	    CheckTrue(sscanf(l, TSTRING_TYPE " %*s %[^\v]", tmp) == 1);
	    msgMessage(MSG_DEBUG,
		       "ilReadInstructionLibrary::Debug: Adding parameter type \"%s\" -> [%s]",
		       typename, tmp);
	    op = iilReadParameter(tmp);
	    hPut(ilUser, typename, op);
	    sPushElement(ilParameterList, op, STACK_DONT_ALLOW_DUPLICATE_ELEMENTS);
	    CheckFree(typename);
	    break;

	case TOKEN_ALLOWMULTIPLE:
	    CheckTrue(CurrentSection != -1);
	    ilCurrentLibrary->Section[CurrentSection]->AllowMultiple = 1;
	    break;

	case TOKEN_ALWAYSDUMP:
	    CheckTrue(CurrentSection != -1);
	    ilCurrentLibrary->Section[CurrentSection]->AlwaysDump = 1;
	    break;

	case TOKEN_SECTION:
	    CheckTrue(sscanf(l, TSTRING_SECTION " %s", tmp) == 1);
	    if (!hCheckKey(ilUser, tmp))
		iilAddSectionToCurrentIL(tmp);
	    CurrentSection = (int)hGet(ilUser, tmp);
	    ilCurrentLibrary->Section[CurrentSection]->AllowMultiple = 0;
	    ilCurrentLibrary->Section[CurrentSection]->AlwaysDump = 0;
	    ilCurrentLibrary->Section[CurrentSection]->LabelFormat =
		CheckStrdup(DEFAULT_LABEL_FORMAT);

	    ilCurrentLibrary->Section[CurrentSection]->Prologue = 
		(IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
	    memcpy(ilCurrentLibrary->Section[CurrentSection]->Prologue,
		   &EmptyPrologue, sizeof(IL_MACRO));
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->Section = ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->InstructionLibrary = ilCurrentLibrary;

	    ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue = 
		(IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
	    memcpy(ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue,
		   &EmptyPrologue, sizeof(IL_MACRO));
	    ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->Section = ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->InstructionLibrary = ilCurrentLibrary;

	    ilCurrentLibrary->Section[CurrentSection]->Epilogue =
		(IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
	    memcpy(ilCurrentLibrary->Section[CurrentSection]->Epilogue,
		   &EmptyEpilogue, sizeof(IL_MACRO));
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->Section = ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->InstructionLibrary = ilCurrentLibrary;

	    ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue =
		(IL_MACRO *)CheckMalloc(sizeof(IL_MACRO));
	    memcpy(ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue,
		   &EmptyEpilogue, sizeof(IL_MACRO));
	    ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->Section = ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->InstructionLibrary = ilCurrentLibrary;

	    msgMessage(MSG_DEBUG, "ilReadInstructionLibrary::Debug: currently in section %d \"%s\"",
		       CurrentSection, tmp);
	    break;

	case TOKEN_GLOBALPROLOGUE:
	    if(CurrentSection==-1) {
		iilFreeMacro(ilCurrentLibrary->GlobalPrologue);
		ilCurrentLibrary->GlobalPrologue = iilReadMacro(F);
		ilCurrentLibrary->GlobalPrologue->Section = NULL;
		ilCurrentLibrary->GlobalPrologue->InstructionLibrary = ilCurrentLibrary;
		ilCurrentLibrary->GlobalPrologue->id = -1;
		ilCurrentLibrary->GlobalPrologue->Probability = 0;
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: added main GlobalPrologue");
	    } else {
		iilFreeMacro(ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue);
		ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue = iilReadMacro(F);
		ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->Section =
		    ilCurrentLibrary->Section[CurrentSection];
		ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->InstructionLibrary = ilCurrentLibrary;
		ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->id = -1;
		ilCurrentLibrary->Section[CurrentSection]->GlobalPrologue->Probability = 0;
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: added GlobalPrologue to section %d (\"%s\")",
			   CurrentSection, ilCurrentLibrary->Section[CurrentSection]->Tag);
	    }	    
	    break;

	case TOKEN_GLOBALEPILOGUE:
	    if(CurrentSection==-1) {
		iilFreeMacro(ilCurrentLibrary->GlobalEpilogue);
		ilCurrentLibrary->GlobalEpilogue = iilReadMacro(F);
		ilCurrentLibrary->GlobalEpilogue->Section = NULL;
		ilCurrentLibrary->GlobalEpilogue->InstructionLibrary = ilCurrentLibrary;
		ilCurrentLibrary->GlobalEpilogue->id = -1;
		ilCurrentLibrary->GlobalEpilogue->Probability = 0;
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: added main GlobalEpilogue");
	    } else {
		iilFreeMacro(ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue);
		ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue = iilReadMacro(F);
		ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->Section =
		    ilCurrentLibrary->Section[CurrentSection];
		ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->InstructionLibrary = ilCurrentLibrary;
		ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->id = -1;
		ilCurrentLibrary->Section[CurrentSection]->GlobalEpilogue->Probability = 0;
		msgMessage(MSG_DEBUG,
			   "ilReadInstructionLibrary::Debug: added GlobalEpilogue to section %d (\"%s\")",
			   CurrentSection, ilCurrentLibrary->Section[CurrentSection]->Tag);
	    }
	    break;

	case TOKEN_PROLOGUE:
	    CheckTrue(CurrentSection!=-1);
	    iilFreeMacro(ilCurrentLibrary->Section[CurrentSection]->Prologue);
	    ilCurrentLibrary->Section[CurrentSection]->Prologue = iilReadMacro(F);
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->Section =
		ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->InstructionLibrary = ilCurrentLibrary;
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->id = -1;
	    ilCurrentLibrary->Section[CurrentSection]->Prologue->Probability = 0;
	    msgMessage(MSG_DEBUG,
		       "ilReadInstructionLibrary::Debug: added Prologue to section %d (\"%s\")",
		       CurrentSection, ilCurrentLibrary->Section[CurrentSection]->Tag);
	    break;

	case TOKEN_EPILOGUE:
	    CheckTrue(CurrentSection!=-1);
	    iilFreeMacro(ilCurrentLibrary->Section[CurrentSection]->Epilogue);
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue = iilReadMacro(F);
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->Section =
		ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->InstructionLibrary = ilCurrentLibrary;
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->id = -2;
	    ilCurrentLibrary->Section[CurrentSection]->Epilogue->Probability = 0;
	    msgMessage(MSG_DEBUG,
		       "ilReadInstructionLibrary::Debug: added Epilogue to section %d (\"%s\")",
		       CurrentSection, ilCurrentLibrary->Section[CurrentSection]->Tag);
	    break;

	case TOKEN_MACRO:
	    CheckTrue(CurrentSection != -1);
	    num = ilCurrentLibrary->Section[CurrentSection]->nMacros;
	    ilCurrentLibrary->Section[CurrentSection]->Macro =
		(IL_MACRO **) CheckRealloc(ilCurrentLibrary->Section[CurrentSection]->Macro,
					   (1 + num) * sizeof(IL_MACRO *));
	    ilCurrentLibrary->Section[CurrentSection]->Macro[num] = iilReadMacro(F);
	    ilCurrentLibrary->Section[CurrentSection]->Macro[num]->Section =
		ilCurrentLibrary->Section[CurrentSection];
	    ilCurrentLibrary->Section[CurrentSection]->Macro[num]->InstructionLibrary = ilCurrentLibrary;
	    ilCurrentLibrary->Section[CurrentSection]->Macro[num]->id = num;
	    msgMessage(MSG_DEBUG,
		       "ilReadInstructionLibrary::Debug: added macro %d to section %d (\"%s\")",
		       ilCurrentLibrary->Section[CurrentSection]->nMacros, CurrentSection,
		       ilCurrentLibrary->Section[CurrentSection]->Tag);
	    ++ilCurrentLibrary->Section[CurrentSection]->nMacros;
	    break;
	default:
	    msgMessage(MSG_ERROR, "Syntax error (token %d not allowed here) in line: \"%s\"",
		       iilTokenize(l), l);
	    break;
	}
    }

    fclose(F);
    hFreeHash(ilSystem);
    hFreeHash(ilUser);
    ilSystem = NULL;
    ilUser = NULL;
    return ilCurrentLibrary;
}

void            ilFreeInstructionLibrary(IL_LIBRARY * IL)
{
    int             t;
    IL_PARAMETER   *OP;

    msgMessage(MSG_DEBUG, "ilFreeInstructionLibrary::Debug: Trying to free instruction library %p",
	       IL);
    /*
     * build a stack of op to be deleted
     */
    CheckFree(IL->LabelFormat);
    CheckFree(IL->CommentFormat);
    iilFreeMacro(IL->GlobalPrologue);
    iilFreeMacro(IL->GlobalEpilogue);
    for (t = 0; t < IL->nSections; ++t)
	iilFreeILSection(IL->Section[t]);
    CheckFree(IL->Section);

    CheckFree(IL);

    while (!sStackEmpty(ilParameterList)) {
	OP = sPopElement(ilParameterList);
	iilFreeParameter(OP);
    }
    sFreeStack(ilParameterList);

    msgMessage(MSG_DEBUG, "ilFreeInstructionLibrary::Debug: All done");
}

static void     iilFreeILSection(IL_SECTION * S)
{
    int             t;

    msgMessage(MSG_DEBUG, "iilFreeILSection::Debug: Trying to free section %p (\"%s\")", S, S->Tag);
    CheckFree(S->LabelFormat);
    iilFreeMacro(S->Prologue);
    iilFreeMacro(S->Epilogue);
    CheckFree(S->Tag);
    for (t = 0; t < S->nMacros; ++t)
	iilFreeMacro(S->Macro[t]);
    CheckFree(S->Macro);
    msgMessage(MSG_DEBUG, "iilFreeILSection::Debug: All done");
    CheckFree(S);
}

static void     iilFreeMacro(IL_MACRO * M)
{
    int             t;

    msgMessage(MSG_DEBUG, "iilFreeMacro::Debug: Trying to free macro %p", M);
    CheckFree(M->LabelFormat);
    CheckFree(M->Text);
    for (t = 0; t < M->nParameters; ++t) {
	msgMessage(MSG_DEBUG, "iilFreeMacro::Debug: Flagging parameter %p to be deleted",
		   M->Parameter[t]);
	sPushElement(ilParameterList, M->Parameter[t], STACK_DONT_ALLOW_DUPLICATE_ELEMENTS);
    }
    CheckFree(M->Parameter);
    CheckFree(M);
    msgMessage(MSG_DEBUG, "iilFreeMacro::Debug: All done");
}

static void     iilFreeParameter(IL_PARAMETER * O)
{
    int             t;

    if (O->Type == PARAMETER_CONSTANT) {
	for (t = 0; t < O->Def.Constant.nConstants; ++t) {
	    msgMessage(MSG_DEBUG, "iilFreeParameter::Debug: Releasing const \"%s\"",
		       O->Def.Constant.Constant[t]);
	    CheckFree(O->Def.Constant.Constant[t]);
	}
	CheckFree(O->Def.Constant.Constant);
    }
    CheckFree(O);
    msgMessage(MSG_DEBUG, "iilFreeParameter::Debug: All done");
}

int             ilIsParameterInnerLabel(IL_PARAMETER * OP)
{
    if (OP->Type == PARAMETER_INNER_FORWARD_LABEL)
	return 1;
    if (OP->Type == PARAMETER_INNER_BACKWARD_LABEL)
	return 1;
    if (OP->Type == PARAMETER_INNER_GENERIC_LABEL)
	return 1;
    return 0;
}

int             ilIsParameterOuterLabel(IL_PARAMETER * OP)
{
    if (OP->Type == PARAMETER_OUTER_LABEL)
	return 1;
    return 0;
}

int             ilIsParameterLabel(IL_PARAMETER * OP)
{
    if (ilIsParameterInnerLabel(OP))
	return 1;
    if (ilIsParameterOuterLabel(OP))
	return 1;
    return 0;
}

int             ilWeightedGetRandomMacro(IL_LIBRARY * IL, int sec)
{
    char           *_FunctionName = "ilWeightedGetRandomMacro";
    int             sum;
    int             t;
    int             rnd;

    sum = 0;
    for (t = 0; t < IL->Section[sec]->nMacros; ++t)
	sum += IL->Section[sec]->Macro[t]->Probability;
    rnd = lrand48() % sum;
    msgMessage(MSG_DEBUG, "nMacros: %d (%d) -> rnd: %d", IL->Section[sec]->nMacros, sum, rnd);

    t = 0;
    rnd -= IL->Section[sec]->Macro[t]->Probability;
    while(rnd >= 0) {
	++t;
	rnd -= IL->Section[sec]->Macro[t]->Probability;
    }
    msgMessage(MSG_DEBUG, "Selected: %d", t);

    CheckTrue(IL->Section[sec]->Macro[t]->Section->id == sec)
    return t;
}
