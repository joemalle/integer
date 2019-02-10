CXXFLAGS:=-Wall -Wextra -g -O0 -std=c++17 -fsanitize=address
CXXFLAGSRELEASE:=-Wall -Wextra -DNDEBUG -O3 -std=c++17
CXX:=clang++
CPPFILES:=$(wildcard *.cpp)
OBJECTS:=$(CPPFILES:.cpp=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

a.out: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $? -o $@

clean:
	rm -f *.o a.out
