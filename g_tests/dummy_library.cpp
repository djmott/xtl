/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/xtd.hpp>

extern "C" {
  XTD_EXPORT int Add(int x, int y){ return x + y; }
  XTD_EXPORT const char *Echo(const char *src){ return src; }
}
