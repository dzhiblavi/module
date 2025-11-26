add_library(module STATIC)

target_sources(module PUBLIC ./Context.cpp ./Storage.cpp)
target_link_libraries(module PUBLIC reflectcpp)
