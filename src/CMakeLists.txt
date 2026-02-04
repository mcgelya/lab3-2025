add_library(lab3_core
    graph.cpp
    directed_graph.cpp
    shortest_paths.cpp
)

target_include_directories(lab3_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(graph_cli graph_cli.cpp)
target_link_libraries(graph_cli PRIVATE lab3_core)
target_include_directories(graph_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
