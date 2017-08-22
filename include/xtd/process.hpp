/** @file
  represents an in-memory process
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#include <xtd/xtd.hpp>

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/types.h>
  #include <unistd.h>
  #include <dirent.h>
#endif

#if (XTD_OS_WINDOWS & XTD_OS)
#include <Psapi.h>
#endif

#include <regex>
#include <memory>
#include <map>

#include <xtd/dynamic_library.hpp>


namespace xtd {


#if (XTD_OS_UNIX & XTD_OS)
  class process {
  public:

    using pid_type = pid_t;
    using pointer = std::shared_ptr<process>;
    using map = std::map<pid_type, pointer>;

    static map system_processes() {
      map oRet;
      std::regex oRE("([0-9]+)");
      _dir::ptr oDir(opendir("/proc"));
      dirent * oEntry;
      while ((oEntry = readdir(oDir.get()))){
        if ((oEntry->d_type & DT_DIR) &&  std::regex_match(oEntry->d_name, oRE)) {
          auto oPID = static_cast<pid_type>(atoi(oEntry->d_name));
          oRet[oPID] = pointer(new process(oPID));
        }
      }
      return oRet;
    }

    dynamic_library::map libraries() {
      dynamic_library::map oRet;
      return oRet;
    }

    pid_type id() const { return _pid; }

    static process &this_process() {
      static process _this_process(getpid());
      return _this_process;
    }

  private:
    pid_type _pid;

    class _dir{
    public:
      using ptr = std::unique_ptr<DIR, _dir>;
      void operator()(DIR * d){ closedir(d); }
    };

    explicit process(pid_type hPid) : _pid(hPid) {

    }
  };
#elif (XTD_OS_WINDOWS & XTD_OS)
  class process {
  public:

    using pid_type = DWORD;
    using handle_type = HANDLE;
    using pointer = std::shared_ptr<process>;
    using map = std::map<pid_type, pointer>;


    static process& this_process() {
      static process _this_process(GetCurrentProcessId(), GetCurrentProcess());
      return _this_process;
    }

    static map system_processes() {
      map oRet;
      std::vector<DWORD> pids(10, 0);
      DWORD dwNeeded;
      forever {
        xtd::windows::exception::throw_if(EnumProcesses(&pids[0], static_cast<DWORD>(pids.size() * sizeof(DWORD)), &dwNeeded), [](BOOL b){ return FALSE==b; });
        if ((dwNeeded / sizeof(DWORD)) < pids.size()) {
          break;
        }
        pids.resize(pids.size() * 2);
      }
      pids.resize(dwNeeded / sizeof(DWORD));
      for (auto pid : pids) {
        oRet[pid] = pointer(new process(pid));
      }
      return oRet;
    }

    explicit process(pid_type hPid) : _pid(hPid), _hProcess(nullptr) {}
    process(pid_type hPid, handle_type hProc) : _pid(hPid), _hProcess(hProc) {}

    ~process() {
      if (_hProcess) {
        CloseHandle(_hProcess);
      }
    }

    pid_type id() const { return _pid; }

    dynamic_library::map libraries() {
      dynamic_library::map oRet;
      std::vector<HMODULE> modules(10, 0);
      DWORD dwNeeded;
      forever {
        xtd::windows::exception::throw_if(EnumProcessModules(*this, &modules[0], static_cast<DWORD>(modules.size() * sizeof(HMODULE)), &dwNeeded), [](BOOL b){return FALSE==b;});
        if ((dwNeeded / sizeof(HMODULE)) < modules.size()) {
          break;
        }
        modules.resize(modules.size() * 2);
      }
      modules.resize(dwNeeded / sizeof(HMODULE));
      for (dynamic_library::native_handle_type module : modules) {
        xtd::string sPath(MAX_PATH, 0);
        forever {
          dwNeeded = GetModuleFileNameExA(*this, module, &sPath[0], static_cast<DWORD>(sPath.size()));
          if (dwNeeded < sPath.size()) {
            break;
          }
          sPath.resize(dwNeeded);
        }
        oRet[sPath] = dynamic_library::pointer(new dynamic_library(module));
      }
      return oRet;
    }

    operator handle_type() {
      if (!_hProcess) {
        _hProcess = xtd::windows::exception::throw_if(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _pid), [](HANDLE h) { return NULL == h; });
      }
      return _hProcess;
    }

  private:
    pid_type _pid;
    handle_type _hProcess;

  };

#else
  #error "Unsupported system for xtd::process"
#endif


}
