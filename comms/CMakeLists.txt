add_executable(commd commd.c)
add_executable(commSend commSend.c)

target_include_directories(commd PUBLIC /usr/local/include/polysat)
target_include_directories(commSend PUBLIC /usr/local/include/polysat)

target_link_libraries(commd proc pthread)
target_link_libraries(commSend proc pthread)
