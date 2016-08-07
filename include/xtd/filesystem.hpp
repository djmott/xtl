/** @file
handle necessary filesystem and path functionality until C++17 is finalized
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <xtd/string.hpp>

namespace xtd{
  namespace filesystem{

    namespace _{
      template <typename, typename> struct path_adder;
    }
  }
}

#if (XTD_HAS_EXP_FILESYSTEM)
namespace xtd{
  namespace filesystem{

    struct path : std::experimental::filesystem::path{
      using _super_t = std::experimental::filesystem::path;
      template <typename ... _ArgTs> path(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}

      template <typename _Ty> inline path operator+(const _Ty& src) const{ return _::path_adder<value_type, const _Ty&>::add(*this, src); }

      path filename() const{ return path(_super_t::filename()); }

    };

    static inline bool remove(const path& src){ return std::experimental::filesystem::remove(src); }

    static inline path temp_directory_path(){ return path(std::experimental::filesystem::temp_directory_path()); }

  }
}
#elif (XTD_HAS_FILESYSTEM)
namespace xtd{
  namespace filesystem{
    using namespace std::filesystem;
  }
}
#else

namespace xtd{
  namespace filesystem{


    class path_base : public xtd::string{
    public:

      xtd::string& string() { return static_cast<xtd::string&>(*this);}
      const xtd::string& string() const { return static_cast<const xtd::string&>(*this);}
      template <typename ... _ArgTs> path_base(_ArgTs...oArgs) : xtd::string(std::forward<_ArgTs>(oArgs)...){}
    };
    class path : public path_base{
    public:
#if ((XTD_OS_MINGW|XTD_OS_WINDOWS) & XTD_OS)
      static constexpr value_type preferred_separator  = '\\';
      static constexpr value_type non_preferred_separator  = '/';
#else
      static constexpr value_type preferred_separator  = '/';
      static constexpr value_type non_preferred_separator  = '\\';
#endif

      template <typename ... _ArgTs> path(_ArgTs...oArgs) : path_base(std::forward<_ArgTs>(oArgs)...){}


      template <typename _RHST>
      path& operator /= (const _RHST& rhs){
        append(rhs);
        return *this;
      }

      template <typename _Ty> inline path operator+(const _Ty& src) const{
        path oRet(*this);
        oRet /= src;
        return oRet;
      }

      path& remove_filename(){
        if (preferred_separator == string().back()){
          return *this;
        }
        auto iEnd = _super_t::find_last_of(preferred_separator);
        if (_super_t::npos != iEnd){
          substr(0, find_last_of(preferred_separator));
        }
        return *this;
      }

      path& replace_filename(const path& newval){
        remove_filename();
        *this /= newval;
        return *this;
      }

      path filename() const {
        auto iEndSep = string().find_last_of(preferred_separator);
        return string().substr(1+iEndSep);
      }

      path& make_preferred(){
        for (auto & oCh : *this){
          if (oCh == non_preferred_separator){
            oCh = preferred_separator;
          }
        }
        return *this;
      }

      path& append(const path& rhs){
        if (non_preferred_separator==back()) back() = preferred_separator;
        if (preferred_separator != back() && preferred_separator != rhs.string().front()) push_back(preferred_separator);
        if (preferred_separator == back() && preferred_separator == rhs.string().front()) pop_back();
        _super_t::append(rhs.string());
        return *this;
      }

      path& operator+=(const path& rhs){ return append(rhs); }

    };



    inline path temp_directory_path(){
      return path(getenv("TEMP"));
    }

    inline bool remove(const path& oPath){
      return 0==::remove(oPath.string().c_str());
    }
  }
}

#endif


namespace xtd{
  namespace filesystem{
    namespace _{


      template <typename _ValueT, typename _Ch2, size_t _Dims> struct path_adder<_ValueT, const _Ch2(&)[_Dims]>{
        inline static path add(const path& dest, const _Ch2(&src)[_Dims]){
          auto str = xtd::xstring<_ValueT>::format(src);
          path oRet(dest);
          oRet.append(str);
          return oRet;
        }
      };
      template <typename _ValueT, typename _Ch2> struct path_adder<_ValueT, const _Ch2*>{
        inline static path add(const path& dest, const _Ch2* src){
          auto str = xtd::xstring<_ValueT>::format(src);
          path oRet(dest);
          oRet.append(str);
          return oRet;
        }
      };
      template <typename _ValueT> struct path_adder<_ValueT, const path&>{
        inline static path add(const path& dest, const path& src){
          path oRet(dest);
          oRet.append(src);
          return oRet;
        }
      };
      template <typename _ValueT> struct path_adder<_ValueT, const xtd::xstring<_ValueT>&>{
        inline static path add(const path& dest, const xtd::xstring<_ValueT>& src){
          return path_adder<_ValueT, const _ValueT*>::add(dest, src.c_str());
        }
      };
    }



  }
}
