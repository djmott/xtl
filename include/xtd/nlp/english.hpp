/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    struct english{

      using pointer = std::shared_ptr<english>;

      struct lemma{
        using pointer = std::shared_ptr<lemma>;
        using lemmata = std::vector<pointer>;
        using length_index = std::map<size_t, lemmata>;

        const std::string& value() const { return _value; }
        const lexical_category::vector& lexical_categories() const { return _lexical_categories; }
        lemma(const std::string& val) : _value(val){}

      private:
        std::string _value;
        lexical_category::vector _lexical_categories;
      };

      const lemma::lemmata& lemmata() const { return _lemmata; }


      english(const xtd::filesystem::path& oMobyPath, const xtd::filesystem::path& oWordnetPath) : _moby(oMobyPath), _wordnet(oWordnetPath){
        for (const auto & oRecord : _wordnet._data_adj.records){
          for (const auto & oWord : oRecord.second.words){
            std::cout << oWord.word << std::endl;
          }

        }
      }

    private:
      moby::database _moby;
      wordnet::database _wordnet;
      lemma::lemmata _lemmata;
    };
  }
}
