output: bridge.o semlib.o
	gcc -g -Wall bridge.o semlib.o -lpthread

bridge.o: bridge.c
	gcc -g -Wall -c bridge.c -o bridge.o -lpthread

semlib.o: ../lib/semlib.c
	gcc -g -Wall -c ../lib/semlib.c -o semlib.o -lpthread

clean:
	rm *.o
