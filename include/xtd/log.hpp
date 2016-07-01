/** @file
  runtime logging to various destinations
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#define FATAL(...) xtd::log::get().write(xtd::log::type::fatal, here(), __VA_ARGS__)
#define ERR(...)  xtd::log::get().write(xtd::log::type::error, here(), __VA_ARGS__)
#define WARNING(...)  xtd::log::get().write(xtd::log::type::warning, here(), __VA_ARGS__)
#define INFO(...) xtd::log::get().write(xtd::log::type::info, here(), __VA_ARGS__)
#define DBG(...)  xtd::log::get().write(xtd::log::type::debug, here(), __VA_ARGS__)

namespace xtd{

  struct log{
    enum class type{
      fatal,
      error,
      warning,
      info,
      debug,
      enter,
      leave,
    };

  private:
    
    struct message{


      using pointer_type = std::shared_ptr<message>;
      using deque_type = std::deque<pointer_type>;
      using time_type = std::chrono::time_point<std::chrono::system_clock>;

      message(type msg_type, const xtd::source_location& location, xtd::string&& text)
        : _tid(std::this_thread::get_id()), _type(msg_type), _location(location), _text(std::move(text)), _time(std::chrono::system_clock::now()){}
      message(const message& src)
        :_tid(src._tid), _type(src._type), _location(src._location), _text(src._text), _time(src._time){}
      message& operator=(const message& src){
        if (this == &src){
          return *this;
        }
        _tid=src._tid;
        _type = src._type;
        _location = src._location;
        _text = src._text;
        _time= src._time;
        return *this;
      }

      std::thread::id _tid;
      type _type;
      source_location _location;
      xtd::string _text;
      time_type _time;
    };

    struct syslog_target{
    #if (!XTD_LOG_TARGET_SYSLOG)
      void operator()(const message::pointer_type&){}
    #else
      syslog_target(){ openlog(nullptr, LOG_PID | LOG_NDELAY, 0); }
      ~syslog_target(){ closelog(); }
      void operator()(const message::pointer_type& oMessage) {
        int iFacility = LOG_MAKEPRI(LOG_USER, LOG_DEBUG);
        switch (oMessage->type){
          case message::type::fatal:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_CRIT); 
            break;
          }
          case message::type::error:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_ERR); 
            break;
          }
          case message::type::warning:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_WARNING); 
            break;
          }
          case message::type::info:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_INFO); 
            break;
          }
          case message::type::debug:
          case message::type::enter:
          case message::type::leave:
          {
            iFacility = LOG_MAKEPRI(LOG_USER, LOG_DEBUG); 
            break;
          }
        }
        syslog(iFacility, "%s", oMessage->_text.c_str());
      }

    #endif
    };
    syslog_target _syslog_target;

    struct win_dbg_target{
    #if (!XTD_LOG_TARGET_WINDBG)
      void operator()(const message::pointer_type&){}
    #else
      void operator()(const message::pointer_type& oMessage) {
        OutputDebugStringA(oMessage->_text.c_str());
      }
    #endif
    };
    win_dbg_target _win_dbg_target;

    struct csv_target{
      void operator()(const message::pointer_type&){ TODO("Implement me"); }
    };
    csv_target _csv_target;

    struct xml_target{
      void operator()(const message::pointer_type&){ TODO("Implement me"); }
    };
    xml_target _xml_target;

    struct std_cout_target {
    #if (!XTD_LOG_TARGET_COUT)
      void operator()(const message::pointer_type&) const {}
    #else
      void operator()(const message::pointer_type& oMessage) const {
        std::cout << oMessage->_text << std::endl;
      }
    #endif
    };
    std_cout_target _std_cout_target;

    void callback_thread(){
      for (; !_Exit;){
        callback_type oCallback;
        {
          std::unique_lock<std::mutex> oLock(_CallbackLock);
          _CallbackCheck.wait(oLock, [this]{
            return _Callbacks.size();
          });
          oCallback = _Callbacks.front();
          _Callbacks.pop_front();
          oLock.unlock();
          _CallbackCheck.notify_one();
        }
        if (!oCallback) continue;
        oCallback();
      }
    }

    log(){
      _CallbackThread = std::thread(&log::callback_thread, this);
    }

    ~log(){
      {
        std::lock_guard<std::mutex> oLock(_CallbackLock);
        _Callbacks.push_back([this](){
          _Exit = true;
        });
        _CallbackCheck.notify_one();
      }
      _CallbackThread.join();
    }

    using callback_type = std::function<void()>;
    using callback_deque = std::deque<callback_type>;

    message::deque_type _Messages;
    callback_deque _Callbacks;
    std::thread _CallbackThread;
    std::mutex _CallbackLock;
    std::condition_variable _CallbackCheck;
    bool _Exit = false;

    public:

      static log& get(){
        static log _log;
        return _log;
      }

      template <typename ... _ArgTs>
      inline void write(type mesageType, const source_location& location, _ArgTs&&...oArgs){
        auto sMessage = xtd::string::format(std::forward<_ArgTs>(oArgs)...);
        if ('\n' != sMessage.back()){
          sMessage += '\n';
        }
        auto oMessage = std::make_shared<message>(mesageType, location, std::move(sMessage));
        {
          std::lock_guard<std::mutex> oLock(_CallbackLock);
          _Callbacks.push_back([oMessage, this](){
            _std_cout_target(oMessage);
            _win_dbg_target(oMessage);
            _syslog_target(oMessage);
            _csv_target(oMessage);
            _xml_target(oMessage);
          });
        }
        _CallbackCheck.notify_one();
      }

  };


}
