/** @file
    host, target and build configurations and settings
    Various components are purpose built for specific targets
    xtd.hpp is a descriptive configuration header that is generated by CMake from the xtd.hpp.in template.
    Various components include xtd.hpp for configuration info
    @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstddef>
#include <cstring>

#define XTD_GIT_IDENT "$Id: f267349cb36e69c940125fd88a8c2611c9ae4f94 $"

#if !defined(ON)
  #define ON    1
#endif

#if !defined(OFF)
  #define OFF   0
#endif

#if !defined(YES)
  #define YES   1
#endif

#if !defined(NO)
  #define NO    0
#endif

#if !defined(DOXY_INVOKED)
  #define DOXY_INVOKED 0
#endif

/** @name XTD_CPU
RESERVED FOR FUTURE USE
@{*/
#define XTD_CPU_ARM32     1
#define XTD_CPU_ARM64     2
#define XTD_CPU_PPC32     4
#define XTD_CPU_PPC64     8
#define XTD_CPU_X32       16
#define XTD_CPU_X64       32
///@}

/** @name XTD_OS
The build/target/host OS selected by cmake configure
@{*/
#define XTD_OS_IOS        0x04
#define XTD_OS_UNIX      0x08
#define XTD_OS_WINDOWS    0x10
#if !defined(XTD_OS)
    #define XTD_OS XTD_OS_WINDOWS
#endif
///@}


/** @name XTD_COMPILER
@{*/
#define XTD_COMPILER_MSVC 1
#define XTD_COMPILER_GCC 2
#define XTD_COMPILER_CLANG 4
#define XTD_COMPILER_INTEL 8
#define XTD_COMPILER_DMC 16
#define XTD_COMPILER_WATCOM 32
#define XTD_COMPILER_XCODE 64
#define XTD_COMPILER_MINGW 128
#if !defined(XTD_COMPILER)
    #define XTD_COMPILER XTD_COMPILER_GCC
#endif


#if (XTD_COMPILER_GCC & XTD_COMPILER) && (XTD_OS_UNIX & XTD_OS)
#define fopen_s( retfile , spath, smode ) *retfile = fopen(spath, smode)

auto _dupenv_s = [](char **buffer, std::size_t *numberOfElements, const char *varname) -> int{
  auto sVal = getenv(varname);
  return 0;
};

#endif

///@}


/** @name XTD_CHARSET
    The default character set used in resulting binaries
    @{*/
#define XTD_CHARSET_MBCS      1
#define XTD_CHARSET_UNICODE   2
#if !defined(XTD_CHARSET)
    #define XTD_CHARSET XTD_CHARSET_MBCS
#endif


#if (XTD_CHARSET_UNICODE & XTD_CHARSET)
  using tchar = wchar_t;
  #define __(x) L ## x
#else
  using tchar = char;
  #define __(x)  x
#endif
    ///@}




/** @name XTD_CONFIG
    The build configuration set by cmake configure
    {@*/
#define XTD_CONFIG_DEBUG      1
#define XTD_CONFIG_RELEASE    2
#if !defined(XTD_CONFIG)
    #define XTD_CONFIG XTD_CONFIG_DEBUG
#endif

#if (XTD_CONFIG_RELEASE & XTD_CONFIG)
  #define D_(...)
  #define R_(...) __VA_ARGS__
#else
  #define D_(...) __VA_ARGS__
  #define R_(...)
#endif
    ///@}


#if !defined(XTD_LOG_TARGET_SYSLOG)
    #define XTD_LOG_TARGET_SYSLOG OFF
#endif

#if !defined(XTD_LOG_TARGET_WINDBG)
    #define XTD_LOG_TARGET_WINDBG OFF
#endif

#if !defined(XTD_LOG_TARGET_CSV)
    #define XTD_LOG_TARGET_CSV OFF
#endif

#if !defined(XTD_LOG_TARGET_COUT)
    #define XTD_LOG_TARGET_COUT OFF
#endif

#if !defined(XTD_LOG_TARGET_XML)
    #define XTD_LOG_TARGET_XML OFF
#endif

#if !defined(XTD_HAS_LIBUUID)
    #define XTD_HAS_LIBUUID NO
#endif

#if !defined(XTD_COLOR_MESSAGES)
    #define XTD_COLOR_MESSAGES NO
#endif

#if !defined(XTD_HAS_CODECVT)
    #define XTD_HAS_CODECVT NO
#endif

#if !defined(XTD_HAS_EXP_CODECVT)
    #define XTD_HAS_EXP_CODECVT NO
#endif

#if !defined(XTD_HAS_ICONV)
    #define XTD_HAS_ICONV NO
#endif

#if !defined(XTD_HAS_FILESYSTEM)
    #define XTD_HAS_FILESYSTEM NO
#endif

#if !defined(XTD_HAS_EXP_FILESYSTEM)
    #define XTD_HAS_EXP_FILESYSTEM NO
#endif

#if !defined(XTD_USE_DBGHELP)
    #define XTD_USE_DBGHELP NO
#endif




/** @name Misc macros
    @{*/
#if !defined(NOMINMAX)
  #define NOMINMAX
#endif

#define W_(x) L # x

#define QUOTE2_( x ) # x
#define QUOTE( x ) QUOTE2_( x )

#define CONCAT2_(x, y) x ## y
#define CONCAT(x, y) CONCAT2_(x,y)

#define DEFINE(x) x

#if defined(__COUNTER__)
  #define UNIQUE_IDENTIFIER(_prefix) CONCAT(_prefix, __COUNTER__)
#else
  #define UNIQUE_IDENTIFIER(_prefix) CONCAT(_prefix, __LINE__)
#endif


#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #define PRAGMA_(x) __pragma(x)
  #define FORCEINLINE __forceinline
  #define NOVTABLE __declspec(novtable)
  #define NORETURN __declspec(noreturn)
  #define MULTIPLE __declspec(selectany)
  #define ALIGN(val) __declspec(align(val))
  #define XTD_EXPORT __declspec(dllexport)
#elif (XTD_COMPILER_GCC & XTD_COMPILER)
  #define PRAGMA_(x) _Pragma( #x )
  #define FORCEINLINE __attribute__((always_inline))
  #define NOVTABLE
  #define NORETURN __attribute__((noreturn))
  #define MULTIPLE __attribute__ ((weak))
  #define ALIGN(val) __attribute__ ((aligned (val)))
  #define XTD_EXPORT __attribute__ ((visibility ("default")))
#endif

#define PACK_PUSH(n) PRAGMA_(pack(push, n))
#define PACK_POP() PRAGMA_(pack(pop))

#if !defined(XTD_VERBOSE_BUILD)
    #define XTD_VERBOSE_BUILD OFF
#endif

#if (!XTD_VERBOSE_BUILD)
    #undef TODO
    #undef NOTE
    #define TODO(...)
    #define NOTE(...)
#elif XTD_COLOR_MESSAGES
  #define TODO( x ) PRAGMA_(message ( __FILE__ ":" QUOTE(__LINE__) " : \e[31mTODO : " x "\e[0m" ))
  #define NOTE( x ) PRAGMA_(message ( __FILE__ ":" QUOTE(__LINE__) " : \e[32mNOTE : " x  "\e[0m"))
#else
  #define TODO( x ) PRAGMA_(message ( __FILE__ ":" QUOTE(__LINE__) " : TODO : " x ))
  #define NOTE( x ) PRAGMA_(message ( __FILE__ ":" QUOTE(__LINE__) " : NOTE : " x ))
#endif
    ///@}

/// @defgroup Annotation Annotation and documentation to clarify intent of code

/*! \namespace xtd
    The main namespace of the XTL library
*/
namespace xtd{
    /// @internal
    /// @namespace xtd::_
    /// private namespace not for external consumption
    namespace _{}
    /// @endinternal

    /** @defgroup Concurrent Concurrent algorithms library
    @{
    @namespace xtd::concurrent
    Concurrent algorithms library
    */
    namespace concurrent{}
    /// @}


    /** @defgroup Parsing Text parsing and AST generation library

    Grammars are defined using classes and macros in the xtd::parse namespace.  The fully constructed AST from the start
    rule is created and returned from xtd::parse<>::parse() if the parse succeeds.

    @{
    @namespace xtd::parse
    Grammar and AST declarations
    @example example_parse1.cpp
    @example example_parse2.cpp
    */
    namespace parse{}
    /// @}


    /** @defgroup Sockets General purpose socket communication

    @{
    @namespace xtd::socket
    General purpose socket communication
    */
    namespace socket{}
    /// @}

}

#define forever for(;;)

//System
#if (XTD_OS_UNIX & XTD_OS)
  #if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
  #endif
#endif

#if (XTD_OS_WINDOWS & XTD_OS)
  #if !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x600
  #endif

  #include <winsock2.h>
  #include <windows.h>
  #include <Shlobj.h>
#endif

#if !defined(interface)
  #define interface struct
#endif
