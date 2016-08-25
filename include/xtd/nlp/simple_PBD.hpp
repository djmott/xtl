/** @file
simple paragraph boundary detector
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
namespace xtd{
  namespace nlp{
    namespace PBD{
      template <typename _DocumentT>
      struct simple{
        
        void operator()(_DocumentT& oDoc){
          TODO("Implement proper paragraph detector");
          using paragraph_type = typename _DocumentT::paragraph_type;
          const xtd::string & oOriginalText = oDoc.item<_DocumentT::original_text>();
          paragraph_type oParagraph;
          oParagraph.item<typename paragraph_type::original_text>() = oOriginalText;
          oDoc.item<paragraph_type>() = std::move(oParagraph);
          /*
          auto oStart = oOriginalText.begin();
          for (auto oCurrPos = oStart; oOriginalText.end() != oCurrPos; ++oCurrPos){
            if ("\n\n" == *oCurrPos){

            }
          }
          */

        }
      };
    }
  }
}