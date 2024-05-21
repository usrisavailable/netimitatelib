cmake_minimum_required(VERSION 3.8)

set(This Buffer)

set(Headers 
    include/buffer/buffer.hpp)

set(Sources 
    src/buffer.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_include_directories(${This} PUBLIC include)

#add_subdirectory(test)