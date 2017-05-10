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
*   $Source: /home/squiller/tools/uGP/RCS/InstructionLibrary.h,v $
* $Revision: 1.7 $
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

/****************************************************************************/
/*              L O C A L   D A T A   S T R U C T U R E S                   */
/****************************************************************************/

#define DEFAULT_COMMENT_FORMAT  	"; %s\n"
#define DEFAULT_LABEL_FORMAT    	"%s:\n"

#define TSTRING_SECTION 		".section"
#define TSTRING_TYPE 			".type"
#define TSTRING_MACRO 			".macro"
#define TSTRING_REGEXP 			".re"
#define TSTRING_ENDMACRO 		".endmacro"
#define TSTRING_PROLOGUE 		".prologue"
#define TSTRING_ENDPROLOGUE 		".endprologue"
#define TSTRING_GLOBALPROLOGUE 		".globalprologue"
#define TSTRING_ENDGLOBALPROLOGUE 	".endglobalprologue"
#define TSTRING_EPILOGUE 		".epilogue"
#define TSTRING_ENDEPILOGUE 		".endepilogue"
#define TSTRING_GLOBALEPILOGUE 		".globalepilogue"
#define TSTRING_ENDGLOBALEPILOGUE 	".endglobalepilogue"
#define TSTRING_PROBABILITY		".probability"
#define TSTRING_PARAMETER 		".parameter"
#define TSTRING_LABELFORMAT		".labelformat"
#define TSTRING_COMMENTFORMAT		".commentformat"
#define TSTRING_ALLOWMULTIPLE		".allowmultiple"
#define TSTRING_ALWAYSDUMP		".alwaysdump"

enum {
    TOKEN_ILLEGAL = -1,
    TOKEN_DATA = 0,
    TOKEN_TYPE,
    TOKEN_SECTION,
    TOKEN_MACRO, TOKEN_ENDMACRO,
    TOKEN_PROLOGUE, TOKEN_ENDPROLOGUE,
    TOKEN_GLOBALPROLOGUE, TOKEN_ENDGLOBALPROLOGUE,
    TOKEN_EPILOGUE, TOKEN_ENDEPILOGUE,
    TOKEN_GLOBALEPILOGUE, TOKEN_ENDGLOBALEPILOGUE,
    TOKEN_PROBABILITY,
    TOKEN_PARAMETER,
    TOKEN_LABELFORMAT,
    TOKEN_REGEXP,
    TOKEN_COMMENTFORMAT,
    TOKEN_ALLOWMULTIPLE,
    TOKEN_ALWAYSDUMP
};

enum {
    PARAMETER_UNKNOWN = 0,
    PARAMETER_HEX,
    PARAMETER_INTEGER,
    PARAMETER_CONSTANT,
    PARAMETER_INNER_FORWARD_LABEL,
    PARAMETER_INNER_BACKWARD_LABEL,
    PARAMETER_INNER_GENERIC_LABEL,
    PARAMETER_OUTER_LABEL,
    PARAMETER_UNIQUE_TAG,
    PARAMETER_SAVED_TYPE
};

typedef struct _IL_PARAMETER {
    int             Type;
    union {
	int             Digits;
	struct {
	    long            From;
	    long            To;
	} Integer;
	struct {
	    int             nConstants;
	    char          **Constant;
	} Constant;
	int             Section;
    } Def;
} IL_PARAMETER;

typedef struct _IL_MACRO {
    int             id;
    int             Probability;
    char           *LabelFormat;
    char           *RegExp;
    char           *Text;
    int             nParameters;
    IL_PARAMETER    **Parameter;
    /* back link */
    struct _IL_SECTION *Section;
    struct _IL_LIBRARY *InstructionLibrary;
} IL_MACRO;

typedef struct _IL_SECTION {
    int             id;
    int             AllowMultiple;
    int             AlwaysDump;
    char           *LabelFormat;
    IL_MACRO       *GlobalPrologue;
    IL_MACRO       *GlobalEpilogue;
    IL_MACRO       *Prologue;
    IL_MACRO       *Epilogue;
    char           *Tag;
    int             nMacros;
    IL_MACRO      **Macro;
    /* back link */
    struct _IL_LIBRARY *InstructionLibrary;
} IL_SECTION;

typedef struct _IL_LIBRARY {
    char           *LabelFormat;
    char           *CommentFormat;
    int             nSections;
    IL_SECTION    **Section;
    IL_MACRO       *GlobalPrologue;
    IL_MACRO       *GlobalEpilogue;
} IL_LIBRARY;

IL_LIBRARY     *ilReadInstructionLibrary(char *FileName);
void            ilInfoLibrary(IL_LIBRARY * IL);
void            ilFreeInstructionLibrary(IL_LIBRARY * IL);
int             ilIsParameterInnerLabel(IL_PARAMETER * OP);
int             ilIsParameterOuterLabel(IL_PARAMETER * OP);
int             ilIsParameterLabel(IL_PARAMETER * OP);
int             ilWeightedGetRandomMacro(IL_LIBRARY * IL, int sec);
