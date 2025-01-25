CXX=g++
CXFLAGS= -std=c++20 -O3 -g
LDFLAGS = -lSDL2 -g $(pkg-config --libs vulkan) -lvulkan.1.3.290 

TARGET=simulation
SRCS=$(wildcard src/*.cpp) $(wildcard utils/vkbootstrap/*.cpp)
OBJS=$(SRCS:.cpp=.o)

build: $(TARGET)

run: $(TARGET)
	./$(TARGET)

profile:
	make -B -j8
	rm -rf simulation.trace
	xctrace record --output simulation.trace --template "Time Profiler" --launch -- ./simulation 
	open simulation.trace

dump:
	find src/ -type f -name "*.cpp" -exec sh -c 'echo "File: {}" && echo && cat "{}" && echo "\n\n"' \; > cpp_files_with_content.txt

$(TARGET): $(OBJS)
	$(CXX) -pg -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -pg $(CXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean

print_objs:
	$(OBJS)



