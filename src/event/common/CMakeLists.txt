include_directories("../..")

aux_source_directory("." src)

add_library(cppgo_event_common ${src})

