cmake_minimum_required(VERSION 3.8)

set(This Acceptor)

set(Headers 
    include/acceptor/acceptor.hpp)

set(Sources 
    src/acceptor.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC 
    Eventloop
    Channel
    Socket)

#add_subdirectory(test)