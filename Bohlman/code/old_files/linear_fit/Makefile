# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := linear_fit

# Build tools and flags
LD         := $(CXX)

# Rules for building
all: $(NAME)

$(NAME): $(NAME).o
	$(LD) -o $@ $^ -lgsl -lgslcblas -lm

$(NAME).o: $(NAME).cpp
	$(CXX) -c -o $@ $< 

clean:
	$(RM) $(NAME).o $(NAME)
