#include <xtd/xtd.hpp>
#include <xtd/nlp/pos.hpp>
#include "wordnet.hpp"



void import_xtd_pos(wordnet::index_file& oIndexFile, wordnet::data_file& oDataFile){
  using namespace xtd::nlp;
  part_of_speech::map oPosMap;
  for (const auto & oIndex : oIndexFile.records){
    xtd::string sLemma = oIndex.second.lemma;
    sLemma.replace({ '_' }, ' ');
    auto oPOS = oPosMap.find(sLemma);
    if (oPosMap.end() == oPOS){
      oPosMap.insert(std::make_pair(sLemma, part_of_speech()));
      oPOS = oPosMap.find(sLemma);
      oPOS->second.category = part_of_speech::unknown;
    }
    auto oDataEntry = oDataFile.records.find(oIndex.second.synset_offset);
    oPOS->second.category = part_of_speech::unknown;
  }
}

int main() {
  wordnet::database oDB;
  import_xtd_pos(oDB._index_adj, oDB._data_adj);
  return 0;
}