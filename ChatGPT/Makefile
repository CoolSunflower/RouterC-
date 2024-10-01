# Makefile for Router Simulation

CXX = g++
CXXFLAGS = -std=c++11

OBJS = main.o port.o scheduler.o

router_sim: $(OBJS)
	$(CXX) $(CXXFLAGS) -o router_sim $(OBJS)

main.o: main.cpp port.h packet.h scheduler.h
	$(CXX) $(CXXFLAGS) -c main.cpp

port.o: port.cpp port.h packet.h
	$(CXX) $(CXXFLAGS) -c port.cpp

scheduler.o: scheduler.cpp scheduler.h port.h packet.h
	$(CXX) $(CXXFLAGS) -c scheduler.cpp

clean:
	rm -f *.o router_sim
