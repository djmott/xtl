set(TEST_FILES
  btree_test.cpp
  callback_test.cpp
  concurrent_hash_map_test.cpp
  concurrent_queue_test.cpp
  concurrent_recursive_spin_lock_test.cpp
  concurrent_rw_lock_test.cpp
  concurrent_spin_lock_test.cpp
  concurrent_stack_test.cpp
  concurrent_test.cpp
  debug_test.cpp
  decorated_object_test.cpp
  dynamic_library_test.cpp
  dynamic_object_test.cpp
  event_trace_test.cpp
  exception_test.cpp
  executable_test.cpp
  filesystem_test.cpp
  log_test.cpp
  lru_cache_test.cpp
  mapped_file_test.cpp
  mapped_vector_test.cpp
  memory_test.cpp
  meta_test.cpp
  parse_test.cpp
  process_test.cpp
  rpc_test.cpp
  shared_mem_obj_test.cpp
  socket_test.cpp
  source_location_test.cpp
  tuple_test.cpp
  type_info_test.cpp
  unique_id_test.cpp
  var_test.cpp
  wrapped_type_test.cpp
  xstring_test.cpp
)

if(WIN32)
	list(APPEND TEST_FILES
		win_debug_help_test.cpp
		win_image_help_test.cpp
		win_pipe_test.cpp
		win_service_test.cpp
		com_class_factory_test.cpp
		com_data_object_test.cpp
		com_dll_test.cpp
		com_exception_test.cpp
		com_initializer_test.cpp
		com_object_test.cpp
		com_pointer_test.cpp
		com_test.cpp
	)
endif()

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

foreach(_item ${TEST_FILES})
  get_filename_component(_name ${_item} NAME_WE)
	add_executable(${_name} ${_item})
	target_link_libraries(${_name} PRIVATE xtl catch2)
	add_test(NAME ${_name} COMMAND $<TARGET_FILE:${_name}> --success)
endforeach()

add_library(dummy_library SHARED dummy_library.cpp)
target_link_libraries(dummy_library PRIVATE xtl)
add_dependencies(dynamic_library_test dummy_library)