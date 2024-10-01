CXX = g++
CXXFLAGS = -Wall -std=c++17

TARGET = router
SRC = router.cpp

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
clean:
	rm -f $(TARGET)
