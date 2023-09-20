# Minimum required version of CMake
cmake_minimum_required(VERSION 3.0)

# Project name
project(xsolarlogger)

# Add the source and include directories
#add_subdirectory(src)
include_directories(include)

# List of source files
set(SOURCES 
    src/main.c
    src/datalog.c
    src/logger.c
    src/squeue.c
    src/influxdb.c
    src/mqtt_sink.c
    src/mqtt_src.c
    src/configuration.c
)

# List of header files
set(HEADERS
    include/datalog.h
    include/logger.h
    include/squeue.h
)

# Create an executable from the sources
add_executable(xsolarlogger ${SOURCES} ${HEADERS})
target_link_libraries(xsolarlogger PRIVATE cjson paho-mqtt3c curl config)
