/** @file
nlp document
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <fstream>
#include <ios>

#include <xtd/dynamic_object.hpp>
#include <xtd/wrapped_type.hpp>


namespace xtd{
  namespace nlp{

    enum word_category_t : uint64_t{
      unknown_cat = 0,
      adj_all = 1,
      adj_pert = (adj_all << 1),
      adv_all = (adj_pert << 1),
      noun_Tops = (adv_all << 1),
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
      verb_body = (noun_time << 1),
      verb_change = (verb_body << 1),
      verb_cognition = (verb_change << 1),
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
      adj_ppl = (verb_weather << 1),
    };

    enum part_of_speech_t : uint16_t{
      unknown_pos = 0,
      noun = 1,
      plural = (noun << 1),
      noun_phrase = (plural << 1),
      verb_participle = (noun_phrase << 1),
      verb_transitive = (verb_participle << 1),
      verb_intransitive = (verb_transitive << 1),
      adjective = (verb_intransitive << 1),
      adverb = (adjective << 1),
      conjunction = (adverb << 1),
      preposition = (conjunction << 1),
      interjection = (preposition << 1),
      pronoun = (interjection << 1),
      definite_article = (pronoun << 1),
      indefinite_article = (definite_article << 1),
      nominative = (indefinite_article << 1),
    };

    template <template <typename> class _StemmerT>
    class word : public dynamic_object{
    public:
      using _my_t = word<_StemmerT>;
      using stemmer_type = _StemmerT<_my_t>;
      using original_text = WRAPPED(xtd::string);
      using vector = std::vector<word>;
      using word_category = WRAPPED(word_category_t);
      using part_of_speech = WRAPPED(part_of_speech_t);
    };


    template <template <typename> class _StemmerT>
    class sentence : public dynamic_object{
    public:
      using _my_t = sentence<_StemmerT>;
      using stemmer_type = _StemmerT<_my_t>;
      using word_type = word<_StemmerT>;
      using original_text = WRAPPED(xtd::string);
      using vector = std::vector<sentence>;

    };


    template <template <typename> class _SBDT, template <typename> class _StemmerT>
    class paragraph : public dynamic_object{
    public:
      using _my_t = paragraph<_SBDT, _StemmerT>;
      using sentence_boundary_detector_type = _SBDT<_my_t>;
      using sentence_type = sentence<_StemmerT>;

      using original_text = WRAPPED(xtd::string);
      
      using vector = std::vector<paragraph>;

      sentence_boundary_detector_type& sentence_boundary_detector(){
        if (!dynamic_object::has_item<sentence_boundary_detector_type>()){
          dynamic_object::item<sentence_boundary_detector_type>() = sentence_boundary_detector_type();
        }
        return dynamic_object::item<sentence_boundary_detector_type>();
      }

      typename sentence_type::vector& sentences(){
        if (!dynamic_object::has_item<typename sentence_type::vector>()){
          (sentence_boundary_detector())(*this);
        }
        return dynamic_object::item<typename sentence_type::vector>();
      }

    };


    template <template <typename> class _PBDT, template <typename> class _SBDT, template <typename> class _StemmerT>
    class document : public dynamic_object{
    public:
      using _my_t = document<_PBDT, _SBDT, _StemmerT>;

      using paragraph_bound_detector_type = _PBDT<_my_t>;
      using paragraph_type = paragraph<_SBDT, _StemmerT>;

      using original_text = WRAPPED(xtd::string);


      document(const xtd::filesystem::path& oPath){
        std::ifstream in;
        in.exceptions(std::ios::badbit | std::ios::failbit);
        in.open(oPath, std::ios::in);
        dynamic_object::item<original_text>() = std::move(xtd::string((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>())));
      }

      paragraph_bound_detector_type& paragraph_bound_detector(){
        if (!dynamic_object::has_item<paragraph_bound_detector_type>()){
          dynamic_object::item<paragraph_bound_detector_type>() = paragraph_bound_detector_type();
        }
        return dynamic_object::item<paragraph_bound_detector_type>();
      }

      typename paragraph_type::vector& paragraphs(){
        if (!dynamic_object::has_item<typename paragraph_type::vector>()){
          (paragraph_bound_detector())(*this);
        }
        return dynamic_object::item<typename paragraph_type::vector>();
      }

    };

  }



  namespace _{
    template<>
    class xstring_format<char, const nlp::part_of_speech_t &>{
    public:
      inline static string format(const nlp::part_of_speech_t &iPOS){
        using namespace xtd::nlp;
        xtd::string sRet = "(";
        if (part_of_speech_t::unknown_pos == iPOS) sRet += "unknown   ";
        if (part_of_speech_t::noun & iPOS) sRet += "noun | ";
        if (part_of_speech_t::plural & iPOS) sRet += "plural | ";
        if (part_of_speech_t::noun_phrase & iPOS) sRet += "noun_phrase | ";
        if (part_of_speech_t::verb_participle & iPOS) sRet += "verb_participle | ";
        if (part_of_speech_t::verb_transitive & iPOS) sRet += "verb_transitive | ";
        if (part_of_speech_t::verb_intransitive & iPOS) sRet += "verb_intransitive | ";
        if (part_of_speech_t::adjective & iPOS) sRet += "adjective | ";
        if (part_of_speech_t::adverb & iPOS) sRet += "adverb | ";
        if (part_of_speech_t::conjunction & iPOS) sRet += "conjunction | ";
        if (part_of_speech_t::preposition & iPOS) sRet += "preposition | ";
        if (part_of_speech_t::interjection & iPOS) sRet += "interjection | ";
        if (part_of_speech_t::pronoun & iPOS) sRet += "pronoun | ";
        if (part_of_speech_t::definite_article & iPOS) sRet += "definite_article | ";
        if (part_of_speech_t::indefinite_article & iPOS) sRet += "indefinite_article | ";
        if (part_of_speech_t::nominative & iPOS) sRet += "nominative | ";
        sRet.resize(sRet.size() - 3);
        sRet += ")";
        return sRet;
      }
    };
  }
}


