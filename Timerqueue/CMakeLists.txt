cmake_minimum_required(VERSION 3.8)

set(This Timerqueue)

set(Headers 
    include/timerqueue/timerqueue.hpp)

set(Sources 
    src/timerqueue.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    Eventloop
    Channel
)

#add_subdirectory(test)