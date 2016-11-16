#VPATH=src
CC=g++

#.SUFFIXES: .c .cpp .o

all: ./build/gameoff

yellowmellow:
	@echo HEY MAN

./build/gameoff: ./build/objects/*
	$(CC) -o ./build/gameoff ./build/objects/*

%.o : %.cpp
	$(CC) -c -o $@ $<

run:
	./build/gameoff

clean:
	rm -rf ./build/*
