/** @file
histogram of words in a document
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd{
  namespace nlp{
    class english{
    public:
      english(const xtd::filesystem::path& oWordnetPath, const xtd::filesystem::path& oMobyPath){
        std::shared_ptr<wordnet::database> oWordNet;
        std::shared_ptr<moby::database> oMoby;
        auto own = std::async(std::launch::async, [ &oWordNet, &oWordnetPath](){ oWordNet = std::shared_ptr<wordnet::database>(new wordnet::database(oWordnetPath)); });
        auto omo = std::async(std::launch::async, [ &oMoby, &oMobyPath](){ oMoby = std::shared_ptr<moby::database>(new moby::database(oMobyPath)); });
        own.get();
        omo.get();
      }
    };
  }
}