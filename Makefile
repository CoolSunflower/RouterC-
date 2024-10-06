CXX = g++
CXXFLAGS = -Wall -std=c++17

TARGET = router
METRICS = metrics
SRC = router.cpp
METRICSSRC = metrics.cpp

all: $(TARGET)
	make run

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	$(CXX) $(CXXFLAGS) -o $(METRICS) $(METRICSSRC)

clean:
	rm -f $(TARGET)
	rm -f $(METRICS)

SIMULATIONP = ./$(TARGET)

run:
	@while ! $(SIMULATIONP); do \
		echo "Randomised Fault, Rerunning..."; \
	done
	@echo "Succesful Simulation.";\
	./$(METRICS)