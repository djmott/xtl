#pragma once

#include <xtd/xtd.hpp>

#include <fstream>
#include <sstream>

#include <xtd/string.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/debug.hpp>
#include <xtd/log.hpp>
#include <xtd/nlp/document.hpp>

namespace xtd{
  namespace nlp{
    struct wordnet{
      wordnet() : 
        _data_adj(XTD_ASSETS_DIR "/WordNet-3.0/dict/data.adj"),
        _data_adv(XTD_ASSETS_DIR "/WordNet-3.0/dict/data.adv"),
        _data_noun(XTD_ASSETS_DIR "/WordNet-3.0/dict/data.noun"),
        _data_verb(XTD_ASSETS_DIR "/WordNet-3.0/dict/data.verb"),
        _index_adj(XTD_ASSETS_DIR "/WordNet-3.0/dict/index.adj"),
        _index_adv(XTD_ASSETS_DIR "/WordNet-3.0/dict/index.adv"),
        _index_noun(XTD_ASSETS_DIR "/WordNet-3.0/dict/index.noun"),
        _index_sense(XTD_ASSETS_DIR "/WordNet-3.0/dict/index.sense"),
        _index_verb(XTD_ASSETS_DIR "/WordNet-3.0/dict/index.verb")
      {
      }


      word::part_of_speech_t get_word_pos(const xtd::string& src){
        auto sAdj = std::async(std::launch::async, [&](){ return _index_adj.find(src); });
        auto sAdv = std::async(std::launch::async, [&](){ return _index_adv.find(src); });
        auto sNoun = std::async(std::launch::async, [&](){ return _index_noun.find(src); });
        auto sVerb = std::async(std::launch::async, [&](){ return _index_verb.find(src); });
        uint16_t iRet = word::part_of_speech_t::unknown_pos;
        if ("" != sAdj.get()) iRet |= word::part_of_speech_t::adjective;
        if ("" != sAdv.get()) iRet |= word::part_of_speech_t::adverb;
        if ("" != sNoun.get()) iRet |= word::part_of_speech_t::noun;
        if ("" != sVerb.get()) iRet |= word::part_of_speech_t::verb;
        return static_cast<word::part_of_speech_t>(iRet);
      }

      word::category_t get_word_category(const xtd::string& src, word::part_of_speech_t pos = word::part_of_speech_t::unknown_pos){
        uint64_t iRet = word::category_t::unknown_cat;
        index_file_record::pointer oIndex;
        data_file_record::pointer oData;
        verb_data_file_record::pointer oVerbData;
        if (word::part_of_speech_t::adjective == pos || word::part_of_speech_t::unknown_pos == pos) {
          oIndex = _index_adj.get_record(src);
          oData = _data_adj.get_record<data_file_record>(oIndex->synset_offset);
          if (oData){
            iRet |= oData->lex_filenum;
          }

        }
        return static_cast<word::category_t>(iRet);
      }

    private:

      enum SynsetType : char{
        noun = 'n',
        verb = 'v',
        adj = 'a',
        adj_satellite = 's',
        adverb = 'r',
      };



      struct index_file_record{
        enum SyntacticCategory : char{
          noun = 'n',
          verb = 'v',
          adj = 'a',
          adverb = 'r',
        };
        using pointer = std::shared_ptr<index_file_record>;
        std::vector<std::string> ptr_symbol;

        xtd::string lemma;
        SyntacticCategory pos;
        uint32_t synset_offset, synset_cnt, sense_cnt, tagsense_cnt;

        bool load(const xtd::string& sz){
          std::stringstream oSS;
          xtd::string spos, ssynset_offset, p_cnt, ssynset_cnt, ssense_cnt, stagsense_cnt;
          oSS.str(sz);
          oSS >> lemma >> spos >> ssynset_cnt >> p_cnt;
          pos = static_cast<SyntacticCategory>(spos[0]);
          synset_cnt = atoi(ssynset_cnt.c_str());
          for (auto t = atoi(p_cnt.c_str()); t; --t){
            xtd::string sTemp;
            oSS >> sTemp;
            ptr_symbol.push_back(sTemp);
          }
          oSS >> ssense_cnt >> stagsense_cnt >> ssynset_offset;
          sense_cnt = atoi(ssense_cnt.c_str());
          tagsense_cnt = atoi(stagsense_cnt.c_str());
          synset_offset = atoi(ssynset_offset.c_str());
          lemma.replace({ '_' }, ' ');
          return true;
        }
      };


      struct data_file_record{
        using pointer = std::shared_ptr<data_file_record>;
        struct word_index{
          using vector = std::vector<word_index>;
          xtd::string word, lex_id;
          word_index(const xtd::string& sword, const xtd::string& slexid) : word(sword), lex_id(slexid){}
        };

        struct ptr{
          enum SyntacticCategory : char{
            noun = 'n',
            verb = 'v',
            adj = 'a',
            adverb = 'r',
          };
          SyntacticCategory pos;
          xtd::string pointer_symbol, source_target;
          uint32_t synset_offset;
          using vector = std::vector<ptr>;
          ptr(const xtd::string& spointer_symbol, const xtd::string& ssynset_offset, SyntacticCategory spos, const xtd::string& ssource_target)
            : pos(spos), pointer_symbol(spointer_symbol), source_target(ssource_target), synset_offset(atoi(ssynset_offset.c_str())){}
        };

        bool load(const xtd::string& sz){
          auto oItems = sz.split({ ' ' }, true);
          xtd::string w_cnt, p_cnt;
          size_t x = 0;
          synset_offset = atoi(oItems[x++].c_str());
          lex_filenum = atoi(oItems[x++].c_str());
          ss_type = static_cast<SynsetType>(oItems[x++][0]);
          w_cnt = oItems[x++];
          for (auto t = atoi(w_cnt.c_str()); t; --t){
            auto p1 = oItems[x++];
            auto p2 = oItems[x++];
            p1.replace({ '_' }, ' ');
            p2.replace({ '_' }, ' ');
            words.emplace_back(p1, p1);
          }
          p_cnt = oItems[x++];
          for (auto t = atoi(p_cnt.c_str()); t; --t){
            auto p1 = oItems[x++];
            auto p2 = oItems[x++];
            auto p3 = static_cast<ptr::SyntacticCategory>(oItems[x++][0]);
            auto p4 = oItems[x++];
            pointers.emplace_back(p1, p2, p3, p4);
          }
          auto i = sz.find('|');
          if (i != xtd::string::npos) gloss = sz.substr(i);
          return true;
        }

        uint32_t synset_offset, lex_filenum;
        SynsetType ss_type;
        xtd::string gloss;
        word_index::vector words;
        ptr::vector pointers;
      };


      struct verb_data_file_record : data_file_record{
        using pointer = std::shared_ptr<verb_data_file_record>;
        struct generic_frame{
          using vector = std::vector<generic_frame>;
          xtd::string plus, f_num, w_num;
          generic_frame(const xtd::string& splus, const xtd::string& sf_num, const xtd::string& sw_num) : plus(splus), f_num(sf_num), w_num(sw_num){}
        };

        bool load(const xtd::string& sFile, size_t & i){
          size_t iEnd = i;
          for (; '\n' != sFile[iEnd] && iEnd < sFile.size(); ++iEnd);
          auto oItems = xtd::string(&sFile[i], &sFile[iEnd]).split({ ' ' }, true);
          xtd::string w_cnt, p_cnt;
          size_t x = 0;
          synset_offset = atoi(oItems[x++].c_str());
          lex_filenum = atoi(oItems[x++].c_str());
          ss_type = static_cast<SynsetType>(oItems[x++][0]);
          w_cnt = oItems[x++];
          for (auto t = atoi(w_cnt.c_str()); t; --t){
            auto p1 = oItems[x++];
            auto p2 = oItems[x++];
            p1.replace({ '_' }, ' ');
            p2.replace({ '_' }, ' ');
            words.emplace_back(p1, p2);
          }
          p_cnt = oItems[x++];
          for (auto t = atoi(p_cnt.c_str()); t; --t){
            auto p1 = oItems[x++];
            auto p2 = oItems[x++];
            auto p3 = static_cast<ptr::SyntacticCategory>(oItems[x++][0]);
            auto p4 = oItems[x++];
            pointers.emplace_back(p1, p2, p3, p4);
          }
          f_cnt = oItems[x++];
          for (auto t = atoi(f_cnt.c_str()); t; --t){
            auto p1 = oItems[x++];
            auto p2 = oItems[x++];
            auto p3 = oItems[x++];
            generic_frames.emplace_back(p1, p2, p3);
          }
          for (; '|' != sFile[i] && i < iEnd; ++i);
          gloss = xtd::string(&sFile[i], &sFile[iEnd]);
          i = ++iEnd;
          return true;
        }

        xtd::string f_cnt;
        generic_frame::vector generic_frames;

      };

      struct text_file : std::ifstream{
        text_file(const xtd::filesystem::path& oPath){
          using namespace std;
          ifstream::open(oPath, std::ios::in | std::ios::binary);
          size_t iLine = 1;
          std::string sLine;
          while (!ifstream::eof()){
            std::getline(*this, sLine);
            std::string sTemp = std::string("  ") + std::to_string(iLine);
            if (sLine.substr(0, 3) != sTemp) break;
            iLine++;
          }
          _Begin = ifstream::tellg();
          _Begin -= (1 + sLine.size());
          ifstream::seekg(0, std::ios::end);
          _End = ifstream::tellg();
          ifstream::seekg(_End - 200);
          std::string sTemp(200, 0);
          ifstream::read(&sTemp[0], 200);
          size_t i = 198;
          for (; i > 0 && '\n' != sTemp[i]; --i, --_End);
        }

        xtd::string line_at(size_t& pos){
          using namespace std;
          bool bol_found = false;
          static const size_t partition_size = 0x100;
          size_t iOffset = partition_size;
          std::string sRet, sTemp(partition_size, 0);
          while (!bol_found){
            ifstream::seekg(pos - iOffset);
            ifstream::read(&sTemp[0], sTemp.size());
            for (size_t i = sTemp.size() - 1; i >= 0; --i, --pos){
              if ('\n' == sTemp[i]){
                bol_found = true;
                break;
              }
            }
          }
          ifstream::seekg(pos);
          std::getline(*this, sRet);
          return sRet;
        }

        static int key_compare(const xtd::string& key, const xtd::string& line){

          for (size_t i = 0; i < key.size(); ++i){
            if (i > line.size()) return 1;
            auto iCmp = key[i] - line[i];
            if (0 != iCmp) return iCmp;
          }
          return 0;
        }

        xtd::string find(const xtd::string& key){
          size_t iStart = _Begin, iEnd = _End, iMid;
          while (iStart <= iEnd){
            iMid = (iStart + iEnd) / 2;
            auto sTemp = line_at(iMid);
            auto iCmp = key_compare(key, sTemp);
            if (0 == iCmp)
              return sTemp;
            else if (iCmp < 0)
              iEnd = iMid - 1;
            else iStart = iMid + 2 + sTemp.size();
          }
          return "";
        }

        size_t _Begin, _End;
      };

      struct index_file : text_file{
        index_file(const xtd::filesystem::path& oPath) : text_file(oPath){}

        index_file_record::pointer get_record(const xtd::string& key){
          auto sRecord = find(key);
          if (0 == sRecord.size()) return index_file_record::pointer(nullptr);
          auto oRet = std::make_shared<index_file_record>();
          if (!oRet->load(sRecord)) return index_file_record::pointer(nullptr);
          return oRet;
        }

      };

      struct data_file : text_file{
        data_file(const xtd::filesystem::path& oPath) : text_file(oPath){}

        template <typename _RecordT> typename _RecordT::pointer get_record(size_t offset){
          auto sLine = line_at(offset);
          if (0 == sLine.size()) return _RecordT::pointer(nullptr);
          auto oRet = std::make_shared<_RecordT>();
          if (!oRet->load(sLine)) return _RecordT::pointer(nullptr);
          return oRet;
        }

      };

      data_file _data_adj;
      data_file _data_adv;
      data_file _data_noun;
      data_file _data_verb;

      index_file _index_adj;
      index_file _index_adv;
      index_file _index_noun;
      index_file _index_sense;
      index_file _index_verb;

    };
  }
}