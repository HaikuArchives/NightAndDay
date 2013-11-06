################################################
#
# Night And Day, Copyright (C) 1998-1999 Jean-Baptiste M. Queru
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either version 2 
# of the License, or (at your option) any later version. 
#
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details. 
#
# You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# Contact the author at:
# Jean-Baptiste M. Queru, 1706 Marina Ct #B, San Mateo, CA 94403, USA
# or by e-mail at : djaybee@cyberdude.com
#
################################################

# the file name, ...

BINARY := Night\ And\ Day
VERSION := 0.1.2
LIBS := -lbe

################################################

# output directories

OBJDIR := obj
DEPDIR := dep
SRCDIR := src
DATADIR := data

# compiler, linker, ...

CC := gcc -c
CFLAGS :=
CFLAGS += -O3
CFLAGS += -fno-pic
CFLAGS += -fomit-frame-pointer
CFLAGS += -ffast-math
CFLAGS += -Wall -W -Wno-multichar -Wp,-Wall
CFLAGS += -Werror
CFLAGS += -DVERSION_STRING=\"$(VERSION)\" -DBINARY_NAME=\"$(BINARY)\"

LD := gcc
LDFLAGS := $(LIBS)

DEP := gcc -MM

ZIP := zip -r -9 -y

################################################

# the engine

MAKEFILE := GNUmakefile

FULLNAME := $(subst \ ,_,$(BINARY))-$(VERSION)

DATA := $(shell find $(DATADIR) -type f)

BASESOURCES := $(shell cd $(SRCDIR) && ls -1 *.cpp)
SOURCES := $(addprefix $(SRCDIR)/,$(BASESOURCES))
OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(BASESOURCES))))
DEPENDS := $(addprefix $(DEPDIR)/,$(addsuffix .d,$(basename $(BASESOURCES))))

BASEHEADERS := $(shell cd $(SRCDIR) && ls -1 *.h)
HEADERS := $(addprefix $(SRCDIR)/,$(BASEHEADERS))

.PHONY : default clean binarchive sourcearchive all

.DELETE_ON_ERROR : $(BINARY)

default : $(BINARY)

clean :
	@echo cleaning
	@rm -rf $(BINARY) $(OBJDIR) $(DEPDIR) *.zip *.zip~

all : sourcearchive binarchive

sourcearchive : $(FULLNAME)-src.zip

binarchive : $(FULLNAME)-$(BE_HOST_CPU).zip

$(BINARY) : $(OBJECTS)
	@echo linking $@
	@$(LD) $(LDFLAGS) $^ -o "$@"

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo compiling $@
	@mkdir -p $(OBJDIR)
	@$(CC) $< $(CFLAGS) -o $@

$(DEPDIR)/%.d : $(SRCDIR)/%.cpp
	@echo generating dependencies for $<
	@mkdir -p $(DEPDIR)
	@$(DEP) $< > $@
	@echo -e yf.i$(OBJDIR)/\\033t:a $(DEPDIR)/\\033pad \\033f:a $(MAKEFILE)\\033ZZ | vi $@ >/dev/null 2>/dev/null

$(FULLNAME)-$(BE_HOST_CPU).zip : $(BINARY) $(DATA) $(MAKEFILE)
	@rm -rf $@~
	@mkdir -p $@~/$(FULLNAME)-$(BE_HOST_CPU)/
	@cp -a $(BINARY) $(DATADIR)/* $@~/$(FULLNAME)-$(BE_HOST_CPU)/
	@cd $@~ && $(ZIP) $@ $(FULLNAME)-$(BE_HOST_CPU)
	@mv -f $@~/$@ .
	@rm -rf $@~

$(FULLNAME)-src.zip : $(SOURCES) $(HEADERS) $(DATA) $(MAKEFILE)
	@rm -rf $@~
	@mkdir -p $@~/$(FULLNAME)-src/
	@cp -a $(SRCDIR) $(DATADIR) $(MAKEFILE) $@~/$(FULLNAME)-src/
	@cd $@~ && $(ZIP) $@ $(FULLNAME)-src
	@mv -f $@~/$@ .
	@rm -rf $@~

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),depend)
ifneq ($(MAKECMDGOALS),sourcearchive)
include $(DEPENDS)
endif
endif
endif
