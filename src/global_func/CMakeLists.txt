aux_source_directory(. LIB_SRC)

add_compile_options(-std=c++11)

include_directories(../../include)

add_library(GLOBAL ${LIB_SRC})
