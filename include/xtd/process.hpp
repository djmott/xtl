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
  #include <windows.h>
  #include <Psapi.h>
#endif

#include <regex>
#include <map>

#include <xtd/memory.hpp>
#include <xtd/dynamic_library.hpp>
#include <xtd/filesystem.hpp>

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
        xtd::windows::exception::throw_if(::EnumProcesses(&pids[0], static_cast<DWORD>(pids.size() * sizeof(DWORD)), &dwNeeded), [](BOOL b){ return FALSE==b; });
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

    ~process() {
      if (_hProcess) CloseHandle(_hProcess);
      if (_hMainThread) CloseHandle(_hMainThread);
    }

    process(pid_type hPid, handle_type hProc, handle_type hMainThread) : _pid(hPid), _hProcess(hProc), _hMainThread(hMainThread) {}
    process(pid_type hPid, handle_type hProc) : process(hPid,hProc,nullptr) {}
    explicit process(pid_type hPid) : process(hPid, nullptr, nullptr) {}

    process(const process& src) : _pid(src._pid), _hProcess(nullptr), _hMainThread(nullptr) {
      if (src._hProcess) {
        xtd::windows::exception::throw_if(DuplicateHandle(GetCurrentProcess(), src._hProcess, GetCurrentProcess(), &_hProcess, 0, TRUE, DUPLICATE_SAME_ACCESS), [](BOOL b) { return FALSE == b; });
      }
      if (src._hMainThread) {
        xtd::windows::exception::throw_if(DuplicateHandle(GetCurrentProcess(), src._hMainThread, GetCurrentProcess(), &_hMainThread, 0, TRUE, DUPLICATE_SAME_ACCESS), [](BOOL b) { return FALSE == b; });
      }
    }

    process(process&& src) : _pid(src._pid), _hProcess(src._hProcess), _hMainThread(src._hMainThread){
      src._pid = 0;
      src._hProcess = nullptr;
      src._hMainThread = nullptr;
    }
    process& operator=(const process& src){
      _pid = src._pid;
      _hProcess = nullptr;
      _hMainThread = nullptr;
      if (src._hProcess) {
        xtd::windows::exception::throw_if(DuplicateHandle(GetCurrentProcess(), src._hProcess, GetCurrentProcess(), &_hProcess, 0, TRUE, DUPLICATE_SAME_ACCESS), [](BOOL b){ return FALSE==b;});
      }
      if (src._hMainThread) {
        xtd::windows::exception::throw_if(DuplicateHandle(GetCurrentProcess(), src._hMainThread, GetCurrentProcess(), &_hMainThread, 0, TRUE, DUPLICATE_SAME_ACCESS), [](BOOL b) { return FALSE == b; });
      }
      return *this;
    }
    process& operator=(process&& src){
      std::swap(_pid, src._pid);
      std::swap(_hProcess, src._hProcess);
      std::swap(_hMainThread, src._hMainThread);
      return *this;
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
        xtd::tstring sPath(MAX_PATH, 0);
        forever {
          dwNeeded = GetModuleFileNameEx(*this, module, &sPath[0], static_cast<DWORD>(sPath.size()));
          if (dwNeeded < sPath.size()) {
            break;
          }
          sPath.resize(dwNeeded);
        }
        oRet[xtd::filesystem::path(sPath.c_str())] = dynamic_library::pointer(new dynamic_library(module));
      }
      return oRet;
    }

    handle_type handle() {
      if (!_hProcess) _hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, _pid);
      return _hProcess;
    }

    operator handle_type() {
      if (!_hProcess) {
        _hProcess = xtd::windows::exception::throw_if(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, _pid), [](HANDLE h) { return NULL == h; });
      }
      return _hProcess;
    }

    const xtd::filesystem::path& path() {
      if (!_path) {
        DWORD dwsize = 1 + MAX_PATH;
        xtd::tstring stemp(dwsize, 0);
        if (::QueryFullProcessImageName(handle(), 0, &stemp[0], &dwsize)) {
          stemp.resize(dwsize);
        }else {
          stemp = __("");
        }
        _path = xtd::make_unique<xtd::filesystem::path>(stemp.c_str());
      }
      return *_path;
    }

    static process create(const xtd::tstring& sPath) {
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      memset(&si, 0, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      memset(&pi, 0, sizeof(PROCESS_INFORMATION));
      xtd::tstring sMutablePath = sPath;
      xtd::windows::exception::throw_if(::CreateProcess(nullptr, &sMutablePath[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi), [](BOOL b) { return !b; });      
      return process(pi.dwProcessId, pi.hProcess, pi.hThread);
    }

  private:
    pid_type _pid;
    handle_type _hProcess;
    handle_type _hMainThread;
    std::unique_ptr<xtd::filesystem::path> _path;
  };

#else
  #error "Unsupported system for xtd::process"
#endif


}
