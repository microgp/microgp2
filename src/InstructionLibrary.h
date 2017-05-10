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
*   $Source: /home/squiller/tools/uGP/RCS/InstructionLibrary.h,v $
* $Revision: 1.7 $
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
