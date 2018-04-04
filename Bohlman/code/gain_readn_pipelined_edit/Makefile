# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := gain_readn_pipelined_edit

# Installation directories for pylon
PYLON_ROOT ?= /opt/pylon5

# Build tools and flags
LD         := $(CXX)
CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
CXXFLAGS   := #e.g., CXXFLAGS=-g -O0 for debugging
LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs)

# Rules for building
all: $(NAME)

$(NAME): $(NAME).o write_basler_fits.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS) -g -L/opt/cfitsio -lcfitsio -lgsl -lgslcblas -lm

$(NAME).o: $(NAME).cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS)  -g -c -o $@ $< 

write_basler_fits.o: write_basler_fits.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -g -c -o $@ $< 


clean:
	$(RM) $(NAME).o $(NAME)
