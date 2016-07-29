/** @file
c++ interface to moby databases
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace nlp{
        namespace moby{
            
            struct pos_file{
                struct record{
                    using vector = std::vector<record>;
                    using map = std::map<std::string, record>;
                    std::string word;
                    std::vector<char> pos;
                };
                

                record::map records;
                
            private:
                friend std::istream& operator >> (std::istream &in, pos_file& r);
                
            };

          inline std::istream& operator >> (std::istream &in, pos_file& f){
            xtd::string sFile((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            auto oLines = sFile.split({ '\r' }, true);
            for (auto & oLine : oLines){
              auto oElements = oLine.split({ (char)0xd7 }, true);
              if (oElements.size() > 1){
                pos_file::record r;
                r.word = oElements[0];
                auto oItem = f.records.find(r.word);
                if (oItem == f.records.end()){
                  for (char ch : oElements[1]){
                    r.pos.push_back(ch);
                  }
                  f.records.insert(std::make_pair(oElements[0], r));
                } else{
                  for (char ch : oElements[1]){
                    oItem->second.pos.push_back(ch);
                  }
                }
              }
            }
            return in;
          }

          struct database{
              pos_file _pos_file;
              database(const xtd::filesystem::path& oPath){
                std::ifstream in(oPath + "mpos/mobyposi.i");
                in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                in >> _pos_file;
              }
          };
        }
    }
}
