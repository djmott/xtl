/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace nlp{
    struct english{
      using pointer = std::shared_ptr<english>;

      english( const xtd::filesystem::path& oMobyPath, const xtd::filesystem::path& oWordnetPath) : _moby(oMobyPath), _wordnet(oWordnetPath){}

    private:
      moby::database _moby;
      wordnet::database _wordnet;
    };
  }
}
