/** @file
Debugging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#if(XTD_OS_UNIX & XTD_OS)
  #include <iostream>
#endif

#include <xtd/log.hpp>
#include <xtd/source_location.hpp>

/// @def XTD_ASSERT debug assertion. The expression is expanded and checked for debug builds
#if (XTD_CONFIG_RELEASE & XTD_CONFIG)
  #define XTD_ASSERT(...)
#elif ((XTD_COMPILER_CLANG | XTD_COMPILER_GCC | XTD_COMPILER_MINGW) & XTD_COMPILER)
  #define XTD_ASSERT( expression , ... ) while ( !xtd::Debug::Assert( here(), !(!(expression)), #expression , ##__VA_ARGS__ ) ){}
#else
  #define XTD_ASSERT( expression , ... ) while ( !xtd::Debug::Assert( here(), !(!(expression)), #expression , __VA_ARGS__ ) ){}
#endif

/// @def DUMP writes a debug dump record to the log printing the passed value
#define DUMP(x) xtd::Debug::Dump(x, #x, here());

namespace xtd{

#if (!DOXY_INVOKED)
  namespace _{
    template <typename _ty> class DebugDump{
    public: 
      static void Dump(const _ty& value, const char * name, const source_location& location){
        xtd::log::get().write(xtd::log::type::debug, location, "Dumping ", name, " type ", typeid(_ty).name(), " at ", static_cast<const void*>(&value), " : ", value);
      }
    };
  }
#endif

  /** Manages debug info about processes
  */
  class Debug{
  public:
    /** Writes a debug dump record to the log.
     * The method is rarely called directly, instead use the DUMP maco which produces the correct name and location members
     * @param value the value to dump
     * @param name name of the item to dump
     * @param location location of the call site
     */
    template <typename _ty>
    static void Dump(const _ty& value, const char * name, const source_location& location){ _::DebugDump<const _ty&>::Dump(value, name, location); }


  #if (XTD_OS_WINDOWS & XTD_OS)
    template <typename ... _arg_ts>
    static bool Assert(const source_location& location, bool test, const char * test_str, _arg_ts&&...oArgs){
      if (test) return true;
      auto sMsg = xtd::cstring::format(location.file(), " line ", location.line(), "\nThe expression evaluated to false.\n\n\t", test_str, "\n", std::forward<_arg_ts>(oArgs)...);
      auto iRet = MessageBoxA(nullptr, sMsg.c_str(), "Debug Assertion Failed", MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_SYSTEMMODAL);
      if (IDABORT == iRet) std::abort();
      return (IDIGNORE == iRet);
    }
  #else
    template <typename ... _arg_ts>
    static bool Assert(const source_location& location, bool test, const char * test_str, _arg_ts&&...oArgs){
      if (test) return true;
      auto sMsg = xtd::cstring::Format(location.file(), " line ", location.line(), "\nThe expression evaluated to false.\n\n\t", test_str, "\n", std::forward<_arg_ts>(oArgs)...);
      std::cout << sMsg.c_str();
      abort();
    }
  #endif
  };



}
