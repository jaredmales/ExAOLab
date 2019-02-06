# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := view1Basler

UNAME ?= $(shell uname)
ifeq ($(UNAME),Darwin)
	CFLAGS += -D_BSD_SOURCE
<<<<<<< HEAD
	CXXFLAGS += -O3 -D_BSD_SOURCE -I /Library/Frameworks/pylon.framework/Headers/GenICam
=======
	CXXFLAGS += -D_BSD_SOURCE -I /Library/Frameworks/pylon.framework/Headers/GenICam
>>>>>>> cd0d986f3ec6ea1407f25b7009e7276e7bea894a
	LDLIBS += -framework pylon -rpath /Library/Frameworks -rpath /System/Library/Frameworks
else
	# Installation directories for pylon
	PYLON_ROOT ?= /opt/pylon5
	CFLAGS     += -D_XOPEN_SOURCE=700
<<<<<<< HEAD
	CXXFLAGS   += -O3 -D_XOPEN_SOURCE=700
	CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
=======
	CXXFLAGS   += -D_XOPEN_SOURCE=700
	CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
	CXXFLAGS   := #e.g., CXXFLAGS=-g -O0 for debugging
>>>>>>> cd0d986f3ec6ea1407f25b7009e7276e7bea894a
	LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
	LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs) -L/opt/cfitsio -lxpa
endif

# Build tools and flags
LD         := $(CXX)

# Rules for building
all: $(NAME)

$(NAME): $(NAME).o 
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lcfitsio -lm

$(NAME).o: $(NAME).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) *.o $(NAME) *.fits
