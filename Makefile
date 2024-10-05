CXX = g++
CXXFLAGS = -Wall -std=c++17

TARGET = router
METRICS = metrics
SRC = router.cpp
METRICSSRC = metrics.cpp

all: $(TARGET)
	./$(TARGET)
	./$(METRICS)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	$(CXX) $(CXXFLAGS) -o $(METRICS) $(METRICSSRC)

clean:
	rm -f $(TARGET)
	rm -f $(METRICS)
