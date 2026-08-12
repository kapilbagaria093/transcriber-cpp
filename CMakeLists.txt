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

add_library(wordnet STATIC
    third_party/wordnet/lib/binsrch.c
    third_party/wordnet/lib/morph.c
    third_party/wordnet/lib/search.c
    third_party/wordnet/lib/wnglobal.c
    third_party/wordnet/lib/wnhelp.c
    third_party/wordnet/lib/wnrtl.c
    third_party/wordnet/lib/wnutil.c
)

target_include_directories(wordnet
    PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/wordnet/include
)

target_compile_definitions(wordnet
    PRIVATE
    UNIX
)

target_compile_options(wordnet
    PRIVATE
    -fcommon
)

# Link whisper.cpp
target_link_libraries(translator PRIVATE whisper)
target_link_libraries(translator
    PRIVATE
    wordnet
)