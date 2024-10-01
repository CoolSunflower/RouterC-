CXX = g++
CXXFLAGS = -Wall -std=c++17

# Define the target executable name
TARGET = program

# Define the source files
SRC = router.cpp

# Default rule to compile and run the program
all: $(TARGET)
	./$(TARGET)

# Rule to compile the source files into the executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Rule to clean up the directory
clean:
	rm -f $(TARGET)
