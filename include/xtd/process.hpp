/** @file
  represents an in-memory process
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

namespace xtd{


#if ((XTD_OS_LINUX | XTD_OS_MSYS | XTD_OS_CYGWIN) & XTD_OS)
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
#if (XTD_OS_LINUX & XTD_OS)
    dynamic_library::map libraries() {

      dynamic_library::map oRet;
      /*
      for (auto pMap = reinterpret_cast<const struct link_map *>(dlopen(0, RTLD_LAZY)); pMap; pMap = pMap->l_next) {
        if (pMap->l_name) {
          std::cout << pMap->l_name << std::endl;
        }
      }
       */
      return oRet;
    }
#endif
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
  class process{
  public:

    using pid_type = DWORD;
    using pointer = std::shared_ptr<process>;
    using map = std::map<pid_type, pointer>;


    static process& this_process(){
      static process _this_process(GetCurrentProcessId());
      return _this_process;
    }

    static map system_processes(){
      map oRet;
      std::vector<DWORD> pids(10, 0);
      DWORD dwNeeded;
      BOOL bRet;
      DWORD dwLastError;
      forever{
        bRet = EnumProcesses(&pids[0], pids.size() * sizeof(DWORD), &dwNeeded);
        dwLastError = GetLastError();
        if ((dwNeeded / sizeof(DWORD)) < pids.size()){
          break;
        }
        pids.resize(pids.size() * 2);
      }
      pids.resize(dwNeeded / sizeof(DWORD));
      for (auto pid : pids){
        oRet[pid] = pointer(new process(pid));
      }
      return oRet;
    }

    explicit process(pid_type hPid) : _pid(hPid), _hProcess(nullptr){}
    ~process(){
      if (_hProcess){
        CloseHandle(_hProcess);
      }
    }

    pid_type id() const{ return _pid; }

    dynamic_library::map libraries(){
      dynamic_library::map oRet;
      std::vector<HMODULE> modules(10, 0);
      DWORD dwNeeded;
      BOOL bRet;
      DWORD dwLastError;
      forever{
        bRet = EnumProcessModules(*this, &modules[0], modules.size() * sizeof(HMODULE), &dwNeeded);
        dwLastError = GetLastError();
        if ((dwNeeded / sizeof(HMODULE)) < modules.size()){
          break;
        }
        modules.resize(modules.size() * 2);
      }
      modules.resize(dwNeeded / sizeof(HMODULE));
      for (dynamic_library::native_handle_type module : modules){
        xtd::string sPath(MAX_PATH, 0);
        forever{
          dwNeeded = GetModuleFileNameExA(*this, module, &sPath[0], sPath.size());
          if (dwNeeded < sPath.size()){
            break;
          }
          sPath.resize(dwNeeded);
        }
        oRet[sPath] = dynamic_library::pointer(new dynamic_library(module));
      }
      return oRet;
    }



  private:
    pid_type _pid;
    HANDLE _hProcess;
    operator HANDLE(){
      if (!_hProcess){
        _hProcess = xtd::windows::exception::throw_if(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _pid), [](HANDLE h){ return NULL == h; });
      }
      return _hProcess;
    }
  };

#else
  #error "Unsupported system for xtd::process"
#endif





}
