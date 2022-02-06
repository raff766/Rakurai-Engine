cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/out)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})
set(SHADERS_OUTPUT_DIR ${OUTPUT_DIR}/shaders)

project(VulkanTest VERSION 0.1)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw vulkan ${CMAKE_DL_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC libs src)

# Compile Shaders
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS shaders/*.frag shaders/*.vert)

make_directory(${SHADERS_OUTPUT_DIR})
foreach(file ${SHADER_FILES})
    cmake_path(GET file FILENAME fileName)
    set(outputFile ${SHADERS_OUTPUT_DIR}/${fileName}.spv)

    add_custom_command(OUTPUT ${outputFile} COMMAND glslangValidator --target-env vulkan1.2 -o ${outputFile} ${file} DEPENDS ${file})
    add_custom_target(${fileName}.spv ALL DEPENDS ${outputFile})
endforeach()

# Create symbolic link for models
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/models ${OUTPUT_DIR}/models)
