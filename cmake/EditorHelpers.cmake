# ctags
# inspired by https://stackoverflow.com/a/26003085
find_program(CTAGS_PATH ctags)
if(CTAGS_PATH)
    message(STATUS "Found ctags: ${CTAGS_PATH}")
	set_source_files_properties(tags PROPERTIES GENERATED true)
	add_custom_target(tags
		COMMAND ctags --recurse --c++-kinds=+p --fields=+iaS --extra=+q --exclude="*.js" --languages=C --languages=+C++ --totals ${CMAKE_SOURCE_DIR} ${SOURCE_ROOT} ${INCLUDE_ROOT}
		COMMAND ln -srf tags ${CMAKE_HOME_DIRECTORY}/tags
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
#add_dependencies(${PROJECT_NAME} tags)
endif(CTAGS_PATH)

# YouCompleteMe
# you should use syntastic or neomake instead
# otherwise, install the YCM-Generator plugin
#	and add your .vim/bundle/YCM-Generator/ path
#	to your PATH variable, then this works
find_program(YCM_CONFIG_GEN_PATH config_gen.py)
if(YCM_CONFIG_GEN_PATH)
	message(STATUS "Found config_gen.py: ${YCM_CONFIG_GEN_PATH}")
	set_source_files_properties(.ycm_extra_conf.py PROPERTIES GENERATED true)
	add_custom_target(ycmconfig
		COMMAND config_gen.py -f ${CMAKE_SOURCE_DIR}
		COMMAND ln -srf .ycm_extra_conf.py ${CMAKE_HOME_DIRECTORY}/.ycm_extra_conf.py
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
#add_dependencies(${PROJECT_NAME} ycmconfig)
endif(YCM_CONFIG_GEN_PATH)

# clang compile_commands.json file
set_source_files_properties(compile_commands.json PROPERTIES GENERATED true)
add_custom_target(compilecommands
	COMMAND ln -srf compile_commands.json ${CMAKE_HOME_DIRECTORY}/compile_commands.json
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# cscope database
find_program(CSCOPE_PATH cscope)
if(CSCOPE_PATH)
	message(STATUS "Found cscope: ${CSCOPE_PATH}")
	set_source_files_properties(
		cscope.files cscope.in.out cscope.out cscope.o.out
		PROPERTIES GENERATED true
	)
	add_custom_target(cscope
		COMMAND find ${SOURCE_ROOT}
			-name *.[chxsS] -o
			-name *.cc -o
			-name *.cpp -o
			-name *.cxx -o
			-name *.hpp > cscope.files
		COMMAND find ${INCLUDE_ROOT}
			-name *.h -o
			-name *.hpp >> cscope.files
		COMMAND cscope -b -q -k
		COMMAND ln -srf cscope.out ${CMAKE_HOME_DIRECTORY}/cscope.out
	)
#add_dependencies(${PROJECT_NAME} cscope)
endif(CSCOPE_PATH)
