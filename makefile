CC = g++
FLAGS = -Wall -Werror -std=c++2a

srcdir = src
build = build

objects = main.o magicbb.o movegen.o position.o search.o utils.o


build/main: $(addprefix build/, $(objects))
	$(CC) $(FLAGS) -o build/main $(addprefix build/, $(objects))

build/%.o : src/%.cpp
	$(CC) -c $(FLAGS) $< -o $@

.PHONY : clean
clean :
	rm *.o $(addprefix build/, $(objects)) \
	rm build/main