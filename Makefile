CXXFLAGS := -std=c++23 -Wall -Wextra -O3 -I./ -MMD -MP
SOURCES  := $(wildcard src/*.cpp)
OBJECTS  := $(patsubst src/%.cpp,build/%.o,$(SOURCES))
DEPS     := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: main

main: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# registry.cpp is ~15k lines of static data; -O3 takes ~80s, -O0 takes ~2s
build/registry.o: CXXFLAGS := $(patsubst -O%,-O0,$(CXXFLAGS))

build:
	mkdir -p build

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPS)
	rm -f main

run: all
	./main

-include $(DEPS)
