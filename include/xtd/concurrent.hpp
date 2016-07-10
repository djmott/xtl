/** @file
shared declarations for the concurrent namespace
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{

  /** @namespace xtd::concurrent
  Declarations and definitions of the concurrent algorithms library
  @defgroup Concurrent algorithms library
  @{*/
  namespace concurrent{
    ///Wait policy that does nothing. This is the default behavior.
    class null_wait_policy{
    public:
      FORCEINLINE void spin(){/*nothing*/}
    };
    ///Wait policy that yields the current thread.
    class yield_wait_policy{
    public:
      FORCEINLINE void spin(){ std::this_thread::yield(); }
    };

  }
  ///@}
}