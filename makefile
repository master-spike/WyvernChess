CC = g++
FLAGS = -Wall -Werror -std=c++2a -O3 -g

srcdir = src


objdir = objects
objects = main.o magicbb.o movegen.o position.o search.o utils.o evaluate.o zobrist.o transposition.o zobrist_generate.o
builddir = build
utilitiesdir = utilities
utilities = zobrist_generate
main = main


all: $(builddir)/$(main) $(addprefix $(utilitiesdir)/, $(utilities))

mainobjs = $(main).o $(utilities).o

$(builddir)/main: $(objdir)/$(main).o $(addprefix $(objdir)/, $(filter-out $(mainobjs) ,$(objects))) 
	$(CC) $(FLAGS) -o $(builddir)/$(main) $^

$(utilitiesdir)/%: $(objdir)/%.o $(addprefix $(objdir)/, $(filter-out $(mainobjs) ,$(objects)))
	$(CC) $(FLAGS) -o $@ $^

$(objdir)/%.o: $(srcdir)/%.cpp
	$(CC) -c $(FLAGS) $< -o $@

.PHONY : clean
clean :
	rm -r build/* objects/* utilities/*
	rm perf.data
