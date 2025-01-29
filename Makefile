CXX = g++
CXXFLAGS = -std=c++20 -O3 -g 
LDFLAGS = -g -lSDL2 -lSDL2_ttf\
        -framework Metal \
        -framework Foundation \
      
TARGET=simulation
SRCS=$(wildcard src/*.cpp) $(wildcard utils/vkbootstrap/*.cpp)
OBJS=$(SRCS:.cpp=.o)

SHADERS=$(wildcard shaders/*.metal)
SHADERS_OBJS=$(SHADERS:.metal=.metallib)

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

$(TARGET): $(OBJS) $(SHADERS_OBJS)
	$(CXX) -pg -o $@ $(OBJS) $(LDFLAGS)


%.o: %.cpp
	$(CXX) -pg $(CXXFLAGS) -c $< -o $@

%.metallib: %.metal
	xcrun -sdk macosx metal -frecord-sources=flat $< -o $@
clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean

print_objs:
	$(OBJS)



