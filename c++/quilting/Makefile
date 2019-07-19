CC = g++
FLAGS=`pkg-config --cflags --libs opencv` -Wall -O3

all: transfer synthesis metric
transfer: transfer.o quilting.o util.o
	$(CC) $(FLAGS) transfer.o quilting.o util.o -o transfer
synthesis: synthesis.o quilting.o util.o
	$(CC) $(FLAGS) synthesis.o quilting.o util.o -o synthesis
metric: metric.cpp
	$(CC) $(FLAGS) metric.cpp -o metric
transfer.o: transfer.cpp quilting.h
	$(CC) $(FLAGS) -c transfer.cpp
synthesis.o: synthesis.cpp quilting.h
	$(CC) $(FLAGS) -c synthesis.cpp
quilting.o: quilting.cpp quilting.h util.h
	$(CC) $(FLAGS) -c quilting.cpp
util.o: util.cpp util.h
	$(CC) $(FLAGS) -c util.cpp
clean:
	rm -rf *.o transfer synthesis
