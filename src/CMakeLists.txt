# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

set(COMPONENT_REQUIRES )

set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
"main.cpp"
)

set(COMPONENT_ADD_INCLUDEDIRS
"../lib/IR_STD"
"../lib/IR_STD/Example"
"../lib/CleanRTOS/src"
)

register_component()

#idf_component_register(SRCS ${app_sources})

