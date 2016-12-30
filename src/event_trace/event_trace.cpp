/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/process.hpp>
#include <xtd/executable.hpp>
#include <xtd/log.hpp>
#include <xtd/windows/debug_help.hpp>
#include <xtd/windows/image_help.hpp>
#include <string>
#include <stack>
#include <dbghelp.h>
#include <unordered_map>

#pragma comment(lib, "dbghelp.lib")

namespace {

  struct SymbolInfo : SYMBOL_INFO {
    using pointer = std::shared_ptr<SymbolInfo>;
    using map = std::unordered_map<void*, pointer>;
    SymbolInfo() {
      memset(this, 0, sizeof(SymbolInfo));
      MaxNameLen = MAX_SYM_NAME;
      SizeOfStruct = sizeof(SYMBOL_INFO);
      _LineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE);
    }
    SymbolInfo(const SymbolInfo& src) {
      memcpy(this, &src, sizeof(SymbolInfo));
    }
    SymbolInfo& operator=(const SymbolInfo& src){
      if (this == &src) return *this;
      memcpy(this, &src, sizeof(SymbolInfo));
      return *this;
    }
    TCHAR _namebuff[MAX_SYM_NAME];
    IMAGEHLP_LINE _LineInfo;
  };

  thread_local std::stack<std::string> _ThreadStack;
  thread_local bool _InTrace = false;

  struct event_trace {


    ~event_trace(){}

    void enter(void * addr) {
      if (_InTrace) {
        return;
      }
      SymbolInfo::pointer oSym;
      auto oSymbol = _Symbols.find(addr);
      if (_Symbols.end() == oSymbol){
        oSym = std::make_shared<SymbolInfo>();
        SymFromAddr(xtd::process::this_process(), reinterpret_cast<DWORD64>(addr), nullptr, oSym.get());
        DWORD disp = 0;
        SymGetLineFromAddr(xtd::process::this_process(), reinterpret_cast<DWORD64>(addr), &disp, &oSym->_LineInfo);
        auto oRet = _Symbols.insert(std::make_pair(addr, oSym));
          oSymbol = oRet.first;
        }
      if (_Symbols.end() != oSymbol) {
        xtd::source_location oLoc(oSymbol->second->_LineInfo.FileName,oSymbol->second->_LineInfo.LineNumber);
        xtd::log::get().write(xtd::log::type::enter, oLoc, "Entering ", oSymbol->second->Name);

      }
      _InTrace = true;
      _InTrace = false;
    }
    void leave(void * addr) {
      if (_InTrace) {
        return;
      }
      auto oSymbol = _Symbols.find(addr);
      if (_Symbols.end() == oSymbol) {
        xtd::log::get().write(xtd::log::type::leave, xtd::source_location("", 0), "Leaving");
      }else {
        xtd::source_location oLoc(oSymbol->second->_LineInfo.FileName, oSymbol->second->_LineInfo.LineNumber);
        xtd::log::get().write(xtd::log::type::leave, oLoc, "Leaving ", oSymbol->second->Name);
      }

      _InTrace = true;
      _InTrace = false;
    }

    event_trace() {
      SymSetOptions(SYMOPT_DEBUG|SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);
      TODO("Fix me")
        if (
        !SymInitialize(xtd::process::this_process(), nullptr, TRUE) ||
//        !SymRegisterCallback(xtd::process::this_process(), &sym_callback, this) ||
        !SymLoadModuleEx(xtd::process::this_process(), nullptr, xtd::executable::this_executable().path().string().c_str(), nullptr, 0, 0, nullptr, 0)
         ) 
      {
        auto sErr = xtd::tstring::format("Unable to initialize symbol handler.\nGetLastError returned: ", static_cast<uint32_t>(GetLastError()));
        MessageBox(nullptr, sErr.c_str(), __("Error"), MB_OK | MB_ICONERROR);
      }
      
    }


    static BOOL CALLBACK sym_callback(HANDLE hProcess, ULONG ActionCode, PVOID CallbackData, PVOID UserContext){
      return FALSE;
    }

    SymbolInfo::map _Symbols;
  };

  event_trace _EventTrace;


}

extern "C" {
  void __xtd_EventEnter(void*pAddr) {
    _EventTrace.enter(pAddr);
  }

  void __xtd_EventLeave(void*addr) {
    _EventTrace.leave(addr);
  }
}
