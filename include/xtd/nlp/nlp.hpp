/** @file
natural language processing
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <vector>
#include <typeinfo>
#include <string>
#include <fstream>
#include <map>

#include <xtd/meta.hpp>
#include <xtd/string.hpp>
#include <xtd/var.hpp>
#include <xtd/type_info.hpp>

namespace xtd{
  namespace nlp{

    class string_vector : public std::vector<xtd::string>{
      using _super_t = std::vector<xtd::string>;
    public:
      template <typename ... _ArgTs> string_vector(_ArgTs&&...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}
      template <typename _Ty> string_vector& operator=(_Ty&& rhs){
        _super_t::operator=(std::forward<_Ty>(rhs));
        return *this;
      }
    };


    namespace _ {
      class noun;
      class adjective;
      class verb;
      class adverb;
      class pronoun;
      class preposition;
      class determiner;
      class interjection;
      class conjunction;
    }


    class lexical_category{
    public:
      using vector = std::vector<lexical_category>;
      virtual ~lexical_category(){}
      virtual const std::type_info& type() const { return typeid(lexical_category); }
      virtual bool is_a(const std::type_info& oType) const { return type() == oType; }

      using noun = _::noun;
      using adjective = _::adjective;
      using verb = _::verb;
      using adverb = _::adverb;
      using pronoun = _::pronoun;
      using preposition = _::preposition;
      using determiner = _::determiner;
      using interjection = _::interjection;
      using conjunction = _::conjunction;
    };

    namespace _{



      template <typename _Ty> class lexical_category_impl : public lexical_category{
      public:
        virtual const std::type_info& type() const override { return typeid(_Ty); }
        virtual bool is_a(const std::type_info& oType) const override { return (type() == oType); }
        const std::string& value() const { return _value; }
      protected:
        std::string _value;
      };


      class noun : public lexical_category_impl<noun>{};
      class adjective : public lexical_category_impl<adjective>{};
      class verb : public lexical_category_impl<verb>{};
      class adverb : public lexical_category_impl<adverb>{};
      class pronoun : public lexical_category_impl<pronoun>{};
      class preposition : public lexical_category_impl<preposition>{};
      class determiner : public lexical_category_impl<determiner>{};
      class interjection : public lexical_category_impl<interjection>{};
      class conjunction : public lexical_category_impl<conjunction>{};

      class sentence;
      class noun_phrase;
      class prepositional_phrase;
      class verb_phrase;

      template <typename _Ty> struct opt{
        using type = _Ty;
      };

    }



    struct BOW{
      std::map<std::string, size_t> operator()(const std::vector<xtd::string>& oDoc) const{
        std::map<std::string, size_t> oRet;
        for (const auto& oItem : oDoc){
          ++oRet[oItem];
        }
        return oRet;
      }
    };


    struct SimpleParse{
      std::vector<xtd::string> operator()(const std::string& src) const {
        return xtd::string(src).split({'.', '!', '?', ' ', ','});
      }
    };



  }
}



#include "pos.hpp"
#include "english.hpp"

#include "document.hpp"
