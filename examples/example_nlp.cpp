/**
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */


struct IUnknown;

#include <xtd/xtd.hpp>

#include <xtd/decorated_object.hpp>

#include <xtd/string.hpp>


namespace xtd{
  namespace nlp{

    template <typename _SuperT> struct sentence;
    template <typename _SuperT> struct word;

    template <typename _SuperT> struct word : _SuperT{
      int squeegy(){
        return 0;
//        _SuperT::template policy<sentence>().fnord();
      }
    };

    template <typename _SuperT> struct sentence : _SuperT{
      void fnord(){
        _SuperT::template policy<word>().squeegy();
      }
    };

    template <typename _SuperT> struct paragraph : _SuperT{

    };

    template <typename _SuperT> struct chapter : _SuperT{

    };

    template <typename _SuperT> struct book : _SuperT{

    };

    template <typename _SuperT> struct corpus : _SuperT{

    };

    template <typename _SuperT> struct library : _SuperT{

    };


    using text_process = decorated_object<sentence, word>;

  }
}



int main(){
  using namespace xtd::nlp;
  text_process oProcess;
  oProcess.fnord();
  auto& oWord = oProcess.policy<word>();
  oWord.squeegy();
//   using _temp_t = typename text_process::AwesomeReturnType<word>;
//   std::cout << typeid(_temp_t).name() << std::endl;
}


#if 1
#elif 1
#include <xtd/nlp/simple_SBD.hpp>
#include <xtd/nlp/porter_stemmer.hpp>
#include <xtd/nlp/ngram_stemmer.hpp>
#include <xtd/nlp/simple_PBD.hpp>
#include <xtd/nlp/document.hpp>
#elif 1

int main(){
  using namespace xtd::nlp;
  using doc_type = document<PBD::simple, SBD::simple, stemmer::trigram>;
  doc_type oDoc(xtd::filesystem::path("c:/dev/dat/customer_unique.txt"));
  for (auto & oParagraph : oDoc.paragraphs()){
    for (auto & oSentence : oParagraph.sentences()){

    }
  }
}

#elif 1

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
  xtd::nlp::stemmer::porter oStemmer;
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
    for (auto& oWord : oDoc){
      oWord.item<xtd::nlp::stem>() = oStemmer(oWord.item<xtd::nlp::raw_text>());
      std::cout << *oWord.item<xtd::nlp::stem>() << " ";
    }
    std::cout << std::endl;

    xtd::nlp::pos_tagger::tag_doc(oDoc);

  };

  return 0;
}
#endif
