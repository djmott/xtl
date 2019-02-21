/** @file
simple sentence boundary detector
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>
#include <vector>
#include <xtd/string.hpp>

namespace xtd{
  namespace nlp{
    namespace SBD{

      template <typename _DocumentT>
      struct simple{
        void operator()(_DocumentT& oDoc){
          const xtd::cstring& sOriginalText = oDoc.template item<typename _DocumentT::original_text>();
          std::vector<size_t> PositionScore(sOriginalText.size(), 0);
          //scan for question marks
          size_t iCurrPos=0;
          for (; xtd::cstring::npos != iCurrPos; iCurrPos = sOriginalText.find('?', iCurrPos)){
            if (!iCurrPos) continue;
            PositionScore[iCurrPos] += 10;
          }
          //scan for exclamation points
          iCurrPos = 0;
          for (; xtd::cstring::npos != iCurrPos; iCurrPos = sOriginalText.find('!', iCurrPos)){
            if (!iCurrPos) continue;
            PositionScore[iCurrPos] += 10;
          }

        }
      };

    }
  }
}
