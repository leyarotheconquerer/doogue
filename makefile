CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g
LDFLAGS=-g
LDLIBS=-lsfml-graphics -lsfml-window -lsfml-system

SRCS=src/main.cpp
OBJS=$(addprefix objs/,$(notdir $(SRCS:.cpp=.o)))

BUILD_TARGET=gameoff

all: build/$(BUILD_TARGET)

./build/$(BUILD_TARGET): $(OBJS)
	@mkdir -p build
	$(CXX) $(LDFLAGS) -o build/$(BUILD_TARGET) $(OBJS) $(LDLIBS)

./objs/%.o: src/%.cpp
	@mkdir -p objs
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

run:
	build/$(BUILD_TARGET)

clean:
	$(RM) $(OBJS)
	$(RM) build/$(BUILD_TARGET)
