cmake_minimum_required(VERSION 3.1.0)
project(raspberry_car)

add_executable(
raspberry_car
${CMAKE_SOURCE_DIR}/src/include/defs.h
${CMAKE_SOURCE_DIR}/src/main/main.c
)


