cmake_minimum_required(VERSION 4.2.2)

project(translator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Whisper.cpp
# add_subdirectory(third_party/whisper.cpp)

add_executable(translator
    src/main.cpp
    third_party/whisper.cpp/examples/common.cpp
    third_party/whisper.cpp/examples/common-whisper.cpp
)

# whisper.cpp
add_subdirectory(
    third_party/whisper.cpp
    ${CMAKE_BINARY_DIR}/whisper.cpp
)

target_include_directories(translator PRIVATE
    third_party/whisper.cpp/examples
)

# Link whisper.cpp
target_link_libraries(translator PRIVATE whisper)