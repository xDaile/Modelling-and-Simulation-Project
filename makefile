CC= g++
FLAGS= -lsimlib -lm

all: ims.cpp
	$(CC) -o $@ ims.cpp $(FLAGS)

run:
	./ims
