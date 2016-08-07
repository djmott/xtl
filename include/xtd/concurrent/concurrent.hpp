/** @file
shared declarations for the concurrent namespace
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

namespace xtd{


  namespace concurrent{
    /** @addtogroup Concurrent
    @{*/
    ///Wait policy that does nothing. This is the default behavior.
    class null_wait_policy{
    public:
      FORCEINLINE void operator ()(){/*nothing*/}
    };
    ///Wait policy that yields the current thread.
    class yield_wait_policy{
    public:
      FORCEINLINE void operator ()(){ std::this_thread::yield(); }
    };

    ///RAII pattern to automatically acquire and release the spin lock
    template <typename _Ty>
    class scope_locker{
    public:
      using spin_lock_type = _Ty;
      ~scope_locker(){ _Lock.unlock(); }
      explicit scope_locker(spin_lock_type& oLock) : _Lock(oLock){ _Lock.lock(); }
      scope_locker(const scope_locker&) = delete;
      scope_locker& operator=(const scope_locker&) = delete;

    private:
      spin_lock_type& _Lock;
    };

    ///@}
  }
  
}

#include "hash_map.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include "spin_lock.hpp"
#include "rw_lock.hpp"
#include "recursive_spin_lock.hpp"
