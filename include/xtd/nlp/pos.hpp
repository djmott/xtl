/** @file
pos tagging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    namespace pos{
        
        struct pos{
            using vector = std::vector<pos>;
            using document = std::vector<vector>;
            virtual const std::type_info& type() const { return typeid(pos); }
            virtual bool is_a(const std::type_info& oType) const { return type() == oType; }
            static document tag()
        };
        
        template <typename _Ty> struct pos_impl : pos{
            virtual const std::type_info& type() const override { return typeid(_Ty); }
        };
        
        
        struct noun : pos_impl<noun>{};
        struct adjective : pos_impl<adjective>{};
        struct verb : pos_impl<verb>{};
        struct adverb : pos_impl<adverb>{};
        struct pronoun : pos_impl<pronoun>{};
        struct preposition : pos_impl<preposition>{};
        struct determiner : pos_impl<determiner>{};
        struct conjunction : pos_impl<conjunction>{};
        struct interjection : pos_impl<interjection>{};

    }
  }
}


