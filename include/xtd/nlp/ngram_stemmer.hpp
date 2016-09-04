/* @file
n-gram based statistical stemmer
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/
#pragma once

#include <xtd/xtd.hpp>
#include <fstream>
#include <map>
#include <xtd/nlp/document.hpp>
#include <xtd/filesystem.hpp>


namespace xtd{
  namespace nlp{
    namespace stemmer{
      template <typename _DocumentT, size_t _NGramWidth>
      class ngram{
        using histogram = std::map<char[_NGramWidth], size_t>;
        size_t _ngram_width;
        histogram _histogram;
      public:
        ngram() = default;

        void train(const _DocumentT& oDoc){
        }

      };
      template <typename _DocumentT> using bigram = ngram<_DocumentT, 2>;
      template <typename _DocumentT> using trigram = ngram<_DocumentT, 3>;
      template <typename _DocumentT> using quadgram = ngram<_DocumentT, 4>;
      template <typename _DocumentT> using pentgram = ngram<_DocumentT, 5>;
    }
  }
}