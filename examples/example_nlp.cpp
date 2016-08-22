/**
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */




#include <xtd/xtd.hpp>

#include <iterator>
#include <algorithm>
#include <future>
#include <iostream>

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
      std::cout << "Chatty Cathy is an English comprehension and execution engine." << std::endl;
      std::cout << "Commands:" << std::endl;
      std::cout << "\tExit/Quit - Leave Chatty Kathy" << std::endl;
      std::cout << "\tHelp - This info" << std::endl;
    }
  };
}


int main(){
  using namespace xtd;
  using namespace xtd::nlp;
  auto oMoby = std::async(std::launch::async, [](){ moby::database::get(); });
  const char * sCathy = "Chatty Cathy >> ";
  using doc_type = std::vector<xtd::dynamic_object>;
  std::cout << sCathy << "Hello." << std::endl << sCathy << "What is your name?" << std::endl << "Enter name >> ";
  std::string sUser;
  std::getline(std::cin, sUser);
  std::cout << sCathy << "Welcome " << sUser << "." << std::endl;
  oMoby.get();
  forever{
    std::cout << sUser << " >> ";
    xtd::string sLine;
    doc_type oDoc;
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
