cmake_minimum_required(VERSION 4.3)
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

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.9b
        GIT_SHALLOW TRUE
)

FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(spdlog glfw glad glm imgui json)
execute_process(COMMAND ${ACTIVE_VENV} -m pip install -r ${glad_SOURCE_DIR}/requirements.txt)
glad_add_library(glad_gl_41 STATIC API gl:core=4.1)

file(GLOB IMGUI_SOURCE CONFIGURE_DEPENDS ${imgui_SOURCE_DIR}/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCE}
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS src/*.cpp)
add_executable(AI_Escape ${SOURCE})
target_link_libraries(AI_Escape PRIVATE spdlog glad_gl_41 glm imgui glfw nlohmann_json::nlohmann_json)
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