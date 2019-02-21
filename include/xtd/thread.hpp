/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <memory>
#include <vector>


namespace xtd {

  struct unowned_thread {
    using ptr = std::shared_ptr<unowned_thread>;
    using vector = std::vector<ptr>;
  };

}
