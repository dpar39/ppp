#========================================
# Root project
#========================================
cmake_minimum_required(VERSION 3.12.0)
enable_testing()
project(PhotoIDCreator CXX)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

add_subdirectory(libppp)
