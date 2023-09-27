# Minimum required version of CMake
cmake_minimum_required(VERSION 3.0)

#set(KAFKA ON)
set(REDIS ON)
#set(NATS ON)
#set(MOSQUITTO ON)

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
    src/influxdb_sink.c
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

set(LINK_LIBRARIES
    cjson paho-mqtt3c curl config pthread mosquitto)

if(DEFINED KAFKA)
    message("KAFKA flag is defined. Including kafka_sink.c and linking with librdkafka.")
    list(APPEND SOURCES src/kafka_sink.c)
    list(APPEND LINK_LIBRARIES rdkafka)
else()
    message("KAFKA flag is not defined. Not including kafka_sink.c.")
endif()

if(DEFINED REDIS)
    message("REDIS flag is defined.")
    list(APPEND SOURCES src/redis_sink.c)
    list(APPEND LINK_LIBRARIES hiredis)
else()
    message("REDIS flag is not defined!")
endif()

if(DEFINED NATS)
    message("NATS flag is defined.")
    list(APPEND SOURCES src/nats_sink.c)
    list(APPEND LINK_LIBRARIES nats ssl crypto)
else()
    message("NATS flag is not defined!")
endif()

if(DEFINED MOSQUITTO)
    message("MOSQUITTO flag is defined.")
    list(APPEND SOURCES src/mosq_sink.c)
    list(APPEND SOURCES src/mosq_src.c)
    list(APPEND LINK_LIBRARIES mosquitto)
else()
    message("MOSQUITTO flag is not defined!")
endif()


# Create an executable from the sources
add_executable(xsolarlogger ${SOURCES} ${HEADERS})
target_link_libraries(xsolarlogger PRIVATE ${LINK_LIBRARIES})
