executable: main.o api.o
	gcc -g -Wall main.o api.o 

main.o: main.c
	gcc -g -Wall -c main.c
api.o: api.c
	gcc -g -Wall -c api.c

clean:
	rm *.o