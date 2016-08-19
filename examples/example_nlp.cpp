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

#include <xtd/dynamic_object.hpp>
#include <xtd/nlp/nlp.hpp>
#include <xtd/parse.hpp>
#include <xtd/nlp/wordnet.hpp>

namespace commands{
  using namespace xtd::parse;
  namespace strings{
    STRING_(quit);
    STRING_(exit);
  }
  struct quit_command : rule<quit_command, or_<strings::quit, strings::exit>>{
    template <typename ... _Ty>
    quit_command(_Ty&&...params) : rule(std::forward<_Ty>(params)...){}
  };
}

xtd::nlp::english::pointer _english;

struct ChatSession{

};
int main(){

//   wordnet::database oDB;

  std::cout << "Chatty Cathy: Hello! What can I do for you?" << std::endl;

  std::string sLine;
  forever{
    std::cin >> sLine;
    auto oQuit = xtd::parser<commands::quit_command, true, xtd::parse::whitespace<'\t', ' '>>::parse(sLine.begin(), sLine.end());
    if (oQuit) break;
  };

  return 0;
}
#endif
