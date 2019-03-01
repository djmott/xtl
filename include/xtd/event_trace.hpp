/** @file
 generate event-trace diagrams of execution flow
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

 */

#pragma once

#include <xtd/xtd.hpp>
#include <xtd/xstring.hpp>
#include <xtd/process.hpp>

namespace xtd{

  namespace event_trace{
    enum class message_type{
      none=0,
      fatal,
      error,
      warning,
      info,
      debug,
      enter,
      leave,
    };

    static void start(){
      xtd::process::create("./event_trace_server", { xtd::cstring::Format("-", pthread_self()) });
    }

  };

}