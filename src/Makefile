################################################################-*-Makefile-*-
#               #                                                            #
#    #####      #  Giovanni Squillero, Ph.D.                                 #
#   ######      #  Politecnico di Torino - Dip. Automatica e Informatica     #
#   ###   \     #  Cso Duca degli Abruzzi 24 / I-10129 TORINO / ITALY        #
#    ##G  c\    #                                                            #
#    #     _\   #  Tel: +39-011564.7092  /  Fax: +39-011564.7099             #
#    |   _/     #  email: giovanni.squillero@polito.it                       #
#    |  _/      #  www  : http://www.cad.polito.it/home/squillero/           #
#               #                                                            #
##############################################################################
#
#   $Source: /home/squiller/tools/uGP/RCS/Makefile,v $
# $Revision: 1.4 $
#     $Date: 2004/01/23 11:19:08 $ 
#
##############################################################################
#                                                                            #
#  Copyright (c) 2002-2003 Giovanni Squillero                                #
#                                                                            #
#  This  program  is   free  software;   you can  redistribute   it  and/or  #
#  modify  it under   the terms  of  the  GNU General   Public License   as  #
#  published by   the Free  Software  Foundation; either version  2 of  the  #
#  License,  or (at your option) any later version.                          #
#                                                                            #
#  This program  is distributed in the  hope   that it will be useful,  but  #
#  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty    of  #
#  MERCHANTABILITY  or  FITNESS FOR   A  PARTICULAR PURPOSE.   See  the GNU  #
#  General   PublicLicensefor  more details                                  #
#                                                                            #
##############################################################################

# This is the name of your project
MAIN_TARGET = ugp
MAIN_TARGET_PURE = $(MAIN_TARGET)-pure

OBJECTS = GPL.o Memory.o Heapsort.o Messages.o String.o Hash.o Stack.o Queue.o InstructionLibrary.o Graph.o Fitness.o Entropy.o Genetic.o Operators.o getenv.o setenv.o getopt.o getopt1.o
HEADERS = GPL.h ugp.h gmacros.h Check.h Memory.h Heapsort.h Messages.h String.h Hash.h Stack.h Queue.h InstructionLibrary.h Graph.h Fitness.h Entropy.h Genetic.h Operators.h
FILES = $(OBJECTS:.o=.c) $(HEADERS) ugp.c ugp-dump.c
DISTRIBUITION_FILES = $(FILES) Makefile GetVersion.pl GPL.txt

# Uncomment to use dmalloc
#LIBS = -lm -ldmalloc
LIBS = -lm

# -----------------------------------------------------------------------------

# Some are standards
CC = gcc
CHMOD = chmod
CP = cp -p
INSTALL = install
LN = ln -sf
MKDIR = mkdir
PRINT = echo
RM = rm -f
RMDIR = rm -Rf
SHELL = /bin/sh

# Prefix used in constructing the default values of the variables listed below.
prefix = /home/tesi/squillero

# Prefix used in constructing the default values of some of the variables
# listed below.
exec_prefix = $(prefix)

# Directory for installing executable programs that users can run.
bindir = $(exec_prefix)/bin

# Directory for installing the man pages.
mandir = $(prefix)/man/man1

# Directory for installing libs
libdir = $(prefix)/lib

# Directory for installing includes
incdir = $(prefix)/include

# Directory for installing distribution tar file
distdir = $(prefix)/sources

# -----------------------------------------------------------------------------

# Local include and lib directory (for fault2.h and create...)
#LIBPATH = -L/usr/local/dmalloc
#INCLUDEPATH = -I. -I/usr/local/dmalloc
INCLUDEPATH = -I.

# Options
# Uncomment to use dmalloc
#STANDARD = -DDMALLOC -DDMALLOC_FUNC_CHECK -ansi -Wall $(INCLUDEPATH) -DNEED_SOME_PROTOS -DLINUX
STANDARD = -ansi -Wall $(INCLUDEPATH) -DNEED_SOME_PROTOS -DLINUX -g
#STANDARD = -ansi -Wall $(INCLUDEPATH) -DNEED_SOME_PROTOS
OPTIMIZE = -O99
PROFILE = $(OPTIMIZE) -pg
DEBUG = -O -DGS_DEBUG

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -              NO MODIFICATIONS NEEDED AFTER THIS POINT                     -
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

#CFLAGS = $(STANDARD) $(DEBUG)
#CFLAGS = $(STANDARD) $(PROFILE)
CFLAGS = $(STANDARD) $(OPTIMIZE)
#CFLAGS = $(STANDARD)

# -----------------------------------------------------------------------------

default: $(MAIN_TARGET)
#default: $(MAIN_TARGET)-exp
all: TAGS $(MAIN_TARGET) $(MAIN_TARGET_PURE)
new: default

# -----------------------------------------------------------------------------

# object
.c.o:
	$(CC) $(CFLAGS) -c $<

# assembler
.c.S:
	$(CC) $(CFLAGS) -S -o $@ $<

# pre-process only
.c.E:
	$(CC) $(CFLAGS) -C -E -o $@ $<

#insure
%-ins++.o: %.c
	insure $(STANDARD) -g -c -o $@ $<


# -----------------------------------------------------------------------------

$(MAIN_TARGET): $(OBJECTS) ugp.c
	$(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ $(LIBS)

$(MAIN_TARGET)-exp: $(OBJECTS:.o=.c) ugp.c
	$(CC) -o $@ \
	$(LIBPATH) $(STANDARD) $(DEBUG) -fprofile-arcs -ftest-coverage \
	-DUGP_FLAGS="\"$(STANDARD) $(DEBUG) -fprofile-arcs -ftest-coverage\"" \
	-DUGP_LINKED_BY="\"${USER}\"" \
	-DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ \
	$(LIBS)

$(MAIN_TARGET)-pure: $(OBJECTS) ugp.c
	purify $(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ $(LIBS)

$(MAIN_TARGET)-insure: $(OBJECTS:.o=-ins++.o) ugp.c 
	insure $(STANDARD) $(LIBPATH) -g -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ $(LIBS)

test-dmalloc: test-dmalloc.o ugp.c
	$(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ $(LIBS)	

ugp-dump: $(OBJECTS) ugp-dump.c
	$(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" \
	$^ $(LIBS)

TAGS: $(FILES)
	etags $^

install: ugp ugp-dump
	$(INSTALL) -m 775 ugp /cadtools/program/bin
	$(INSTALL) -m 775 ugp-dump /cadtools/program/bin

dist:
	-rm -Rf ugp-`./GetVersion.pl`
	mkdir ugp-`./GetVersion.pl`
	ln $(DISTRIBUITION_FILES) ugp-`./GetVersion.pl`
	chown -R squiller:cadcad ugp-`./GetVersion.pl`
	chmod 644 ugp-`./GetVersion.pl`/Makefile
	chmod 644 ugp-`./GetVersion.pl`/*.[ch]
	chmod 755 ugp-`./GetVersion.pl`/GetVersion.pl
	chmod 444 ugp-`./GetVersion.pl`/GPL.txt
	tar cvfz ugp-`./GetVersion.pl`.tar.gz ugp-`./GetVersion.pl`
	chown -R squiller:cadcad ugp-`./GetVersion.pl`.tar.gz
	-rm -Rf ugp-`./GetVersion.pl`

# -----------------------------------------------------------------------------
# DEPENDENCIES
# -----------------------------------------------------------------------------
Entropy.o: Makefile $(HEADERS) 
Fitness.o: Makefile $(HEADERS) 
Genetic.o: Makefile $(HEADERS) 
Graph.o: Makefile $(HEADERS) 
Hash.o: Makefile $(HEADERS) 
InstructionLibrary.o: Makefile $(HEADERS) 
Memory.o: Makefile $(HEADERS) 
Messages.o: Makefile $(HEADERS) 
Operators.o: Makefile $(HEADERS) 
Queue.o: Makefile $(HEADERS) 
Stack.o: Makefile $(HEADERS) 
