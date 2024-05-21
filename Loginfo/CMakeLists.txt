cmake_minimum_required(VERSION 3.8)

set(This Loginfo)

set(Headers 
    include/loginfo/loginfo.hpp)

set(Sources 
    src/loginfo.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC 
    Timestamp)

#add_subdirectory(test)