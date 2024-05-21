cmake_minimum_required(VERSION 3.8)

set(This Tcpserver)

set(Headers 
    include/tcpserver/tcpserver.hpp)

set(Sources 
    src/tcpserver.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    Eventloop
    Tcpconn
    Acceptor
    Threadpoll
)

#add_subdirectory(test)