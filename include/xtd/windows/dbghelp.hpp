/** @file
object oriented access to the dbghelp library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
  namespace windows{

    class debug_symbol{
      friend class debug_help;
      std::unique_ptr<SYMBOL_INFO> _symbol_info;
      debug_symbol(){}
    public:
      ~debug_symbol(){}
    };


    class debug_help{
    public:
      using pointer = std::shared_ptr<debug_help>;

      debug_help(){}
      ~debug_help(){}

      static pointer make(){ return pointer(new debug_help); }

    private:

      class initializer{
      public:

        using action_strings_map = std::map<ULONG, const char *>;

        initializer(){
          SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
          xtd::windows::exception::throw_if(SymInitialize(GetCurrentProcess(), nullptr, TRUE), [](BOOL b){return FALSE == b; });
          xtd::windows::exception::throw_if(SymRegisterCallback64(GetCurrentProcess(), SymCallback, 0), [](BOOL b){return FALSE == b; });
        }
        ~initializer(){
          SymCleanup(GetCurrentProcess());
        }

        static action_strings_map& action_strings(){
          static action_strings_map _action_strings = {
            { CBA_DEBUG_INFO, "CBA_DEBUG_INFO" },
            { CBA_DEFERRED_SYMBOL_LOAD_CANCEL , "CBA_DEFERRED_SYMBOL_LOAD_CANCEL" },
            { CBA_DEFERRED_SYMBOL_LOAD_COMPLETE, "CBA_DEFERRED_SYMBOL_LOAD_COMPLETE" },
            { CBA_DEFERRED_SYMBOL_LOAD_FAILURE , "CBA_DEFERRED_SYMBOL_LOAD_FAILURE" },
            { CBA_DEFERRED_SYMBOL_LOAD_PARTIAL , "CBA_DEFERRED_SYMBOL_LOAD_PARTIAL" },
            { CBA_DEFERRED_SYMBOL_LOAD_START , "CBA_DEFERRED_SYMBOL_LOAD_START" },
            { CBA_DUPLICATE_SYMBOL , "CBA_DUPLICATE_SYMBOL" },
            { CBA_EVENT , "CBA_EVENT" },
            { CBA_READ_MEMORY  , "CBA_READ_MEMORY" },
            { CBA_SET_OPTIONS , "CBA_SET_OPTIONS" },
            { CBA_SRCSRV_EVENT , "CBA_SRCSRV_EVENT" },
            { CBA_SRCSRV_INFO  , "CBA_SRCSRV_INFO" },
            { CBA_SYMBOLS_UNLOADED  , "CBA_SYMBOLS_UNLOADED" },
          };
          return _action_strings;
        }

        static BOOL CALLBACK SymCallback(HANDLE, ULONG ActionCode, ULONG64 CallbackData, ULONG64){
          switch (ActionCode){
            case CBA_DEBUG_INFO:{
              DBG(reinterpret_cast<const char*>(CallbackData));
              break;
            }
            case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:
            case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:
            case CBA_DEFERRED_SYMBOL_LOAD_PARTIAL:
            case CBA_DEFERRED_SYMBOL_LOAD_START:
            case CBA_DUPLICATE_SYMBOL:
            case CBA_EVENT:
            case CBA_READ_MEMORY:
            case CBA_SET_OPTIONS:
            case CBA_SRCSRV_EVENT:
            case CBA_SRCSRV_INFO:
            case CBA_DEFERRED_SYMBOL_LOAD_CANCEL:
            case CBA_SYMBOLS_UNLOADED:
              break;
          }
          return FALSE;
        }

      };

      static std::mutex& library_lock(){
        static initializer _initializer;
        static std::mutex _library_lock;
        return _library_lock;
      }
    };
  }
}
