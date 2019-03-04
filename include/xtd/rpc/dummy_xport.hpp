/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#if !defined(__XTD_RPC_HPP_INCLUDED__)
#error Dont include this file directly. Instead include rpc.hpp
#endif

namespace xtd {
  namespace rpc {

    template <stub _stub, typename _impl_t>
    struct dummy_transport{
      void write(const payload&){}
      void read(payload&){}
    };

  }
}