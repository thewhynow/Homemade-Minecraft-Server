CXXFLAGS := -std=c++23 -Wall -Wextra -O3
SOURCES  := $(wildcard src/*.cpp)
OBJECTS  := $(patsubst src/%.cpp,build/%.o,$(SOURCES))

.PHONY: all clean run

all: main

main: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -f $(OBJECTS)
	rm -f main

run: all
	./main
