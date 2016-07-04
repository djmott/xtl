/**
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>


#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma warning(push, 0)
#endif

#include "gtest/gtest.h"

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma warning(pop)
#endif

#undef XTD_ASSERT
#define XTD_ASSERT ASSERT_TRUE

#if (ON==TEST_CALLBACK)
  #include "test_callback.hpp"
#endif

#if (ON==TEST_DATA_CONVERT)
  #include "test_data_convert.hpp"
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

#if (ON==TEST_PARSE)
  #include "test_parse.hpp"
#endif

#if (ON==TEST_PATH)
  #include "test_path.hpp"
#endif

#if (ON==TEST_RPC)
  #include "test_rpc.hpp"
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

#if (ON==TEST_STRING)
  #include "test_string.hpp"
#endif

#if (ON==TEST_UNIQUE_ID && XTD_HAS_UUID)
  #include "test_unique_id.hpp"
#endif

#if (ON==TEST_VAR)
  #include "test_var.hpp"
#endif

int main(int argc, char *argv[]){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

