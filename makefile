CC = g++
FLAGS = -Wall -Werror

objects = src/main.o src/magicbb.o src/movegen.o src/position.o src/search.o src/utils.o


main: $(objects)
	$(CC) $(FLAGS) -o main $(objects)

%.o : %.cpp
	$(CC) -c $(FLAGS) $< -o $@

.PHONY : clean
clean :
	rm *.o $(objects) \
	rm main