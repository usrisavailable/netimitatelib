cmake_minimum_required(VERSION 3.8)

set(This Channel)

set(Headers 
    include/channel/channel.hpp)

set(Sources 
    src/channel.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    Eventloop
    Loginfo)

#add_subdirectory(test)