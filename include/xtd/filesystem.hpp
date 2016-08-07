/** @file
handle necessary filesystem and path functionality until C++17 is finalized
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


#pragma once

#if (XTD_HAS_EXP_FILESYSTEM)
namespace xtd{
  namespace filesystem{
    using namespace std::experimental::filesystem;
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


    struct path_base : xtd::string{

      xtd::string& string() { return static_cast<xtd::string&>(*this);}
      const xtd::string& string() const { return static_cast<const xtd::string&>(*this);}
      template <typename ... _ArgTs> path_base(_ArgTs...oArgs) : xtd::string(std::forward<_ArgTs>(oArgs)...){}
    };
    struct path : path_base{

#if ((XTD_OS_MINGW|XTD_OS_WINDOWS) & XTD_OS)
      static constexpr value_type preferred_separator  = '\\';
#else
      static constexpr value_type preferred_separator  = '/';
#endif

      template <typename ... _ArgTs> path(_ArgTs...oArgs) : path_base(std::forward<_ArgTs>(oArgs)...){}

      path& replace_filename(const path& newval){
        auto iLastSeperator = string().find_last_of(seperator);

        if (0==string().size() || xtd::string::npos == iLastSeperator) {
          string() = newval.string();
          return *this;
        }
        if (seperator == string().back()){
          append(newval);
          return *this;
        }

      }

      path filename() const {
        
      }

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
    inline path operator+(const path& lhs, const path& rhs){
      path oRet(lhs);
      oRet.append(rhs);
      return oRet;
    }
    inline path operator+(const path& lhs, const char * rhs){
      path oRet(lhs);
      oRet.append(rhs);
      return oRet;
    }
  }
}
