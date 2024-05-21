cmake_minimum_required(VERSION 3.8)

set(This Threadpoll)

set(Headers 
    include/threadpoll/threadpoll.hpp)

set(Sources 
    src/threadpoll.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC 
    Eventloopthread
    Eventloop)

#add_subdirectory(test)