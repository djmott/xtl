/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#define XTD_VERBOSE_BUILD 1
#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  NOTE("OS : UNIX")
#elif (XTD_OS_WINDOWS & XTD_OS)
  NOTE("OS : Windows")
#endif

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  NOTE("Compiler : MSVC")
  NOTE("Binary must be compiled with /Gh and /GH to generate event trace")
#elif (XTD_COMPILER_GCC & XTD_COMPILER)
  NOTE("Compiler : GCC");
  NOTE("Binary must be compiled with -finstrument-functions to generate event trace")
#elif (XTD_COMPILER_CLANG & XTD_COMPILER)
  NOTE("Compiler : CLANG");
  NOTE("Binary must be compiled with -finstrument-functions to generate event trace")
#elif (XTD_COMPILER_MINGW & XTD_COMPILER)
  NOTE("Compiler : MinGW")
  NOTE("Binary must be compiled with -finstrument-functions to generate event trace")
#endif

#define DUMP_DEFINE2(prefix, x) PRAGMA_(message( prefix QUOTE(x)))
#define DUMP_DEFINE(x) DUMP_DEFINE2("#define " # x " ", x)

#if (XTD_OS_WINDOWS & XTD_OS)
DUMP_DEFINE(_WIN32_WINNT);
#endif

DUMP_DEFINE(XTD_CHARSET);
DUMP_DEFINE(XTD_CONFIG);
DUMP_DEFINE(DOXY_INVOKED);
DUMP_DEFINE(XTD_HAS_UUID);
DUMP_DEFINE(XTD_COLOR_MESSAGES);
DUMP_DEFINE(XTD_HAS_CODECVT);
DUMP_DEFINE(XTD_HAS_EXP_CODECVT);
DUMP_DEFINE(XTD_HAS_ICONV);
DUMP_DEFINE(XTD_HAS_FILESYSTEM);
DUMP_DEFINE(XTD_HAS_EXP_FILESYSTEM);
DUMP_DEFINE(XTD_USE_DBGHELP);
DUMP_DEFINE(XTD_LOG_TARGET_SYSLOG);
DUMP_DEFINE(XTD_LOG_TARGET_WINDBG);
DUMP_DEFINE(XTD_LOG_TARGET_CSV);
DUMP_DEFINE(XTD_LOG_TARGET_COUT);
DUMP_DEFINE(XTD_LOG_TARGET_XML);
