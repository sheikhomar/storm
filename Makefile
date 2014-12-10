build: main.c pixel.o ppm.o bchart.o
	gcc -ansi -Wall -pedantic main.c pixel.o ppm.o bchart.o -lm

bchart.o: bchart.h bchart.c ppm.o pixel.o
	gcc -ansi -Wall -pedantic -c bchart.c bchart.h -lm

ppm.o: ppm.h ppm.c pixel.o
	gcc -ansi -Wall -pedantic -c ppm.c ppm.h -lm

pixel.o: pixel.h pixel.c
	gcc -ansi -Wall -pedantic -c pixel.c pixel.h -lm

doc:
	doxygen Doxyfile

clean:
	rm -f *.o *.gch *.exe *.out
