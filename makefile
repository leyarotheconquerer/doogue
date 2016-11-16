CC=g++
LDLIBS=-lsfml-graphics -lsfml-window -lsfml-system

all: ./build/gameoff

./build/gameoff: ./build/objects/main.o
	$(CC) -o ./build/gameoff ./build/objects/main.o $(LDLIBS)

./build/objects/main.o: ./src/main.cpp
	$(CC) -o ./build/objects/main.o -c ./src/main.cpp

#%.o : %.cpp
#	$(CC) -o $@ -c $<

run:
	./build/gameoff

clean:
	rm -rf ./build/*
