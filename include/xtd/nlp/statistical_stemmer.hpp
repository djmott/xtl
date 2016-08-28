/** @file
statical word stemmer policy
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <map>

#include <xtd/string.hpp>

namespace xtd{
  namespace nlp{

    template <typename _SuperT> struct word_statistics : _SuperT{
      using word_histogram_t = std::map<std::string, size_t>;

      using word_histogram_type = PROPERTY(word_histogram_t);


    };

    template <typename _SuperT> struct statistical_stemmer : istemmer{


    };

  }
}