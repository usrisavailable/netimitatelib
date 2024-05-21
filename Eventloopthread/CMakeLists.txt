cmake_minimum_required(VERSION 3.8)

set(This Eventloopthread)

set(Headers 
    include/eventloopthread/eventloopthread.hpp)

set(Sources 
    src/eventloopthread.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC 
    Eventloop
    Thread)

#add_subdirectory(test)