add_library(module STATIC)
add_library(module::module ALIAS module)

target_sources(module PUBLIC ./Context.cpp ./Storage.cpp)
target_link_libraries(module PUBLIC reflectcpp)
target_include_directories(module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
