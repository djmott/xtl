/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  class mapped_file{
    int _FileNum;
  public:
    mapped_file(const path& Path)
    : _FileNum(xtd::crt_exception::throw_if(open(Path. ), [](int i){ return -1==i; }))
    {}

    template <typename _Ty> class mapped_page{
    public:
    };

    template <typename _Ty> mapped_page<_Ty> read(size_t offset){}
  };
}