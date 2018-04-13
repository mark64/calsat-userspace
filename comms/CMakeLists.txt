add_executable(commd commd.c)
add_executable(cmdSend cmdSend.c)

target_include_directories(commd PUBLIC /usr/local/include/polysat)
target_include_directories(cmdSend PUBLIC /usr/local/include/polysat)

target_link_libraries(commd proc pthread)
target_link_libraries(cmdSend proc pthread)
