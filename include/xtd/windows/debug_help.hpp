/** @file
object oriented access to the dbghelp library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>
#include <dbghelp.h>
#include <mutex>
#include <map>
#include <xtd/debug.hpp>
#include <xtd/exception.hpp>
#include <xtd/process.hpp>
#include <xtd/callback.hpp>

namespace xtd {
  namespace windows {
    namespace debug_help {

      struct initializer {
        static initializer& get() {
          static initializer _initializer;
          return _initializer;
        }


        ~initializer() { SymCleanup(xtd::process::this_process()); }

        callback<void(const TCHAR*)> on_debug_info;

      private:
        initializer() {
          xtd::windows::exception::throw_if(SymInitialize(xtd::process::this_process(), nullptr, TRUE), [](BOOL b) { return !b; });
          xtd::windows::exception::throw_if(SymRegisterCallback(xtd::process::this_process(), &sym_callback, this), [](BOOL b) { return !b; });

        }

        static BOOL CALLBACK sym_callback(HANDLE hProcess, ULONG ActionCode, PVOID CallbackData, PVOID UserContext) {
          auto pThis = reinterpret_cast<initializer*>(UserContext);
          if (CBA_DEBUG_INFO == ActionCode) {
            pThis->on_debug_info(reinterpret_cast<const TCHAR*>(CallbackData));
          }
          return TRUE;
        }
      };

      struct source_file : xtd::filesystem::path {
        using vector = std::vector<source_file>;

        source_file(const source_file& src) noexcept : xtd::filesystem::path(src), _base(src._base) {}
        source_file(source_file&& src) noexcept : xtd::filesystem::path(std::move(src)), _base(src._base) {}

        source_file& operator=(const source_file& src) noexcept {
          if (this == &src) return *this;
          xtd::filesystem::path::operator=(src);
          _base = src._base;
          return *this;
        }

        source_file& operator=(source_file&& src) noexcept {
          if (this == &src) return *this;
          _base = src._base;
          xtd::filesystem::path::operator=(std::move(src));
          return *this;
        }

      private:
        friend struct module;

        template <typename _Ty>
        source_file(_Ty sPath, ULONG64 base) : xtd::filesystem::path(sPath), _base(base) {}

        ULONG64 _base;
      };

      struct module : IMAGEHLP_MODULE {
        using pointer = std::shared_ptr<module>;
        using vector = std::vector<pointer>;

        static vector get_all() {
          vector oRet;
          xtd::windows::exception::throw_if(SymEnumerateModules(xtd::process::this_process(), &enum_modules_callback, &oRet), [](BOOL b) { return !b; });
          return oRet;
        }

        source_file::vector source_files() {
          source_file::vector oRet;
          xtd::windows::exception::throw_if(SymEnumSourceFiles(xtd::process::this_process(), this->BaseOfImage, nullptr, &enum_source_files_callback, &oRet), [](BOOL b) { return GetLastError() && !b; });
          return oRet;
        }


      private:

        module() {
          memset(this, 0, sizeof(IMAGEHLP_MODULE));
          SizeOfStruct = sizeof(IMAGEHLP_MODULE);
        }

        static BOOL CALLBACK enum_source_files_callback(PSOURCEFILE pSourceFile, PVOID UserContext) {
          auto pRet = reinterpret_cast<source_file::vector*>(UserContext);
          pRet->push_back(source_file(pSourceFile->FileName, pSourceFile->ModBase));
          return TRUE;
        }

        static BOOL CALLBACK enum_modules_callback(PCSTR ModuleName, ULONG BaseOfDll, PVOID UserContext) {
          auto pRet = reinterpret_cast<vector*>(UserContext);
          pRet->emplace_back(new module);
          xtd::windows::exception::throw_if(SymGetModuleInfo(xtd::process::this_process(), BaseOfDll, pRet->back().get()), [](BOOL b) { return !b; });;
          return TRUE;
        }
      };
    }
  }
}

/*
int main() {
  auto oModules = xtd::windows::debug_help::module::get_all();
  for (auto& oModule : oModules) {
    auto pDos = oModule->dos_header();
    auto pNT = oModule->nt_headers();
    auto pSection = oModule->section_header();
    auto oFiles = oModule->source_files();
    for (auto& oFile : oFiles) {
      DBG(oFile.string().c_str());
    }
  }
  return 0;
}
*/
