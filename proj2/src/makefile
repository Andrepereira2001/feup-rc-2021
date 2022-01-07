make: download 

download: download.c utils.o
	gcc download.c utils.o -o download

utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o

clean:
	rm download utils.o 

