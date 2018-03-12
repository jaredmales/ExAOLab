# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := graph_bins
LD         := $(CXX)

$(NAME): $(NAME).o
	$(LD) -g -o $@ $^

$(NAME).o: $(NAME).cpp
	$(CXX) -g -c $< 

clean:
	$(RM) $(NAME).o $(NAME)
