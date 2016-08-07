/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

namespace xtd{
  namespace nlp{
    struct word : xtd::string{
      using pointer = std::shared_ptr<word>;
      using vector = std::vector<pointer>;

      /// sorted word lists grouped by length in characters
      
      using index_by_length = std::map<size_type, vector>;

      template <typename ... _ArgTs> word(_ArgTs...oArgs) : xtd::string(std::forward<_ArgTs>(oArgs)...){}


    };


  }
}
