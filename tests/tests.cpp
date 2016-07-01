/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd.hpp>
#include <xtd/parse.hpp>
#include <xtd/callback.hpp>
#include <xtd/data_convert.hpp>
#include <xtd/dynamic_library.hpp>
#include <xtd/string.hpp>
#include <xtd/executable.hpp>
#include <xtd/event_trace.hpp>

#include <xtd/exception.hpp>
#include <xtd/executable.hpp>
#include <xtd/log.hpp>

#include <xtd/meta.hpp>
#include <xtd/path.hpp>

#include <xtd/socket.hpp>

#include <thread>
#include <future>
#include <chrono>
#include <xtd/string.hpp>

#include <xtd/unique_id.hpp>

#include <xtd/var.hpp>


#if (XTD_COMPILER_MSVC & XTD_COMPILER)
#pragma warning(push, 0)
#endif

#include "gtest/gtest.h"

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
#pragma warning(pop)
#endif



#if (XTD_CI_BUILD)
  #define CI_DISABLE(x) DISABLED_##x
#else
  #define CI_DISABLE(x) x
#endif

#include "test_callback.hpp"
#include "test_data_convert.hpp"
#include "test_dynamic_library.hpp"
#include "test_event_trace.hpp"
#include "test_exception.hpp"
#include "test_executable.hpp"
#include "test_logging.hpp"
#include "test_meta.hpp"
#include "test_parse.hpp"
#include "test_path.hpp"
#include "test_socket.hpp"
#include "test_string.hpp"
#if (XTD_HAS_UUID)
  #include "test_unique_id.hpp"
#endif
#include "test_var.hpp"


int main(int argc, char *argv[]){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

