/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */
#include <xtd/xtd.hpp>

#include <fstream>
#include <map>
#include <vector>
#include <locale>

#include <xtd/nlp/nlp.hpp>


using char_histogram = std::vector<size_t>;

char_histogram get_char_histogram(const xtd::string& src){
  char_histogram oRet(255, 0);
  for (char ch : src){
    oRet[128+ch]++;
  }
  return oRet;
}


using word_histogram = std::map<xtd::string, size_t>;
word_histogram get_word_histogram(const xtd::string& src){
  word_histogram oRet;
  return oRet;
}

int main(){
  using namespace xtd::nlp;
  std::ifstream in;
  in.open("d:/tmp/MarkTwain/CompleteWorks.txt");
  if (in.bad()) return -1;
  xtd::string sDoc((xtd::string((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()))));
  sDoc.replace({'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\r', '\n', '\t', '~', '`', '!','@', '#', '$', '%', '^',  '&', '*','(',')','-','_','+','=','[','{',']','}','\\', '|', ';', ':', '\'', '"', '<',',','>','.','/','?'}, ' ').to_lower(std::locale("en-US"));
  auto sWords = sDoc.split({' '}, true);
  std::map<xtd::string, size_t> oUnigrams;
  for (const auto& sWord : sWords){
    oUnigrams[sWord]++;
  }
  std::map<xtd::string, size_t> oReversed;
  size_t iMaxLen = 0;
  for (const auto & oItem : oUnigrams){
    xtd::string sNewval = oItem.first;
    if (sNewval.size() > iMaxLen) iMaxLen = sNewval.size();
    sNewval.reverse();
    oReversed[sNewval] = oItem.second;
  }
  std::map<size_t, std::map<std::string, size_t>> oEndings;

  auto GetEndOfLength = [oReversed](size_t iLen)->std::map<std::string, size_t>{
    std::map<std::string, size_t> oRet;
    for (const auto & oEntry : oReversed){
      if (oEntry.first.size() < iLen) continue;
      xtd::string sKey = oEntry.first;
      oRet[sKey.substr(0, iLen)] += oEntry.second;
    }
    return oRet;
  };

  for (size_t i = 1; i <= iMaxLen; ++i){
    oEndings.insert(std::make_pair(i, GetEndOfLength(i)));
  }


  

  for (const auto & oItem : oEndings){
    std::vector<size_t> oCounts;
    std::ofstream out;
    out.open(xtd::string::format("d:/tmp/endings/", oItem.first, ".csv").c_str());
    for (const auto & oEnding : oItem.second){
      out << oEnding.first << "," << oEnding.second << std::endl;
    }

  }




  return oEndings.size();
  /*
  raw_document oDoc;
  oDoc.value<raw_document::text>() = xtd::string((xtd::string((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()))));
  auto oHist = get_char_histogram(oDoc.value<raw_document::text>());

  DUMP(oHist.size());
  return oHist.size();
   */
}

