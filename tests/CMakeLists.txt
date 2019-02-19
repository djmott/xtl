
ExternalProject_Add(catch2_download
	PREFIX .catch2
	INSTALL_DIR catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_SHALLOW TRUE
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/catch2
		-DBUILD_TESTING=FALSE
		-DCATCH_BUILD_EXAMPLES=FALSE
		-DCATCH_BUILD_EXTRA_TESTS=FALSE
		-DCATCH_BUILD_TESTING=FALSE
		-DCATCH_ENABLE_COVERAGE=FALSE
		-DCATCH_ENABLE_WERROR=FALSE
		-DCATCH_INSTALL_DOCS=FALSE
		-DCATCH_INSTALL_HELPERS=FALSE
		-DCATCH_USE_VALGRIND=FALSE
	UPDATE_COMMAND ""
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/catch2/include)

add_library(catch2 INTERFACE IMPORTED GLOBAL)
target_include_directories(catch2 INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/catch2/include)
add_dependencies(catch2 catch2_download)


function(add_catch_test _name)
	add_executable(${_name} ${_name}.cpp)
	target_link_libraries(${_name} PRIVATE xtl catch2)
	add_test(NAME ${_name} COMMAND $<TARGET_FILE:${_name}> --success)
endfunction()

add_catch_test(btree_test)
add_catch_test(callback_test)
add_catch_test(com_test)
add_catch_test(concurrent_stack_test)
add_catch_test(dynamic_library_test)
add_catch_test(exception_test)
add_catch_test(executable_test)
add_catch_test(filesystem_test)
add_catch_test(log_test)
add_catch_test(lru_cache_test)
add_catch_test(mapped_file_test)
add_catch_test(mapped_vector_test)
add_catch_test(memory_test)
add_catch_test(meta_test)
add_catch_test(parse_test)
add_catch_test(process_test)
add_catch_test(rpc_test)
add_catch_test(shared_mem_obj_test)
add_catch_test(socket_test)
add_catch_test(source_location_test)
add_catch_test(string_test)
add_catch_test(tuple_test)
add_catch_test(type_info_test)
add_catch_test(unique_id_test)
add_catch_test(var_test)
add_catch_test(wrapped_type_test)

add_library(dummy_library SHARED dummy_library.cpp)
target_link_libraries(dummy_library PRIVATE xtl)
add_dependencies(dynamic_library_test dummy_library)