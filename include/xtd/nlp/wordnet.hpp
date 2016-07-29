/** @file
c++ interface to wordnet databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    namespace wordnet{

      template <typename _FileT> inline void load_wn_file(const xtd::filesystem::path& oPath, _FileT& oDBFile){
        std::ifstream oFile(oPath);
        std::string sLine;
        //skip header
        while (!oFile.eof()){
          auto oPOS = oFile.tellg();
          std::getline(oFile, sLine);
          if (sLine.size() > 2 && ' ' == sLine[0] && ' ' == sLine[1]) continue;
          oFile.seekg(oPOS);
          break;
        }
        //load records
        while (!oFile.eof()){
          typename _FileT::record oRecord;
          oRecord.file_offset = oFile.tellg();
          oFile >> oRecord;
          oDBFile.records.insert(std::make_pair(oRecord.file_offset, oRecord));
        }
        auto x = oDBFile.records.size();
      }

/*      template <typename _FileT> inline void load_wn_file(const xtd::filesystem::path& oPath, _FileT& oDBFile){
        std::ifstream oFile(oPath);
        std::string sLine;
        //skip header
        while (!oFile.eof()){
          auto oPOS = oFile.tellg();
          std::getline(oFile, sLine);
          if (sLine.size() > 2 && ' ' == sLine[0] && ' ' == sLine[1]) continue;
          oFile.seekg(oPOS);
          break;
        }
        //load records
        while (!oFile.eof()){
          typename _FileT::record oRecord;
          oRecord.file_offset = oFile.tellg();
          oFile >> oRecord;
          oDBFile.records.insert(std::make_pair(oRecord.file_offset, oRecord));
        }
        auto x = oDBFile.records.size();
      }
*/
      struct index_file{

        struct record{
          using vector = std::vector<record>;
          using map = std::map<size_t, record>;
          size_t file_offset;
          std::string lemma, pos, synset_cnt, p_cnt, ptr_symbol, sense_cnt, tagsense_cnt, synset_offset;
        private:
          friend std::istream& operator>>(std::istream &in, record& p);
        };

        record::map records;

      };

/*      inline std::istream& operator >> (std::istream &in, index_file::record& r){
        r.file_offset = in.tellg();
        in >> r.lemma >> r.pos >> r.synset_cnt >> r.p_cnt >> r.ptr_symbol >> r.sense_cnt >> r.tagsense_cnt >> r.synset_offset;
        return in;
      }*/

      struct data_file{

        struct record{
          using vector = std::vector<record>;
          using map = std::map<size_t, record>;

          struct word_index{
            using vector = std::vector<word_index>;
            std::string word, lex_id;
          private:
            friend std::istream& operator >> (std::istream &in, word_index& p);
          };

          struct ptr{
            std::string pointer_symbol, synset_offset, pos, source_target;
            using vector = std::vector<ptr>;
          private:
            friend std::istream& operator >> (std::istream &in, ptr& p);
          };

          size_t file_offset;
          std::string synset_offset, lex_filenum, ss_type, w_cnt, p_cnt, gloss;
          word_index::vector words;
          ptr::vector pointers;
        private:
          friend std::istream& operator >> (std::istream &in, record& p);
        };

        record::map records;

      };

/*      inline std::istream& operator >> (std::istream &in, data_file::record& r){
        in >> r.synset_offset >> r.lex_filenum >> r.ss_type >> r.w_cnt;
        auto x = atoi(r.w_cnt.c_str());
        for (auto i = x; i; --i){
          data_file::record::word_index w;
          in >> w;
          r.words.push_back(w);
        }
        in >> r.p_cnt;
        x = atoi(r.p_cnt.c_str());
        for (auto i = x; i; --i){
          data_file::record::ptr pointer;
          in >> pointer;
          r.pointers.push_back(pointer);
        }

        std::getline(in, r.gloss);
        return in;
      }

      inline std::istream& operator >> (std::istream &in, data_file::record::word_index& w){
        in >> w.word >> w.lex_id;
        return in;
      }

      inline std::istream& operator >> (std::istream &in, data_file::record::ptr& p){
        in >> p.pointer_symbol >> p.synset_offset >> p.pos >> p.source_target;
        return in;
      }*/

      struct verb_data_file : data_file{


        struct record : data_file::record{

          struct generic_frame{
            using vector = std::vector<generic_frame>;
            std::string plus, f_num, w_num;
          private:
            friend std::istream& operator >> (std::istream &in, generic_frame& f);
          };
          
          std::string f_cnt;
          generic_frame::vector generic_frames;
        private:
          friend std::istream& operator >> (std::istream &in, record& r);
        };

        record::map records;


      };

/*      inline std::istream& operator >> (std::istream &in, verb_data_file::record& r){
        in >> r.synset_offset >> r.lex_filenum >> r.ss_type >> r.w_cnt;
        auto x = atoi(r.w_cnt.c_str());
        for (auto i = x; i; --i){
          data_file::record::word_index w;
          in >> w;
          r.words.push_back(w);
        }
        in >> r.p_cnt;
        x = atoi(r.p_cnt.c_str());
        for (auto i = x; i; --i){
          data_file::record::ptr pointer;
          in >> pointer;
          r.pointers.push_back(pointer);
        }
        in >> r.f_cnt;
        x = atoi(r.f_cnt.c_str());
        for (auto i = x; i; --i){
          verb_data_file::record::generic_frame frame;
          in >> frame;
          r.generic_frames.push_back(frame);
        }
        std::getline(in, r.gloss);
        return in;
      }*/

/*      inline std::istream& operator >> (std::istream &in, verb_data_file::record::generic_frame& f){
        in >> f.plus >> f.f_num >> f.w_num;
        return in;
      }*/

      struct database{

        data_file _data_adj;
        data_file _data_adv;
        data_file _data_noun;
        verb_data_file _data_verb;
        index_file _index_adj;
        index_file _index_adv;
        index_file _index_noun;
        index_file _index_verb;

        database(const xtd::filesystem::path& oPath) {
          auto t1 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "data.adj", _data_adj); });
          auto t2 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "data.adv", _data_adv); });
          auto t3 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "data.noun", _data_noun); });
          auto t4 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "data.verb", _data_verb); });
          auto t5 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "index.adj", _index_adj); });
          auto t6 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "index.adv", _index_adv); });
          auto t7 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "index.noun", _index_noun); });
          auto t8 = std::async(std::launch::async, [&](){ load_wn_file(oPath + "index.verb", _index_verb); });
          t1.get();
          t2.get();
          t3.get();
          t4.get();
          t5.get();
          t6.get();
          t7.get();
          t8.get();
        }
        database(const database&) = delete;

      };
    }
  }
}
