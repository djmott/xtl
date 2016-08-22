/** @file
pos tagging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <algorithm>
#include <vector>
#include <map>

#include <xtd/meta.hpp>
#include <xtd/string.hpp>

#include <xtd/nlp/nlp.hpp>
#include <xtd/dynamic_object.hpp>

#include <xtd/nlp/moby.hpp>


namespace xtd{
  namespace nlp{

    class pos_tagger{
    public:

      struct tagged_word {
        part_of_speech _pos;
        xtd::string _word;
      };

      static void tag_doc(xtd::dynamic_object::vector& oDoc) {
        using doc_type = xtd::dynamic_object::vector;
        auto & oMoby = moby::database::get();
        forever{
          auto oItem = std::find_if(oDoc.begin(), oDoc.end(), [](const xtd::dynamic_object& obj) { return obj.has_item<xtd::nlp::raw_text>(); });
          if (oDoc.end() == oItem) break;
          auto sRawText = oItem->item<raw_text>();
          auto iPOS = moby::database::get().get_pos(sRawText);
          DUMP(iPOS);
          oItem->item<tagged_word>() = tagged_word{ iPOS, sRawText };
          oItem->remove_item<raw_text>();
        }
      }

      pos_tagger() = default;

    };
  }
}


