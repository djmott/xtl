/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/log.hpp>

TEST(test_logging, CI_DISABLE(message_types)){
  FATAL("fatal message");
  ERR("error message");
  WARNING("warning message");
  INFO("info message");
  DBG("debug message");
}

