CFLAGS = -std=c++20 -g
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: shaders src/*.cpp src/*.h
	g++ $(CFLAGS) -o VulkanTest src/*.cpp $(LDFLAGS)

shaders: shaders/*.frag shaders/*.vert
	$(foreach file, $(wildcard shaders/*.frag) $(wildcard shaders/*.vert), glslc $(file) -o $(file).spv;)

.PHONY: run clean

run: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest