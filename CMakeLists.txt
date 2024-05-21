# CMakeLists.txt for webserver
#
# created 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This webserver)
project(${This} C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)

enable_testing()

get_directory_property(ParentDirectory PARENT_DIRECTORY)
if(ParentDirectory STREQUAL "")

    # Organize applications, libraries, etc. in folders in the IDE.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Ensure proper loading of side-loaded shared libraries
    # on all targets.
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # Don't add the goofy "d" postfix for debug builds of libraries.
    set(CMAKE_DEBUG_POSTFIX "")

    # Google Test
    #set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject" FORCE)
    #set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
    #add_subdirectory(googletest)
    #set_target_properties(gtest gtest_main PROPERTIES
    #    FOLDER Libraries
    #)

    # All other libraries can be pulled in without further configuration.
    add_subdirectory(Loginfo)
    add_subdirectory(Timestamp)
    add_subdirectory(Channel)
    add_subdirectory(Timerqueue)
    add_subdirectory(Poll)
    add_subdirectory(Thread)
    add_subdirectory(Eventloop)
    add_subdirectory(Eventloopthread)
    add_subdirectory(Socket)
    add_subdirectory(Acceptor)
    add_subdirectory(Tcpserver)
    add_subdirectory(Tcpconn)
    add_subdirectory(Buffer)
    add_subdirectory(Threadpoll)
endif(ParentDirectory STREQUAL "")

# Add subdirectories directly in this repository.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set (CLEAR "rm -rf core*")
execute_process(COMMAND ${CLEAR})


# you can create your own test file here or just delete it
set(TARGET test1)
add_executable(${TARGET} test1.cc)
target_link_libraries(${TARGET} PUBLIC
    Timestamp
    Eventloop
    Thread
    Channel
    Loginfo
    Eventloopthread
    Acceptor
    Tcpserver
    Buffer
    Threadpoll)
