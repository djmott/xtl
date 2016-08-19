/** @file
natural language processing
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <algorithm>
#include <vector>

#include <xtd/string.hpp>
#include <xtd/var.hpp>


namespace xtd{
  namespace nlp{

    namespace _{

    }


    static inline std::vector<xtd::var> buparse(const xtd::string& str){
      //populate return vector
      std::vector<xtd::var> oRet;
      for (const auto ch : str){
        oRet.push_back(var(ch));
      }
      std::find_if(oRet.begin(), oRet.end(), [](const xtd::var& oVar){ return typeid(char) == oVar.get_type(); });
      return oRet;
    }
  }
}