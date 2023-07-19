CC = g++
FLAGS = -Wall -Werror -std=c++2a -O3

srcdir = src
build = build

objects = main.o magicbb.o movegen.o position.o search.o utils.o evaluate.o


build/main: $(addprefix build/, $(objects))
	$(CC) $(FLAGS) -o build/main $(addprefix build/, $(objects))

build/%.o : src/%.cpp
	$(CC) -c $(FLAGS) $< -o $@

.PHONY : clean
clean :
	rm $(addprefix build/, $(objects)) \
	build/main \
	perf.data