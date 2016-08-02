/** @file
Debugging
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

/// @def XTD_ASSERT debug assertion. The expression is expanded and checked for debug builds
#if (XTD_CONFIG_DEBUG & XTD_CONFIG)
  #define XTD_ASSERT(_exp, ...) assert(_exp)
#else
  #define XTD_ASSERT(...)
#endif

/// @def DUMP writes a debug dump record to the log printing the passed value
#define DUMP(x) xtd::Debug::Dump(x, #x, here());

namespace xtd{

#if (!DOXY_INVOKED)
  namespace _{
    template <typename _Ty> class DebugDump{
    public: 
      static void Dump(const _Ty& value, const char * name, const source_location& location){
        xtd::log::get().write(xtd::log::type::debug, location, "Dumping ", name, " type ", typeid(_Ty).name(), " at ", static_cast<const void*>(&value), " : ", value);
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
    template <typename _Ty>
    static void Dump(const _Ty& value, const char * name, const source_location& location){ _::DebugDump<const _Ty&>::Dump(value, name, location); }
  };



}
