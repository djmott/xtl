/** @file
* std::type_info extensions
* @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <typeinfo>

inline bool operator<(const std::type_info& lhs, const std::type_info& rhs){
  return lhs.before(rhs);
}

