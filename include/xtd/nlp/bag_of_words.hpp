/** @file
histogram of words in a document
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd{
  namespace nlp{
    class bag_of_words : std::map<std::string, uint32_t>{
    public:
      explicit bag_of_words(const xtd::string& src){
        auto sWords = src.split();
      }
    };
  }
}
