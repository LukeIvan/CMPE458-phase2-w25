# This is the main CMake configuration file
# Make sure CMake is synced whenever you make the changes
cmake_minimum_required(VERSION 3.24)
project(phase2-w25 C)

set(CMAKE_C_STANDARD 11)

# Add include directory (this will be needed to add your tokens to your lexer)
include_directories(${PROJECT_SOURCE_DIR}/phase2-w25/include)

add_compile_options(-Werror -Wall -Wextra -Wpedantic)

# Add executables when needed: Make sure you specify the path to your .c or .h file
add_executable(phase2-w25
        phase2-w25/src/parser/parser.c
        phase2-w25/src/lexer/lexer.c)
        
