make: main 

main: main.c utils.o
	gcc main.c utils.o -o main.o

utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o

clean:
	rm main.o utils.o 

