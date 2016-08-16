/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <iostream>

namespace xtd{
  namespace nlp{

    class english{
    public:
      using pointer = std::shared_ptr<english>;

      enum parts_of_speech{
        adj                 = 0x0000000000000007,
        adj_all             = 0x0000000000000001,
        adj_ppl             = 0x0000000000000002,
        adj_pert	          = 0x0000000000000004,
        adv                 = 0x0000000000000008,
        adv_all         	  = 0x0000000000000008,
        noun                = 0x000000003ffffff0,
        noun_Tops          	= 0x0000000000000010,
        noun_act	          = 0x0000000000000020,
        noun_animal	        = 0x0000000000000040,
        noun_artifact	      = 0x0000000000000080,
        noun_attribute    	= 0x0000000000000100,
        noun_body         	= 0x0000000000000200,
        noun_cognition	    = 0x0000000000000400,
        noun_communication	= 0x0000000000000800,
        noun_event	        = 0x0000000000001000,
        noun_feeling	      = 0x0000000000002000,
        noun_food	          = 0x0000000000004000,
        noun_group	        = 0x0000000000008000,
        noun_location	      = 0x0000000000010000,
        noun_motive	        = 0x0000000000020000,
        noun_object	        = 0x0000000000040000,
        noun_person	        = 0x0000000000080000,
        noun_phenomenon	    = 0x0000000000100000,
        noun_plant	        = 0x0000000000200000,
        noun_possession	    = 0x0000000000400000,
        noun_process	      = 0x0000000000800000,
        noun_quantity	      = 0x0000000001000000,
        noun_relation	      = 0x0000000002000000,
        noun_shape	        = 0x0000000004000000,
        noun_state	        = 0x0000000008000000,
        noun_substance	    = 0x0000000010000000,
        noun_time	          = 0x0000000020000000,
        verb	              = 0x00001fffc0000000,
        verb_body	          = 0x0000000040000000,
        verb_change	        = 0x0000000080000000,
        verb_cognition	    = 0x0000000100000000,
        verb_communication  = 0x0000000200000000,
        verb_competition	  = 0x0000000400000000,
        verb_consumption	  = 0x0000000800000000,
        verb_contact	      = 0x0000001000000000,
        verb_creation	      = 0x0000002000000000,
        verb_emotion	      = 0x0000004000000000,
        verb_motion	        = 0x0000008000000000,
        verb_perception	    = 0x0000010000000000,
        verb_possession	    = 0x0000020000000000,
        verb_social	        = 0x0000040000000000,
        verb_stative        = 0x0000080000000000,
        verb_weather        = 0x0000100000000000,
      };

      struct lemma{
        using pointer = std::shared_ptr<lemma>;
        using vector = std::vector<pointer>;

        const std::string& value() const { return _value; }
        parts_of_speech part_of_speech() const { return _part_of_speech; }
        lemma(const std::string& val) : _value(val){}

      private:
        std::string _value;
        parts_of_speech _part_of_speech;
      };


<<<<<<< HEAD
      lemma::vector& lemmata() { return _lemmata; }
      const lemma::vector lemmata() const { return _lemmata; }
=======

      english() : _wordnet(){
        for (const auto & oRecord : _wordnet._data_adj.records){
          for (const auto & oWord : oRecord.second.words){
            std::cout << oWord.word << std::endl;
          }

        }
      }
>>>>>>> branch 'master' of git@github.com:djmott/xtl.git

    private:
<<<<<<< HEAD
      lemma::vector _lemmata;
=======
      wordnet::database _wordnet;
      lemma::lemmata _lemmata;
>>>>>>> branch 'master' of git@github.com:djmott/xtl.git
    };
  }
}
