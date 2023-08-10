
cmake_minimum_required(VERSION 3.12.0)
project(ThirdPartyLibs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(ResolveRapidJSON)
ResolveRapidJSON(1.1.0)

include(ResolveOpenCV)
ResolveOpenCV(4.8.0)
