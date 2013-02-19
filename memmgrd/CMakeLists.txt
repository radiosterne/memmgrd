project(memmgrd)
cmake_minimum_required(VERSION 2.8)


set(SRC_LIST logger.cpp
             main.cpp
             mem_list_mgr.cpp
             mem_list_node.cpp
             socket_conn_handler.cpp
             server.cpp
             config_reader.cpp
             keccak.cpp

             logger.h
             mem_list_mgr.h
             mem_list_node.h
             common_definitions.h
             queue.h
             server.h
             socket_conn_handler.h
             config_reader.h
             keccak.h
    )


find_package (Threads)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
