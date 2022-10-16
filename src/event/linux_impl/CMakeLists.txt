include_directories("../..")

aux_source_directory("." src)

add_library(cppgo_event ${src})

target_link_libraries(cppgo_event cppgo_event_common)