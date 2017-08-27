/** @file
main system and unit test entry point 
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#define _WIN32_WINNT 0x600

#include <xtd/xtd.hpp>

#include <string>

#include "gtest/gtest.h"

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma warning(push, 0)
#endif


#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma warning(pop)
#endif

#if (ON==TEST_COM) && (XTD_COMPILER_MSVC & XTD_COMPILER)
  #include "test_com.hpp"
#endif

#if (ON==TEST_BTREE)
  #include "test_btree.hpp"
#endif

#if (ON==TEST_CALLBACK)
  #include "test_callback.hpp"
#endif

#if (ON==TEST_CONCURRENT_HASH_MAP)
  #include "test_hash_map.hpp"
#endif

#if (ON==TEST_CONCURRENT_STACK)
  #include "test_concurrent_stack.hpp"
#endif

#if (ON==TEST_DEBUG_HELP && (XTD_OS_WINDOWS & XTD_OS))
  #include "test_debug_help.hpp"
#endif

#if (ON==TEST_DYNAMIC_LIBRARY)
  #include "test_dynamic_library.hpp"
#endif

#if (ON==TEST_EVENT_TRACE)
  #include "test_event_trace.hpp"
#endif

#if (ON==TEST_EXCEPTION)
  #include "test_exception.hpp"
#endif

#if (ON==TEST_EXCUTABLE)
  #include "test_executable.hpp"
#endif

#if (ON==TEST_LOGGING)
  #include "test_logging.hpp"
#endif

#if (ON==TEST_META)
  #include "test_meta.hpp"
#endif

#if (ON==TEST_MAPPED_FILE)
  #include "test_mapped_file.hpp"
#endif

#if (ON==TEST_MAPPED_VECTOR)
  #include "test_mapped_vector.hpp"
#endif

#if (ON==TEST_PARSE)
  #include "test_parse.hpp"
#endif

#if (ON==TEST_PATH)
  #include "test_path.hpp"
#endif

#if (ON==TEST_PROCESS)
  #include "test_process.hpp"
#endif

#if (ON==TEST_READ_WRITE_LOCK)
  #include "test_rw_lock.hpp"
#endif

#if (ON==TEST_RECURSIVE_SPIN_LOCK)
  #include "test_recursive_spin_lock.hpp"
#endif

#if (ON==TEST_RPC)
  #include "test_rpc.hpp"
#endif

#if (ON==TEST_SHARED_MEM_OBJ)
  #include "test_shared_mem_obj.hpp"
#endif

#if (ON==TEST_SOCKET)
  #include "test_socket.hpp"
#endif

#if (ON==TEST_SOCKET)
  #include "test_source_location.hpp"
#endif

#if (ON==TEST_SPIN_LOCK)
  #include "test_spin_lock.hpp"
#endif

#if (ON==TEST_STACK)
  #include "test_stack.hpp"
#endif

#if (ON==TEST_STRING)
  #include "test_string.hpp"
#endif

#if (ON==TEST_UNIQUE_ID && XTD_HAS_UUID)
  #include "test_unique_id.hpp"
#endif

#if (ON==TEST_VAR)
  #include "test_var.hpp"
#endif

#undef forever
#include "src/gtest-all.cc"

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
