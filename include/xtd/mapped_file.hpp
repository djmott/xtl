/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#if 0
namespace xtd{
  class mapped_file{
    int _FileNum;
  public:
    mapped_file(const path& Path)
    : _FileNum(xtd::crt_exception::throw_if(open(Path.string().c_str(), O_CREAT), [](int i){ return -1==i; }))
    {}

    template <typename _Ty> class mapped_page{
    public:
    };

    template <typename _Ty> mapped_page<_Ty> read(size_t offset){}
  };
}
#endif
