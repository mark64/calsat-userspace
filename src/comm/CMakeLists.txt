add_executable(commd commd.c     argvUtil.c encodePacket.c sendPacket.c packPacket.c)
add_executable(cmdSend cmdSend.c argvUtil.c encodePacket.c sendPacket.c packPacket.c)

target_include_directories(commd PUBLIC /usr/local/include/polysat)
target_include_directories(cmdSend PUBLIC /usr/local/include/polysat)

target_link_libraries(commd proc pthread)
target_link_libraries(cmdSend proc pthread)
