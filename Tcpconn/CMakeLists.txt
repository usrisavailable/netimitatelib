cmake_minimum_required(VERSION 3.8)

set(This Tcpconn)

set(Headers 
    include/tcpconn/tcpconn.hpp)

set(Sources 
    src/tcpconn.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    Eventloop
    Channel
    Socket
    Buffer
)

#add_subdirectory(test)