/** @file
c++ interface to wordnet databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <map>
#include <future>
#include <sstream>
#include <fstream>

#include <xtd/filesystem.hpp>


namespace wordnet
{

  struct file {

  protected:
    template <typename _RecordT, typename _ContainerT> bool load(const xtd::filesystem::path& oPath, _ContainerT& oRecords) {
      std::ifstream in(oPath);
      in.exceptions( std::ios::badbit | std::ios::failbit );
      xtd::string sFile((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
      size_t i=0;
      for ( ; i<sFile.size() ; ++i) {
        if (' ' == sFile[i] && ' ' == sFile[1+i]) {
          for(; '\n' != sFile[i] && i < sFile.size(); ++i);
          continue;
        }
        break;
      }
      for (; i<sFile.size() ; ++i) {
        _RecordT oRecord;
        if (!oRecord.load(sFile, i)) {
          return false;
        }
        oRecords.insert(std::make_pair(oRecord.synset_offset, oRecord));
      }
      return true;
    }
  };

  struct index_file : file {

    struct record {
      using vector = std::vector<record>;
      using map = std::map<xtd::string, record>;
      std::vector<std::string> ptr_symbol;
      xtd::string lemma, pos, synset_cnt,  sense_cnt, tagsense_cnt, synset_offset;
      bool load(const xtd::string& sz, size_t & i) {
        std::stringstream oSS;
        auto x = sz.find('\n', i);
        xtd::string p_cnt, sLine(&sz[i], &sz[x]);
        oSS.str(sLine);
        oSS >> lemma >> pos >> synset_cnt >> p_cnt;
        for (int t = 0; t < atoi(p_cnt.c_str()); ++t){
          xtd::string sTemp;
          oSS >> sTemp;
          ptr_symbol.push_back(sTemp);
        }        
        oSS >> sense_cnt >> tagsense_cnt >> synset_offset;
        auto iSynLen = synset_offset.size();
        lemma.replace({ '_' }, ' ');
        i=x;
        return true;
      }
    };

    record::map records;

    bool load(const xtd::filesystem::path& oPath) {
      return file::load<record>(oPath, records);
    }

  };


  struct data_file : file {

    struct record {
      using vector = std::vector<record>;
      using map = std::map<xtd::string, record>;

      struct word_index {
        using vector = std::vector<word_index>;
        xtd::string word, lex_id;
        word_index(const xtd::string& sword, const xtd::string& slexid) : word(sword), lex_id(slexid) {}
      };

      struct ptr {
        xtd::string pointer_symbol, synset_offset, pos, source_target;
        using vector = std::vector<ptr>;
        ptr(const xtd::string& spointer_symbol, const xtd::string& ssynset_offset, const xtd::string& spos, const xtd::string& ssource_target)
            : pointer_symbol(spointer_symbol), synset_offset(ssynset_offset), pos(spos), source_target(ssource_target) {}
      };

      bool load(const xtd::string& sFile, size_t & i) {
        size_t iEnd = i;
        for (; '\n' != sFile[iEnd] && iEnd < sFile.size(); ++iEnd);
        auto oItems = xtd::string(&sFile[i], &sFile[iEnd]).split( {' '}, true);
        xtd::string w_cnt, p_cnt;
        size_t x=0;
        synset_offset = oItems[x++];
        lex_filenum = oItems[x++];
        ss_type = oItems[x++];
        w_cnt = oItems[x++];
        for (auto t = atoi(w_cnt.c_str()); t ; --t) {
          auto p1 = oItems[x++];
          auto p2 = oItems[x++];
          words.emplace_back(p1, p1);
        }
        p_cnt = oItems[x++];
        for (auto t = atoi(p_cnt.c_str()); t ; --t) {
          auto p1 = oItems[x++];
          auto p2 = oItems[x++];
          auto p3 = oItems[x++];
          auto p4 = oItems[x++];
          pointers.emplace_back(p1, p2, p3, p4);
        }
        for(; '|' != sFile[i] && i<iEnd ; ++i);
        gloss = xtd::string( &sFile[i], &sFile[iEnd] );
        i = ++iEnd;
        return true;
      }

      xtd::string synset_offset, lex_filenum, ss_type, gloss;
      word_index::vector words;
      ptr::vector pointers;

    };


    bool load(const xtd::filesystem::path& oPath) {
      return file::load<record>(oPath, records);
    }


    record::map records;

  };


  struct verb_data_file : data_file {


    struct record : data_file::record {

      struct generic_frame {
        using vector = std::vector<generic_frame>;
        xtd::string plus, f_num, w_num;
        generic_frame(const xtd::string& splus, const xtd::string& sf_num,const xtd::string& sw_num) : plus(splus), f_num(sf_num), w_num(sw_num) {}
      };

      bool load(const xtd::string& sFile, size_t & i) {
        size_t iEnd = i;
        for (; '\n' != sFile[iEnd] && iEnd < sFile.size(); ++iEnd);
        auto oItems = xtd::string(&sFile[i], &sFile[iEnd]).split( {' '}, true);
        xtd::string w_cnt, p_cnt;
        size_t x=0;
        synset_offset = oItems[x++];
        lex_filenum = oItems[x++];
        ss_type = oItems[x++];
        w_cnt = oItems[x++];
        for (auto t = atoi(w_cnt.c_str()); t ; --t) {
          auto p1 = oItems[x++];
          auto p2 = oItems[x++];
          p1.replace({ '_' }, ' ');
          p2.replace({ '_' }, ' ');
          words.emplace_back(p1, p2);
        }
        p_cnt = oItems[x++];
        for (auto t = atoi(p_cnt.c_str()); t ; --t) {
          auto p1 = oItems[x++];
          auto p2 = oItems[x++];
          auto p3 = oItems[x++];
          auto p4 = oItems[x++];
          pointers.emplace_back(p1, p2, p3, p4);
        }
        f_cnt = oItems[x++];
        for (auto t=atoi(f_cnt.c_str()) ; t ; --t) {
          auto p1 = oItems[x++];
          auto p2 = oItems[x++];
          auto p3 = oItems[x++];
          generic_frames.emplace_back(p1, p2, p3);
        }
        for(; '|' != sFile[i] && i<iEnd ; ++i);
        gloss = xtd::string( &sFile[i], &sFile[iEnd] );
        i = ++iEnd;
        return true;
      }

      xtd::string f_cnt;
      generic_frame::vector generic_frames;

    };

    bool load(const xtd::filesystem::path& oPath) {
      return file::load<record>(oPath, records);
    }

    record::map records;

  };


  struct database {

    data_file _data_adj;
    data_file _data_adv;
    data_file _data_noun;
    verb_data_file _data_verb;
    index_file _index_adj;
    index_file _index_adv;
    index_file _index_noun;
    index_file _index_verb;

    database() {
      auto make_path = [&](const char * sAddend){
        xtd::filesystem::path oRet(XTD_ASSETS_DIR "/WordNet-3.0/dict");
        oRet /= sAddend;
        return oRet;
      };
      _data_adj.load(make_path("data.adj"));
      _index_adj.load(make_path("index.adj"));
//       auto t1 = std::async(std::launch::async, [&]() {
//         return _data_adj.load(make_path("data.adj"));
//       });
//       auto t2 = std::async(std::launch::async, [&]() {
//         return _data_adv.load(make_path("data.adv"));
//       });
//       auto t3 = std::async(std::launch::async, [&]() {
//         return _data_noun.load(make_path("data.noun"));
//       });
//       auto t4 = std::async(std::launch::async, [&]() {
//         return _data_verb.load(make_path("data.verb"));
//       });
//       auto t5 = std::async(std::launch::async, [&]() {
//         return _index_adj.load(make_path("index.adj"));
//       });
//       auto t6 = std::async(std::launch::async, [&]() {
//         return _index_adv.load(make_path("index.adv"));
//       });
//       auto t7 = std::async(std::launch::async, [&]() {
//         return _index_noun.load(make_path("index.noun"));
//       });
//       auto t8 = std::async(std::launch::async, [&]() {
//         return _index_verb.load(make_path("index.verb"));
//       });
//       t1.get();
//       t2.get();
//       t3.get();
//       t4.get();
//       t5.get();
//       t6.get();
//       t7.get();
//       t8.get();
    }
    database(const database&) = delete;



  };
}
