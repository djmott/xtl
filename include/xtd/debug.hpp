/** @file
 data conversion utilities
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#define XTD_ASSERT(_exp, ...) assert(_exp)

#define DUMP(x) xtd::Debug::Dump(x, #x, here());

namespace xtd{

  namespace _{
    template <typename _Ty> class DebugDump{
    public: 
      static void Dump(_Ty value, const char * name, const source_location& location){
        xtd::log::get().write(xtd::log::type::debug, location, "Dumping ", name, " type ", typeid(_Ty).name(), " at ", static_cast<void*>(&value), " : ", value);
      }
    };
  }

  class Debug{
  public:
    template <typename _Ty>
    static void Dump(const _Ty& value, const char * name, const source_location& location){ _::DebugDump<_Ty>::Dump(value, name, location); }
  };




  namespace _{

  }

}