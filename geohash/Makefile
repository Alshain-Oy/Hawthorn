CC = g++

FLAGS = -O3

all: main libhash

main: libhash
	$(CC) $(FLAGS) main.cpp -o main libhash.o

libhash:
	$(CC) $(FLAGS) libhash.cpp -c -o libhash.o
