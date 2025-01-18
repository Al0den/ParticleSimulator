CXX=g++
CXFLAGS= -std=c++20 -O3 -g
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -g

TARGET=simulation
SRCS=$(wildcard src/*.cpp)
OBJS=$(SRCS:.cpp=.o)

build: $(TARGET)

run: $(TARGET)
	./$(TARGET)

profile:
	make -B -j8
	rm -rf simulation.trace
	xctrace record --output simulation.trace --template "Time Profiler" --launch -- ./simulation 
	open simulation.trace

$(TARGET): $(OBJS)
	$(CXX) -pg -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -pg $(CXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean

print_objs:
	$(OBJS)



