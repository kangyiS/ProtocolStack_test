aux_source_directory(. LIB_SRC)

add_compile_options(-std=c++11)

include_directories(../../include)

add_library(DATALINK_LAYER ${LIB_SRC})
