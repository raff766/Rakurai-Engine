CFLAGS = -std=c++20 -g
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: shaders *.cpp *.h libs/*.cpp libs/*.h
	g++ $(CFLAGS) -o VulkanTest *.cpp $(LDFLAGS)

shaders: shaders/*.frag shaders/*.vert
	$(foreach file, $(wildcard shaders/*.frag) $(wildcard shaders/*.vert), glslc $(file) -o $(file).spv;)

.PHONY: run clean

run: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest