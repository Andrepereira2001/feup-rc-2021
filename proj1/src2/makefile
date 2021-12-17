emissor = emissor/
recetor = recetor/

make: emissor recetor

emissor: ${emissor}emissor.o ${emissor}dataLinkEmissor.o VAR.h
	gcc ${emissor}emissor.o ${emissor}dataLinkEmissor.o -o emissor.o

${emissor}emissor.o: 
	gcc -c ${emissor}emissor.c -o ${emissor}emissor.o

${emissor}dataLinkEmissor.o: ${emissor}dataLinkEmissor.c ${emissor}dataLinkEmissor.h VAR.h
	gcc -c ${emissor}dataLinkEmissor.c -o ${emissor}dataLinkEmissor.o



recetor: ${recetor}recetor.o ${recetor}dataLinkRecetor.o VAR.h
	gcc ${recetor}recetor.o ${recetor}dataLinkRecetor.o -o recetor.o

${recetor}recetor.o: 
	gcc -c ${recetor}recetor.c -o ${recetor}recetor.o

${recetor}dataLinkRecetor.o: ${recetor}dataLinkRecetor.c ${recetor}dataLinkRecetor.h VAR.h
	gcc -c ${recetor}dataLinkRecetor.c -o ${recetor}dataLinkRecetor.o


clean:
	rm emissor.o ${emissor}dataLinkEmissor.o ${emissor}emissor.o
	rm recetor.o ${recetor}dataLinkRecetor.o ${recetor}recetor.o
