/** @file
memory mapped files
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


namespace xtd{
  template <size_t _PageSize>
  class mapped_file{
    int _FileNum;
  public:
    static const size_t page_size = _PageSize;

    ~mapped_file(){ close(_FileNum); }

    explicit mapped_file(const filesystem::path& Path)
    : _FileNum(xtd::crt_exception::throw_if(open(Path.string().c_str(), O_CREAT), [](int i){ return -1==i; }))
    {}

    template <typename _Ty> class mapped_page{
    public:
      using value_type = _Ty;

      ~mapped_page(){ munmap(_Page, page_size); }

      mapped_page(const mapped_page&) = delete;
      mapped_page& operator=(const mapped_page&) = delete;

      mapped_page(mapped_page&& src) : _Page(nullptr){
        std::swap(_Page, src._Page);
      }
      mapped_page& operator=(mapped_page&& src){
        if (this != &src) {
          std::swap(_Page, src._Page);
        }
        return *this;
      }

      value_type* get() { return _Page; }
      const value_type* get() const { return _Page; }

      value_type* operator->() { return get();}
      const value_type* operator->() const { return get();}

      value_type& operator*() { XTD_ASSERT(get()); return *get();}
      const value_type& operator*() const { XTD_ASSERT(get()); return *get();}

      void flush(){
        xtd::crt_exception::throw_if(msync(_Page, page_size, MS_SYNC), [](int i){return 0 != i;});
      }

    protected:
      template <size_t> friend class mapped_file;
      explicit mapped_page(void * addr) : _Page(reinterpret_cast<value_type*>(addr)){}
      value_type* _Page;
    };

    template <typename _Ty> mapped_page<_Ty> get(size_t pageNum){
      return mapped_page<_Ty>(
        xtd::crt_exception::throw_if(
          mmap(nullptr, page_size, PROT_READ|PROT_WRITE, MAP_SHARED,  _FileNum, (pageNum * page_size)),
          [](void*addr){ return nullptr==addr; }
        )
      );
    }
  };
}

