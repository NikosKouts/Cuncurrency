executable: main.o lib.o
	gcc -g -Wall main.o lib.o

main.o: main.c
	gcc -g -Wall -c main.c

lib.o: lib.c
	gcc -g -Wall -c lib.c

clean: 
	rm *.o