/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>


#if (XTD_COMPILER & XTD_COMPILER_GCC)
NOTE("Compiler : GCC");
#endif
#if (XTD_COMPILER & XTD_COMPILER_MSVC)
NOTE("Compiler : MSVC");
#endif


#if (XTD_OS & XTD_OS_ANDROID)
NOTE("OS : Android");
#endif
#if (XTD_OS & XTD_OS_BAREMETAL)
NOTE("OS : Bare Metal");
#endif
#if (XTD_OS & XTD_OS_IOS)
NOTE("OS : IOS");
#endif
#if (XTD_OS & XTD_OS_LINUX)
NOTE("OS : Linux");
#endif
#if (XTD_OS & XTD_OS_WINDOWS)
NOTE("OS : Windows");
#endif
#if (XTD_OS & XTD_OS_CYGWIN)
NOTE("OS : Cygwin");
#endif
#if (XTD_OS & XTD_OS_MINGW)
NOTE("OS : MinGW");
#endif

#if (XTD_CHARSET & XTD_CHARSET_MBCS)
NOTE("Character Set : Multibyte");
#else
NOTE("Character Set : Unicode");
#endif

#if (XTD_CONFIG & XTD_CONFIG_RELEASE)
NOTE("Configuration : Release");
#else
NOTE("Configuration : Debug");
#endif
