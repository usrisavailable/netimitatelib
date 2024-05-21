cmake_minimum_required(VERSION 3.8)

set(This Socket)

set(Headers 
    include/socket/socket.hpp)

set(Sources 
    src/socket.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

#add_subdirectory(test)