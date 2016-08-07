/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    class document{
    public:
      using pointer = std::shared_ptr<document>;

      document(english::pointer oEnglish, const std::string sRaw) : _english(oEnglish), _raw(sRaw) {}

    private:
      english::pointer _english;
      xtd::string _raw;
    };
  }
}
