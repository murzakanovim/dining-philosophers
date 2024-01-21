# These will be the compiler and options to use
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# The final program name
PROGRAM1 = dijkstra
PROGRAM2 = waiter

# These are the sources for the corresponding programs
SOURCES1 = dijkstra.cpp my_rand.cpp
SOURCES2 = waiter.cpp my_rand.cpp

# This is the default target (to compile both programs)
all: $(PROGRAM1) $(PROGRAM2)

# Rules to link the programs
$(PROGRAM1): $(SOURCES1:.cpp=.o)
	$(CXX) $^ -o $@

$(PROGRAM2): $(SOURCES2:.cpp=.o)
	$(CXX) $^ -o $@

# Rule to compile each source file
%.o: %.cpp my_rand.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

# Rule to clean object files
clean:
	rm -f $(PROGRAM1) $(PROGRAM2) *.o