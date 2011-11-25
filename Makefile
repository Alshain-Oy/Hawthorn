
CC=g++

FLAGS = -g -I. -I./leveldb/include -pthread


all: levelDB libgraph benchmarks Hawthorn

levelDB:
	(cd leveldb; make; cd ..;)
	
libgraph: levelDB
	$(CC) $(FLAGS) libgraph.cpp -c -o libgraph.o

benchmarks:
	$(CC) $(FLAGS) benchmark01.cpp -o benchmark01 libgraph.o leveldb/libleveldb.a
	$(CC) $(FLAGS) benchmark02.cpp -o benchmark02 libgraph.o leveldb/libleveldb.a
	$(CC) $(FLAGS) benchmark03.cpp -o benchmark03 libgraph.o leveldb/libleveldb.a

Hawthorn: 
	$(CC) $(FLAGS) hawthorn-server.cpp -o hawthorn-server libgraph.o leveldb/libleveldb.a

clean-level:
	(cd leveldb; make clean; cd ..; )
	

clean: clean-level
	rm *.o
	rm benchmark01
	rm benchmark02
	rm benchmark03
	rm hawthorn-server
	rm *.pyc
	
