cmake_minimum_required(VERSION 3.8)

set(This Eventloop)

set(Headers 
    include/eventloop/eventloop.hpp)

set(Sources 
    src/eventloop.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)
target_link_libraries(${This} PUBLIC
    Timestamp
    Loginfo
    Poll
    Channel
    Timerqueue)

#add_subdirectory(test)