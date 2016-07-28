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
                private:
                    friend std::istream& operator >> (std::istream &in, record& r);
                };
                
                void load(const xtd::filesystem::path& oPath){
                    std::ifstream in(oPath);
                    std::string sLine;
                    while (!oPath.eof()){
                        record r;
                        in >> r;
                        records.insert(std::make_pair(r.word, r));
                    }
                }
                
                record::map records;
                
            private:
                friend std::istream& operator >> (std::istream &in, pos_file& r);
                
            };
            
            
            
            struct database{
                pos_file _pos_file;
                database(const xtd::filesystem::path& oPath){
                    _pos_file.load(oPath + "mpos/mobyposi.i");
                }
            };
        }
    }
}