cmake_minimum_required(VERSION 3.8)

set(This Thread)

set(Headers 
    include/thread/thread.hpp)

set(Sources 
    src/thread.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

#add_subdirectory(test)