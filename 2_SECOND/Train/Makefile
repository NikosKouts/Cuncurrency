output: train.o semlib.o
	gcc -g -Wall train.o semlib.o -lpthread

train.o: train.c
	gcc -g -Wall -c train.c -o train.o -lpthread

semlib.o: ../lib/semlib.c
	gcc -g -Wall -c ../lib/semlib.c -o semlib.o -lpthread

clean:
	rm *.o