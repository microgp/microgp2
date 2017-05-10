################################################################-*-Makefile-*-
#               #                                                            #
#    #####      #  Giovanni Squillero, Ph.D.                                 #
#   ######      #  Politecnico di Torino - Dip. Automatica e Informatica     #
#   ###   \     #  Cso Duca degli Abruzzi 24 / I-10129 TORINO / ITALY        #
#    ##G  c\    #                                                            #
#    #     _\   #  Tel: +39-011564.7186  /  Fax: +39-011564.7099             #
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
#  Copyright (c) 2002-2006 Giovanni Squillero                                #
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
TARGET = ugp
TARGET_PURE = $(TARGET)-pure
VERSION=2.7.0-beta

TARGET_OBJECTS = Memory.o Heapsort.o Messages.o String.o Hash.o Stack.o InstructionLibrary.o Graph.o Fitness.o Genetic.o Operators.o Main.o getenv.o setenv.o getopt.c getopt1.c
#TARGET_OBJECTS = Memory.o Heapsort.o Messages.o String.o Hash.o Stack.o InstructionLibrary.o Graph.o Fitness.o Genetic.o Operators.o Main.o 
TARGET_HEADERS = ugp.h gmacros.h Check.h Memory.h Heapsort.h Messages.h String.h Hash.h Stack.h InstructionLibrary.h Graph.h Fitness.h Genetic.h Operators.h
TARGET_FILES = $(TARGET_OBJECTS:.o=.c) $(TARGET_HEADERS)
DISTRIBUITION_FILES = $(TARGET_FILES) ugp.c Makefile GPL.txt

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
STANDARD = -ansi -Wall $(INCLUDEPATH) -DNEED_SOME_PROTOS -DLINUX
#STANDARD = -ansi -Wall $(INCLUDEPATH) -DNEED_SOME_PROTOS
OPTIMIZE = -O99 -g
PROFILE = $(OPTIMIZE) -pg
DEBUG = -O -DGS_DEBUG -g

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -              NO MODIFICATIONS NEEDED AFTER THIS POINT                     -
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

#CFLAGS = $(STANDARD) $(DEBUG)
#CFLAGS = $(STANDARD) $(PROFILE)
CFLAGS = $(STANDARD) $(OPTIMIZE)

# -----------------------------------------------------------------------------

default: $(TARGET)-exp
all: TAGS $(TARGET) $(TARGET_PURE)
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

$(TARGET): $(TARGET_OBJECTS)
	$(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" ugp.c \
	$^ $(LIBS)

$(TARGET)-exp: $(TARGET_OBJECTS:.o=.c)
	$(CC) -o $@ \
	$(LIBPATH) $(STANDARD) $(DEBUG) -fprofile-arcs -ftest-coverage \
	-DUGP_FLAGS="\"$(STANDARD) $(DEBUG) -fprofile-arcs -ftest-coverage\"" \
	-DUGP_LINKED_BY="\"${USER}\"" \
	-DUGP_LINKED_MACHINE="\"${HOST}\"" \
	ugp.c $^ \
	$(LIBS)

$(TARGET)-pure: $(TARGET_OBJECTS)
	purify $(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" ugp.c \
	$^ $(LIBS)

$(TARGET)-insure: $(TARGET_OBJECTS:.o=-ins++.o)
	insure $(STANDARD) $(LIBPATH) -g -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" ugp.c \
	$^ $(LIBS)

test-dmalloc: test-dmalloc.o
	$(CC) $(LIBPATH) $(CFLAGS) -o $@ \
	-DUGP_FLAGS="\"${CFLAGS}\"" -DUGP_LINKED_BY="\"${USER}\"" -DUGP_LINKED_MACHINE="\"${HOST}\"" ugp.c \
	$^ $(LIBS)	

TAGS: $(TARGET_FILES)
	etags $^

install: ugp
	$(INSTALL) -m 775 ugp /cadtools/program/bin

dist:
	-rm -Rf ugp-$(VERSION)
	mkdir ugp-$(VERSION)
	ln $(DISTRIBUITION_FILES) ugp-$(VERSION)
	chown -R squiller:cadcad ugp-$(VERSION)
	chmod 644 ugp-$(VERSION)/*
	chmod 444 ugp-$(VERSION)/GPL.txt
	tar cvfz ugp-$(VERSION).tar.gz ugp-$(VERSION)
	chown -R squiller:cadcad ugp-$(VERSION).tar.gz
	-rm -Rf ugp-$(VERSION)

# -----------------------------------------------------------------------------
# DEPENDENCIES
# -----------------------------------------------------------------------------
Fitness.o: $(TARGET_HEADERS) ugp.c
Graph.o: $(TARGET_HEADERS) ugp.c
Genetic.o: $(TARGET_HEADERS) ugp.c
Operators.o: $(TARGET_HEADERS) ugp.c
Hash.o: $(TARGET_HEADERS) ugp.c
Stack.o: $(TARGET_HEADERS) ugp.c
InstructionLibrary.o: $(TARGET_HEADERS) ugp.c
Main.o: $(TARGET_HEADERS) ugp.c
Memory.o: $(TARGET_HEADERS) ugp.c
Messages.o: $(TARGET_HEADERS) ugp.c
