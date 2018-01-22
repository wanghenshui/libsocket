cmake_minimum_required(VERSION 3.1)
project(ifaddrs VERSION 1.0.0 LANGUAGES C)

add_library(ifaddrs ifaddrs.h ifaddrs.c)

if(ANDROID)  # avoid conflicts with system's ifaddrs on Linux
    target_include_directories(ifaddrs PUBLIC .)
endif()