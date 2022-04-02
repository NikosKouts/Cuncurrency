output: main.o library.o
	gcc -g -Wall main.o library.o -lpthread

main.o: main.c
	gcc -g -Wall -c main.c

library.o: library.c
	gcc -g -Wall -c library.c

clean:
	rm *.o