CC=g++

all: ./build/gameoff

./build/gameoff: ./build/objects/main.o
	$(CC) -o ./build/gameoff ./build/objects/main.o -lsfml-graphics -lsfml-window -lsfml-system

./build/objects/main.o: ./src/main.cpp
	$(CC) -o ./build/objects/main.o -c ./src/main.cpp

#%.o : %.cpp
#	$(CC) -c -o $@ $<

run:
	./build/gameoff

clean:
	rm -rf ./build/*
