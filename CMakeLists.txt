cmake_minimum_required(VERSION 2.8)

project(raspberry_car LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
raspberry_car
${CMAKE_SOURCE_DIR}/src/include/defs.h
${CMAKE_SOURCE_DIR}/src/include/sensors.h
${CMAKE_SOURCE_DIR}/src/include/camera.h
${CMAKE_SOURCE_DIR}/src/include/actuators.h
${CMAKE_SOURCE_DIR}/src/include/health.h
${CMAKE_SOURCE_DIR}/src/include/motors.h
${CMAKE_SOURCE_DIR}/src/include/kalmanfilter.h

${CMAKE_SOURCE_DIR}/src/main/sensors.cpp
${CMAKE_SOURCE_DIR}/src/main/camera.cpp
${CMAKE_SOURCE_DIR}/src/main/actuators.cpp
${CMAKE_SOURCE_DIR}/src/main/health.cpp
${CMAKE_SOURCE_DIR}/src/main/motors.cpp
${CMAKE_SOURCE_DIR}/src/main/kalmanfilter.cpp

${CMAKE_SOURCE_DIR}/src/main/main.cpp
)

find_package(OpenCV REQUIRED)
#find_library(WIRINGPI_LIBRARIES NAMES wiringPi)

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(raspberry_car ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(raspberry_car ${OpenCV_LIBS})
target_link_libraries(raspberry_car ${WIRINGPI_LIBRARIES})
target_include_directories(raspberry_car PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

