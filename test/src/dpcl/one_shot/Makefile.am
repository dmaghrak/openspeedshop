################################################################################
# Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

noinst_PROGRAMS = mutator hello  mutatee

CFLAGS	= \
	 -g -gdwarf-2

CXXFLAGS	= \
	 -g -gdwarf-2

CPPFLAGS	= \
	 -g -gdwarf-2

mutator_CXXFLAGS = \
	 $(DPCL_CPPFLAGS)

mutator_LDFLAGS = \
	-L$(ROOT)/lib \
	$(DPCL_LDFLAGS) \
	$(DPCL_LIBS) -lelf

mutator_LIBS = \
	$(DPCL_LDFLAGS) \
	$(DPCL_LIBS) -lelf

mutator_SOURCES = \
	mutator.cxx

mutatee_CFLAGS = \
	 $(DPCL_CPPFLAGS) \
	 $(CPPFLAGS)

mutatee_LDADD = \
	-ldl

mutatee_SOURCES = \
	mutatee.c

hello_CFLAGS = \
	$(DPCL_CPPFLAGS) \
	$(CXXFLAGS) \
	-g -gdwarf-2 -fpic 

hello_LDADD = \
	-L$(ROOT)/lib 

hello_LDFLAGS = \
	-shared -o hello.so


hello_SOURCES = \
	hello.c


EXTRA_DIST = \
	README \
        one_shot.tst \
        one_shot.tst.expected \
        runone-dpcl \
        runall-dpcl 

