cmake_minimum_required(VERSION 3.14)
project(ComputerFinalProject C)

set(CMAKE_C_STANDARD 99)

add_executable(ComputerFinalProject main.c utils.c jsh_config.c builtins.c jsh_api.c)