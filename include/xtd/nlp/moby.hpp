/** @file
c++ interface to moby databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    namespace moby{

      class pos_file{
      public:
        class record{
        public:
          using vector = std::vector<record>;
          using map = std::map<std::string, record>;
          std::string word;
          std::vector<char> pos;
          record(const std::string& sWord) : word(sWord){}
        };

        pos_file(const xtd::filesystem::path& oPath){
          std::ifstream in(oPath.string());
          in.exceptions( std::ios::badbit | std::ios::failbit );
          xtd::string sFile((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
          for (auto sBegin = sFile.begin(); sBegin < sFile.end() ; ++sBegin){
            for(;('\r' == *sBegin || '\n' == *sBegin) && sBegin < sFile.end() ; ++sBegin);
            auto sEnd = sBegin;
            for(;(char)0xd7 != *sEnd && sEnd < sFile.end(); ++sEnd);
            record r(std::string(sBegin, sEnd));
            records.insert(std::make_pair(r.word, r));
            for(++sEnd;'\r' != *sEnd && '\n' != *sEnd && sEnd < sFile.end();++sEnd){
              r.pos.push_back(*sEnd);
            }
            sBegin = sEnd;
          }

        }

        record::map records;


      };



      class database{
      public:
        pos_file _pos_file;
        database(const xtd::filesystem::path& oPath) : _pos_file(oPath + "mpos/mobyposi.i"){}
      };
    }
  }
}
