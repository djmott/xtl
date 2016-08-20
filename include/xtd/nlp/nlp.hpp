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
#include <xtd/wrapped_type.hpp>
#include <xtd/debug.hpp>

namespace xtd{
  namespace nlp{


    using raw_text = WRAPPED(xtd::string);
    using lemma = WRAPPED(xtd::string);

    enum part_of_speech : uint64_t {
      unknown = 0,
      adj_ppl = 1,
      adj_pert = (adj_ppl << 1),
      adj = (adj_ppl | adj_pert),
      adv = (adj_pert << 1),
      noun_Tops = (adv << 1),
      noun_act = (noun_Tops << 1),
      noun_animal = (noun_act << 1),
      noun_artifact = (noun_animal << 1),
      noun_attribute = (noun_artifact << 1),
      noun_body = (noun_attribute << 1),
      noun_cognition = (noun_body << 1),
      noun_communication = (noun_cognition << 1),
      noun_event = (noun_communication << 1),
      noun_feeling = (noun_event << 1),
      noun_food = (noun_feeling << 1),
      noun_group = (noun_food << 1),
      noun_location = (noun_group << 1),
      noun_motive = (noun_location << 1),
      noun_object = (noun_motive << 1),
      noun_person = (noun_object << 1),
      noun_phenomenon = (noun_person << 1),
      noun_plant = (noun_phenomenon << 1),
      noun_possession = (noun_plant << 1),
      noun_process = (noun_possession << 1),
      noun_quantity = (noun_process << 1),
      noun_relation = (noun_quantity << 1),
      noun_shape = (noun_relation << 1),
      noun_state = (noun_shape << 1),
      noun_substance = (noun_state << 1),
      noun_time = (noun_substance << 1),
      noun_plural = (noun_time << 1),
      noun_phrase = (noun_plural << 1),
      noun = (noun_Tops | noun_act | noun_animal | noun_artifact | noun_attribute | noun_body | noun_cognition |
        noun_communication | noun_event | noun_feeling | noun_food | noun_group | noun_location | noun_motive |
        noun_object | noun_person | noun_phenomenon | noun_plant | noun_possession | noun_process | noun_quantity |
        noun_relation | noun_shape | noun_state | noun_substance | noun_time | noun_plural | noun_phrase),
      verb_cognition = (noun_phrase << 1),
      verb_communication = (verb_cognition << 1),
      verb_competition = (verb_communication << 1),
      verb_consumption = (verb_competition << 1),
      verb_contact = (verb_consumption << 1),
      verb_creation = (verb_contact << 1),
      verb_emotion = (verb_creation << 1),
      verb_motion = (verb_emotion << 1),
      verb_perception = (verb_motion << 1),
      verb_possession = (verb_perception << 1),
      verb_social = (verb_possession << 1),
      verb_stative = (verb_social << 1),
      verb_weather = (verb_stative << 1),
      verb_body = (verb_weather << 1),
      verb_change = (verb_body << 1),
      verb_participle = (verb_change << 1),
      verb_transitive = (verb_participle << 1),
      verb_intransitive = (verb_transitive << 1),
      verb = (verb_cognition | verb_communication | verb_competition | verb_consumption | verb_contact |
        verb_creation | verb_emotion | verb_motion | verb_perception | verb_possession | verb_social |
        verb_stative | verb_weather | verb_body | verb_change | verb_participle | verb_transitive | verb_intransitive),
      conjunction = (verb_intransitive << 1),
      preposition = (conjunction << 1),
      interjection = (preposition << 1),
      definite_article = (interjection << 1),
      indefinite_article = (definite_article << 1),
      nominative = (indefinite_article << 1),
      pronoun = (nominative << 1),
    };
  }

  template <> class _::xstring_format<char, const nlp::part_of_speech & > {
  public:
    inline static string format(const nlp::part_of_speech & iPOS) {
      xtd::string sRet = "(";
      if (nlp::part_of_speech::unknown == iPOS) sRet += "unknown";
      if (nlp::part_of_speech::adj & iPOS) sRet += "adj | ";
      if (nlp::part_of_speech::adv & iPOS) sRet += "adv | ";
      if (nlp::part_of_speech::noun & iPOS) sRet += "noun | ";
      if (nlp::part_of_speech::verb & iPOS) sRet += "verb | ";
      if (nlp::part_of_speech::conjunction & iPOS) sRet += "conjunction | ";
      if (nlp::part_of_speech::preposition & iPOS) sRet += "preposition | ";
      if (nlp::part_of_speech::interjection & iPOS) sRet += "interjection | ";
      if (nlp::part_of_speech::definite_article & iPOS) sRet += "definite_article | ";
      if (nlp::part_of_speech::indefinite_article & iPOS) sRet += "indefinite_article | ";
      if (nlp::part_of_speech::nominative & iPOS) sRet += "nominative | ";
      if (nlp::part_of_speech::pronoun & iPOS) sRet += "pronoun | ";
      sRet += ")";
      return sRet;
    }
  };

}



#include "pos.hpp"
#include "english.hpp"

#include "document.hpp"
