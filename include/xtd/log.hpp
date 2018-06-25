/** @file
  runtime logging to various destinations
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once


#include <xtd/xtd.hpp>

#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>
#include <deque>

#if (XTD_LOG_TARGET_CSV | XTD_LOG_TARGET_XML)
#include <fstream>
#endif

#if XTD_LOG_TARGET_SYSLOG
  #include <syslog.h>
#endif

#if XTD_LOG_TARGET_COUT
#include <iostream>
#endif


#include <xtd/string.hpp>
#include <xtd/source_location.hpp>
#include <xtd/filesystem.hpp>
#include <xtd/process.hpp>
#include <xtd/executable.hpp>
#include <xtd/meta.hpp>

#define FATAL(...) xtd::log::get().write(xtd::log::type::fatal, here(), __VA_ARGS__)
#define ERR(...)  xtd::log::get().write(xtd::log::type::error, here(), __VA_ARGS__)
#define WARNING(...)  xtd::log::get().write(xtd::log::type::warning, here(), __VA_ARGS__)
#define INFO(...) xtd::log::get().write(xtd::log::type::info, here(), __VA_ARGS__)
#define DBG(...)  xtd::log::get().write(xtd::log::type::debug, here(), __VA_ARGS__)

namespace xtd {

  class log {
  public:
    enum class type {
      fatal,
      error,
      warning,
      info,
      debug,
      enter,
      leave,
    };

    static const char* type_string(type oType) {
      switch (oType) {
        case xtd::log::type::fatal:
          return "fatal";
        case xtd::log::type::error:
          return "error";
        case xtd::log::type::warning:
          return "warning";
        case xtd::log::type::debug:
          return "debug";
        case xtd::log::type::enter:
          return "enter";
        case xtd::log::type::leave:
          return "leave";
        default:
          return "info";
      }
    }


    class message {
    public:
      using pointer_type = std::shared_ptr<message>;
      using deque_type = std::deque<pointer_type>;
      using time_type = std::chrono::time_point<std::chrono::system_clock>;

      message(type msg_type, const xtd::source_location& location, xtd::string&& text)
        : _tid(std::this_thread::get_id()), _type(msg_type), _location(location), _text(std::move(text)), _time(std::chrono::system_clock::now()) {}

      message(const message& src)
        : _tid(src._tid), _type(src._type), _location(src._location), _text(src._text), _time(src._time) {}

      message& operator=(const message& src) {
        if (this == &src) {
          return *this;
        }
        _tid = src._tid;
        _type = src._type;
        _location = src._location;
        _text = src._text;
        _time = src._time;
        return *this;
      }

      std::thread::id _tid;
      type _type;
      source_location _location;
      xtd::string _text;
      time_type _time;
    };

    class log_target {
    public:
      virtual ~log_target() = default;
      using pointer_type = std::shared_ptr<log_target>;
      using vector_type = std::vector<pointer_type>;
      virtual void operator()(const message::pointer_type&) = 0;
    };

    void Exit() {
      {
        std::lock_guard<std::mutex> oLock(_callback_lock);
        _callbacks.push_back([this]() {
          _callbackThreadExit = true;
        });
        _callbackCheck.notify_one();
      }
      if (_callbackThread.joinable()) {
        _callbackThread.join();
        _callbackThreadFinished.get_future().get();
      }
    }


    template <typename _ty>
    void AddTarget() {
      std::lock_guard<std::mutex> oLock(_callback_lock);
      _logTargets.emplace_back(new _ty);
    }

    void AddTarget(log_target::pointer_type oTarget) {
      std::lock_guard<std::mutex> oLock(_callback_lock);
      _logTargets.push_back(oTarget);
    }
  private:

#if (XTD_LOG_TARGET_SYSLOG)
    class syslog_target : public log_target{
    public:

      syslog_target(){ openlog(nullptr, LOG_PID | LOG_NDELAY, 0); }
      ~syslog_target() override { closelog(); }
      void operator()(const message::pointer_type& oMessage) override {
        int iFacility = LOG_MAKEPRI(LOG_USER, LOG_DEBUG);
        switch (oMessage->_type){
          case type::fatal:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_CRIT); 
            break;
          }
          case type::error:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_ERR); 
            break;
          }
          case type::warning:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_WARNING); 
            break;
          }
          case type::info:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_INFO); 
            break;
          }
          case type::debug:
          case type::enter:
          case type::leave:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_DEBUG); 
            break;
          }
        }
        syslog(iFacility, "%s", oMessage->_text.c_str());
      }
    };
#endif

#if (XTD_LOG_TARGET_WINDBG)
    class win_dbg_target : public log_target {
    public:
      ~win_dbg_target() override = default;

      void operator()(const message::pointer_type& oMessage) override {
        OutputDebugStringA(xtd::string::format(oMessage->_text, "\n").c_str());
      }
    };
#endif


#if (XTD_LOG_TARGET_COUT)
    class std_cout_target : public log_target {
    public:
      ~std_cout_target() override = default;

      void operator()(const message::pointer_type& oMessage) override {
        std::cout << oMessage->_text << std::endl;
      }
    };
#endif

#if (XTD_LOG_TARGET_CSV)
    class csv_target : public log_target {
      std::ofstream _logfile;
      std::mutex _FileLock;
    public:


      void operator()(const message::pointer_type& oMsg) override {
        static thread_local size_t _StackDepth = 1;
        if (type::leave == oMsg->_type) --_StackDepth;
        std::string sMsgPrefix(_StackDepth, ',');
        std::hash<std::thread::id> oHash;
        auto sMsg = xtd::string::format(oHash(oMsg->_tid), ",", oMsg->_time.time_since_epoch().count(), ",", type_string(oMsg->_type), ",", oMsg->_location.file(), ",", oMsg->_location.line(), sMsgPrefix, oMsg->_text);
        if (type::enter == oMsg->_type) ++_StackDepth;
        std::unique_lock<std::mutex> oLock(_FileLock);
        _logfile << sMsg << '\n';
      }

      csv_target() : _logfile(), _FileLock() {
        auto oLogPath = xtd::filesystem::home_directory_path();
        oLogPath /= xtd::executable::this_executable().path().filename();
        if (!xtd::filesystem::exists(oLogPath)) xtd::filesystem::create_directories(oLogPath);
        oLogPath /= xtd::tstring::format(intrinsic_cast(xtd::process::this_process().id()), __(".csv"));
        _logfile.open(oLogPath.tstring(), std::ios::out);
      }
    };
#endif

    void callback_thread() {
      _callbackThreadStarted.set_value();
      while (!_callbackThreadExit) {
        callback_type oCallback; {
          std::unique_lock<std::mutex> oLock(_callback_lock);
          _callbackCheck.wait(oLock, [this] {
                                return _callbacks.size();
                              });
          oCallback = _callbacks.front();
          _callbacks.pop_front();
          oLock.unlock();
          _callbackCheck.notify_one();
        }
        if (oCallback) {
          oCallback();
        }
      }
      _callbackThreadFinished.set_value();
    }

    log() : _messages(), _callbacks(), _callbackThread(), _callback_lock(), _callbackCheck(), _logTargets()
        ,_callbackThreadStarted(), _callbackThreadFinished()
    {

#if (XTD_LOG_TARGET_SYSLOG)
      _logTargets.emplace_back(new syslog_target);
#endif

#if (XTD_LOG_TARGET_WINDBG)
      _logTargets.emplace_back(new win_dbg_target);
#endif

#if (XTD_LOG_TARGET_COUT)
      _logTargets.emplace_back(new std_cout_target);
#endif

#if (XTD_LOG_TARGET_CSV)
      _logTargets.emplace_back(new csv_target);
#endif

      _callbackThread = std::thread(&log::callback_thread, this);
      _callbackThreadStarted.get_future().get();
    }

    ~log() { Exit(); }

    using callback_type = std::function<void()>;
    using callback_deque = std::deque<callback_type>;

    message::deque_type _messages;
    callback_deque _callbacks;
    std::thread _callbackThread;
    std::mutex _callback_lock;
    std::condition_variable _callbackCheck;
    log_target::vector_type _logTargets;
    std::promise<void> _callbackThreadStarted;
    std::promise<void> _callbackThreadFinished;
    bool _callbackThreadExit = false;

  public:

    static log& get() {
      static log _log;
      return _log;
    }

    template <typename ... _arg_ts>
    inline void write(type mesageType, const source_location& location, _arg_ts&&...oArgs) {
      auto sMessage = xtd::string::format(std::forward<_arg_ts>(oArgs)...);
      auto oMessage = std::make_shared<message>(mesageType, location, std::move(sMessage)); {
        std::lock_guard<std::mutex> oLock(_callback_lock);
        _callbacks.push_back([oMessage, this]() {
            for (auto oTarget : _logTargets) {
              (*oTarget)(oMessage);
            }
          });
      }
      _callbackCheck.notify_one();
    }

  };


}
