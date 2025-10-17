cmake_minimum_required(VERSION 3.16)
project(Fractal C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-O3)
endif()

include_directories(${CMAKE_SOURCE_DIR}/includes)


# ISPC setup
find_program(ISPC_EXECUTABLE ispc REQUIRED)

set(ISPC_SRC ${CMAKE_SOURCE_DIR}/src/calculations/fractal.ispc)
set(ISPC_HDR ${CMAKE_SOURCE_DIR}/includes/fractal_ispc.h)
set(ISPC_OBJ_DIR ${CMAKE_BINARY_DIR}/obj/calculations)
file(MAKE_DIRECTORY ${ISPC_OBJ_DIR})
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/includes)

# Platform-specific ISPC targets
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    add_custom_command(
            OUTPUT ${ISPC_OBJ_DIR}/fractal_ispc.o ${ISPC_HDR}
            COMMAND ${ISPC_EXECUTABLE} ${ISPC_SRC} --emit-obj -h ${ISPC_HDR} -o ${ISPC_OBJ_DIR}/fractal_ispc.o
            DEPENDS ${ISPC_SRC}
            COMMENT "ISPC dispatcher"
    )
    set(ISPC_OBJECTS
            ${ISPC_OBJ_DIR}/fractal_ispc.o
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    message(STATUS "Building ISPC for ARM64 (NEON)")
    set(ISPC_TARGETS "--target=neon")
    set(ISPC_OBJECTS ${ISPC_OBJ_DIR}/fractal_ispc.o)

    add_custom_command(
            OUTPUT ${ISPC_OBJECTS} ${ISPC_HDR}
            COMMAND ${ISPC_EXECUTABLE} ${ISPC_TARGETS} -h ${ISPC_HDR} -o ${ISPC_OBJ_DIR}/fractal_ispc.o ${ISPC_SRC}
            WORKING_DIRECTORY ${ISPC_OBJ_DIR}
            DEPENDS ${ISPC_SRC}
            COMMENT "Running ISPC with ${ISPC_TARGETS}"
            VERBATIM
    )

else()
    message(STATUS "Building ISPC for generic target")
    set(ISPC_TARGETS "--target=generic-1")
    set(ISPC_OBJECTS ${ISPC_OBJ_DIR}/fractal_ispc.o)
endif()

add_custom_target(ispc_build DEPENDS ${ISPC_OBJECTS} ${ISPC_HDR})


# Source files
file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)


# GLFW 3.4
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/glfw-3.4 ${CMAKE_BINARY_DIR}/_glfw)


# GLAD
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glad/include)
file(GLOB GLAD_SOURCES ${CMAKE_SOURCE_DIR}/dependencies/glad/src/*.c)


# Executable
add_executable(fractal
        ${CPP_SOURCES}
        ${GLAD_SOURCES}
        ${ISPC_OBJECTS}
)

add_dependencies(fractal ispc_build)

target_link_libraries(fractal PRIVATE glfw)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(fractal PRIVATE OpenGL::GL)

# Platform specifics
if (APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION=1)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(fractal PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
    )
elseif (WIN32)
    target_link_libraries(fractal PRIVATE opengl32)
else()
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(fractal PRIVATE ${X11_LIBRARIES} Threads::Threads dl)
endif()


# Output
set_target_properties(fractal PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET fractal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:fractal>"
        "${CMAKE_SOURCE_DIR}/fractal"
        COMMENT "Copying fractal to project root"
)