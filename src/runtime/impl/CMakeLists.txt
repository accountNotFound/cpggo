include_directories("../..")

aux_source_directory("." src)

add_library(cppgo_runtime ${src})