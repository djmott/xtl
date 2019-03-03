

add_library(event_trace STATIC event_trace_client.cpp)

if(WIN32)
	if(MSVC AND 8 EQUAL ${CMAKE_SIZEOF_VOID_P})
		target_sources(event_trace PRIVATE evt_vc64.asm)
	elseif(MSVC AND 4 EQUAL ${CMAKE_SIZEOF_VOID_P})
		target_sources(event_trace PRIVATE evt_vc32.cpp)
	elseif(GNU OR Clang OR Intel)
		target_sources(event_trace PRIVATE evt_gcc.cpp)
	endif()
endif()

target_link_libraries(event_trace PUBLIC xtl)

if(MSVC)
	target_compile_options(event_trace INTERFACE /Gh)
	target_compile_options(event_trace INTERFACE /GH)
elseif(GNU)
	target_compile_options(event_trace INTERFACE -finstrument-functions)
endif()

add_executable(event_trace_server event_trace_server.cpp)
target_link_libraries(event_trace_server PRIVATE xtl)

add_executable(event_trace_test_app event_trace_test_app.cpp)
target_link_libraries(event_trace_test_app PRIVATE xtl event_trace)

target_sources(event_trace_server PRIVATE ${XTD_COM_HEADERS} ${XTD_CONCURRENT_HEADERS} ${XTD_GRAMMARS} ${XTD_NLP_HEADERS}
  ${XTD_WINDOWS_HEADERS} ${XTD_RPC_HEADERS} ${XTD_HEADERS})

target_sources(event_trace_test_app PRIVATE ${XTD_COM_HEADERS} ${XTD_CONCURRENT_HEADERS} ${XTD_GRAMMARS} ${XTD_NLP_HEADERS}
  ${XTD_WINDOWS_HEADERS} ${XTD_RPC_HEADERS} ${XTD_HEADERS})

