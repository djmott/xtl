/** @file
natural language processing
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{



    namespace _ {
      struct noun;
      struct adjective;
      struct verb;
      struct adverb;
      struct pronoun;
      struct preposition;
      struct determiner;
      struct interjection;
      struct conjunction;
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


      struct noun : lexical_category_impl<noun>{};
      struct adjective : lexical_category_impl<adjective>{};
      struct verb : lexical_category_impl<verb>{};
      struct adverb : lexical_category_impl<adverb>{};
      struct pronoun : lexical_category_impl<pronoun>{};
      struct preposition : lexical_category_impl<preposition>{};
      struct determiner : lexical_category_impl<determiner>{};
      struct interjection : lexical_category_impl<interjection>{};
      struct conjunction : lexical_category_impl<conjunction>{};

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



#include "wordnet.hpp"
#include "moby.hpp"
#include "pos.hpp"
#include "english.hpp"

#include "document.hpp"