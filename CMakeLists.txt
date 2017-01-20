cmake_minimum_required (VERSION 2.6)

project (common.xml)

### Enable modern compiler support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(src_xml
    xml.cpp
    xml.h
    xpath.cpp
)

add_executable(common.xml ${src_xml} example.cpp)
