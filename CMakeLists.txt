cmake_minimum_required(VERSION 3.14)
project(AI_Escape)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Create virtual environment for building dependencies
set(VENV_DIR ${CMAKE_BINARY_DIR}/.venv)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT EXISTS ${VENV_DIR})
    message("Creating virtual environment in ${VENV_DIR}")
    execute_process(COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR})
endif()
set(ENV{VIRTUAL_ENV} ${VENV_DIR})
set(Python3_FIND_VIRTUALENV ONLY)
set(ACTIVE_VENV ${VENV_DIR}/bin/python3)


include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
)
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR cmake
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spdlog glfw glad glm)
execute_process(COMMAND ${ACTIVE_VENV} -m pip install -r ${glad_SOURCE_DIR}/requirements.txt)
glad_add_library(glad_gl_41 STATIC API gl:core=4.1)
file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS src/*.cpp)

add_executable(AI_Escape ${SOURCE})
target_link_libraries(AI_Escape PRIVATE spdlog glfw glad_gl_41 glm)
target_include_directories(AI_Escape PRIVATE include)
target_compile_definitions(AI_Escape PRIVATE GLFW_INCLUDE_NONE)

file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Assets/*")
add_custom_command(
        OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/copied.stamp"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets"
        COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/copied.stamp"
        DEPENDS ${ASSETS}
        COMMENT "Copying assets..."
)

add_custom_target(copy_assets ALL DEPENDS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/copied.stamp")
add_dependencies(AI_Escape copy_assets)