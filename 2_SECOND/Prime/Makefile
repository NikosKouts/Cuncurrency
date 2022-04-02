
output: IsPrime.o semlib.o
	gcc -g -Wall IsPrime.o semlib.o -lpthread

IsPrime.o: IsPrime.c
	gcc -g -Wall -c IsPrime.c -o IsPrime.o -lpthread

semlib.o: semlib.c
	gcc -g -Wall -c semlib.c -o semlib.o -lpthread

clean:
	rm *.o