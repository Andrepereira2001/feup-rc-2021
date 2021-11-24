make: emissor.c recetor.c
	gcc emissor.c -o emissor.o
	gcc recetor.c -o recetor.o

clean:
	rm emissor.o
	rm recetor.o
