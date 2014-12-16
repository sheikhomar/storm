build: main.c pixel.o ppm.o bchart.o config.o schedule.o
	gcc -ansi -Wall -pedantic main.c pixel.o ppm.o bchart.o config.o schedule.o -lm

schedule.o: schedule.c schedule.h
	gcc -ansi -Wall -pedantic -c schedule.c schedule.h

config.o: config.c config.h
	gcc -ansi -Wall -pedantic -c config.c config.h

bchart.o: bchart.h bchart.c ppm.o pixel.o
	gcc -ansi -Wall -pedantic -c bchart.c bchart.h -lm

ppm.o: ppm.h ppm.c pixel.o
	gcc -ansi -Wall -pedantic -c ppm.c ppm.h -lm

pixel.o: pixel.h pixel.c
	gcc -ansi -Wall -pedantic -c pixel.c pixel.h -lm