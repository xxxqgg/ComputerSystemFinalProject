cmake_minimum_required(VERSION 3.14)
project(ComputerFinalProject C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")


# TODO: Group files later
add_executable(fs main.c utils.c jsh_config.c builtins.c jsh_api.c SimpleFileSystem/FileSystemAPI.c
        SimpleFileSystem/FileSystemConfig.c)