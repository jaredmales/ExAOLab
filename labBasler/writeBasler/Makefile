# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := write1Basler

# Installation directories for pylon
PYLON_ROOT ?= /opt/pylon5
CFLAGS     += -D_XOPEN_SOURCE=700 
CXXFLAGS   += -O3 -D_XOPEN_SOURCE=700
CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs) -L/opt/cfitsio -lxpa

# Build tools and flags
LD         := $(CXX)

# Rules for building
all: $(NAME)

$(NAME): $(NAME).o write_basler_fits.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lcfitsio -lm

# $(NAME).o: $(NAME).cpp
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) *.o $(NAME)
