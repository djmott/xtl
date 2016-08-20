/**
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */



#if 0

#include <xtd/xtd.hpp>

#include <iterator>

#include <xtd/nlp/nlp.hpp>
#include <xtd/nlp/buparser.hpp>
#include <xtd/dynamic_object.hpp>

int main(){ 
  using namespace xtd;


  using doc_type = std::vector<xtd::dynamic_object>;
  doc_type oDoc;

  auto oRet = xtd::string("I WANT MY MONEY").split({ ' ' }, true);
  std::transform(oRet.begin(), oRet.end(), std::back_insert_iterator<doc_type>(oDoc), [](xtd::string str){
    xtd::dynamic_object oRet;
    oRet.item<xtd::string>() = str;
    return oRet;
  });
  return oDoc.size();

}
#elif 0
int main(){
  using namespace xtd;

  std::ifstream oFile;
  std::string sFile = "";
  oFile.open("c:\\users\\david\\documents\\fnord.txt");
  while (!oFile.eof()){
    std::string sTemp;
    std::getline(oFile, sTemp);
    sFile += sTemp;
    sFile += " ";
  }

  std::locale oLoc("en-US");
  for (char & oCH : sFile){
    if (!std::isalnum(oCH, oLoc)) oCH = ' ';
  }
  auto oWords = nlp::SimpleParse()(sFile);
  auto oBOW = nlp::BOW()(oWords);

  return 0;

}
#else


#include <xtd/xtd.hpp>

#include <iterator>
#include <algorithm>

#include <xtd/dynamic_object.hpp>
#include <xtd/parse.hpp>

#include <xtd/nlp/nlp.hpp>


namespace commands{
  using namespace xtd::parse;
  namespace strings{
    STRING_(quit);
    STRING_(exit);
    STRING_(help);
  }
  struct quit_command : rule<quit_command, or_<strings::quit, strings::exit>>{
    template <typename ... _Ty>
    quit_command(_Ty&&...params) : rule(std::forward<_Ty>(params)...){}
  };
  struct help_command : rule<help_command, strings::help> {
    template <typename ... _Ty>
    help_command(_Ty&&...params) : rule(std::forward<_Ty>(params)...) {
      std::cout << "Chatty Kathy is an English comprehension and execution engine." << std::endl;
      std::cout << "Commands:" << std::endl;
      std::cout << "\tExit/Quit - Leave Chatty Kathy" << std::endl;
      std::cout << "\tHelp - This info" << std::endl;
      std::cout << "\tGoogle <query> - Google search" << std::endl;
    }
  };
}

xtd::nlp::english::pointer _english;

struct ChatSession{

};



int main(){
  using namespace xtd;
  using namespace xtd::nlp;
  
  auto i = part_of_speech::unknown;
  i = part_of_speech::adj_ppl;
  i = part_of_speech::adj_pert;
  i = part_of_speech::adj;
  i = part_of_speech::adv;
  i = part_of_speech::noun_Tops;
  i = part_of_speech::noun_act;
  i = part_of_speech::noun_animal;
  i = part_of_speech::noun_artifact;
  i = part_of_speech::noun_attribute;
  i = part_of_speech::noun_body;
  i = part_of_speech::noun_cognition;
  i = part_of_speech::noun_communication;
  i = part_of_speech::noun_event;
  i = part_of_speech::noun_feeling;
  i = part_of_speech::noun_food;
  i = part_of_speech::noun_group;
  i = part_of_speech::noun_location;
  i = part_of_speech::noun_motive;
  i = part_of_speech::noun_object;
  i = part_of_speech::noun_person;
  i = part_of_speech::noun_phenomenon;
  i = part_of_speech::noun_plant;
  i = part_of_speech::noun_possession;
  i = part_of_speech::noun_process;
  i = part_of_speech::noun_quantity;
  i = part_of_speech::noun_relation;
  i = part_of_speech::noun_shape;
  i = part_of_speech::noun_state;
  i = part_of_speech::noun_substance;
  i = part_of_speech::noun_time;
  i = part_of_speech::noun_plural;
  i = part_of_speech::noun_phrase;
  i = part_of_speech::noun;
  i = part_of_speech::verb_cognition;
  i = part_of_speech::verb_communication;
  i = part_of_speech::verb_competition;
  i = part_of_speech::verb_consumption;
  i = part_of_speech::verb_contact;
  i = part_of_speech::verb_creation;
  i = part_of_speech::verb_emotion;
  i = part_of_speech::verb_motion;
  i = part_of_speech::verb_perception;
  i = part_of_speech::verb_possession;
  i = part_of_speech::verb_social;
  i = part_of_speech::verb_stative;
  i = part_of_speech::verb_weather;
  i = part_of_speech::verb_body;
  i = part_of_speech::verb_change;
  i = part_of_speech::verb_participle;
  i = part_of_speech::verb_transitive;
  i = part_of_speech::verb_intransitive;
  i = part_of_speech::verb;
  i = part_of_speech::conjunction;
  i = part_of_speech::preposition;
  i = part_of_speech::interjection;
  i = part_of_speech::definite_article;
  i = part_of_speech::indefinite_article;
  i = part_of_speech::nominative;
  i = part_of_speech::pronoun;


  using doc_type = std::vector<xtd::dynamic_object>;

  forever{
    xtd::string sLine;
    doc_type oDoc;
    std::cout << "Chatty Cathy : Enter a command." << std::endl << " > ";
    std::getline(std::cin, sLine);
    if (xtd::parser<commands::quit_command, true, xtd::parse::whitespace<'\t', ' '>>::parse(sLine.begin(), sLine.end())) {
      break;
    }
    if (xtd::parser<commands::help_command, true, xtd::parse::whitespace<'\t', ' '>>::parse(sLine.begin(), sLine.end())) {
      continue;
    }

    auto sLineArr = sLine.split({ ' ' }, true);
    std::transform(sLineArr.begin(), sLineArr.end(), std::back_insert_iterator<doc_type>(oDoc), [](const xtd::string&str) {
      xtd::dynamic_object oRet;
      oRet.item<xtd::nlp::raw_text>() = str;
      return oRet;
    });

    xtd::nlp::pos_tagger::tag_doc(oDoc);

  };

  return 0;
}
#endif
