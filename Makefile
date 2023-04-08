CC = gcc

test.out : imageFormationUtils.o main.o
	gcc -o test.out imageFormationUtils.o main.o

imageFormationUtils.o : imageFormationUtils.c
	gcc -c -o imageFormationUtils.o imageFormationUtils.c

main.o : main.c
	gcc -c -o main.o main.c

clean:
	rm *.o test.out