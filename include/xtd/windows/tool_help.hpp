/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/
#pragma once

#include <xtd/xtd.hpp>

#include <windows.h>
#include <TlHelp32.h>

#include <memory>
#include <map>

#include <xtd/exception.hpp>

namespace xtd {
  namespace windows {
    namespace toolhelp {

      struct thread : THREADENTRY32 {
        using tid_type = DWORD;
        using ptr = std::shared_ptr<thread>;
        using map = std::map<tid_type, ptr>;
      private:
        friend struct process;
        explicit thread(const THREADENTRY32& src) : THREADENTRY32(src){}
      };

      struct dll : MODULEENTRY32 {
        using handle_type = HMODULE;
        using ptr = std::shared_ptr<dll>;
        using map = std::map<handle_type, ptr>;
      private:
        friend struct process;
        explicit dll(const MODULEENTRY32& src) : MODULEENTRY32(src){}
      };


      struct process : PROCESSENTRY32 {
        using pid_type = DWORD;
        using ptr = std::shared_ptr<process>;
        using map = std::map<pid_type, ptr>;

        static map running_processes() {
          map oRet;
          std::shared_ptr<void> hSnapshot(
            xtd::windows::exception::throw_if(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), [](HANDLE h) { return INVALID_HANDLE_VALUE == h; }),
            [](void * h) { ::CloseHandle(h); });
          PROCESSENTRY32 oPE;
          oPE.dwSize = sizeof(PROCESSENTRY32);
          for (BOOL bContinue = Process32First(hSnapshot.get(), &oPE); bContinue; bContinue = Process32Next(hSnapshot.get(), &oPE)) {
            oRet.insert(std::make_pair(oPE.th32ProcessID, ptr(new process(oPE))));
          }
          return oRet;
        }

        TODO("create dll iterators")
        dll::map dlls() const {
          dll::map oRet;
          std::shared_ptr<void> hSnapshot(
            xtd::windows::exception::throw_if(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, th32ProcessID), [](HANDLE h) { return INVALID_HANDLE_VALUE == h; }),
            [](void * h) { ::CloseHandle(h); });
          MODULEENTRY32 oME;
          oME.dwSize = sizeof(MODULEENTRY32);
          for (BOOL bContinue = Module32First(hSnapshot.get(), &oME); bContinue; bContinue = Module32Next(hSnapshot.get(), &oME)) {
            oRet.insert(std::make_pair(oME.hModule, dll::ptr(new dll(oME))));
          }
          return oRet;
        }

        TODO("create thread iterators")
          thread::map threads() const {
          thread::map oRet;
          std::shared_ptr<void> hSnapshot(
            xtd::windows::exception::throw_if(::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, th32ProcessID), [](HANDLE h) { return INVALID_HANDLE_VALUE == h; }),
            [](void * h) { ::CloseHandle(h); });
          THREADENTRY32 oTE;
          oTE.dwSize = sizeof(THREADENTRY32);
          for (BOOL bContinue = Thread32First(hSnapshot.get(), &oTE); bContinue; bContinue = Thread32Next(hSnapshot.get(), &oTE)) {
            oRet.insert(std::make_pair(oTE.th32ThreadID, thread::ptr(new thread(oTE))));
          }
          return oRet;
        }

      private:
        explicit process(const PROCESSENTRY32& src) : PROCESSENTRY32(src){}
      };

    }
  }
}