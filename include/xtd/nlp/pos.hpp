/** @file
pos tagging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    namespace pos{
        
        struct pos_base{
            using vector = std::vector<pos_base>;
            virtual const std::type_info& type() const { return typeid(pos_base); }
            virtual bool is_a(const std::type_info& oType) const { return type() == oType; }
        };
        
        template <typename _Ty, typename _SuperT = pos_base> struct pos_impl : _SuperT{
            virtual const std::type_info& type() const override { return typeid(_Ty); }
            virtual bool is_a(const std::type_info& oType) const override { return (type() == oType) ? true : _SuperT::is_a(oType); }
        };
        
        struct word : pos_base{
            virtual const std::string& value() const { return _value; }
        protected:
            std::string _value;
        };
        
        template <typename _Ty>
        struct word_impl : pos_impl<_Ty, word>{

        };
        
        struct noun : word_impl<noun>{};
        struct adjective : word_impl<adjective>{};
        struct verb : word_impl<verb>{};
        struct adverb : word_impl<adverb>{};
        struct pronoun : word_impl<pronoun>{};
        struct preposition : word_impl<preposition>{};
        struct determiner : word_impl<determiner>{};
        struct interjection : word_impl<interjection>{};

        struct sentence;
        struct noun_phrase;
        struct prepositional_phrase;
        struct verb_phrase;
        
        template <typename _Ty> struct opt{
            using type = _Ty;
        };
    }
  }
}


