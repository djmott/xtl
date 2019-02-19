

add_library(event_trace STATIC event_trace.cpp)

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
