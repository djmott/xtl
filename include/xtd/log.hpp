/** @file
  runtime logging to various destinations
  @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/

#pragma once

#define FATAL(...) xtd::log::get().write(xtd::log::type::fatal, here(), __VA_ARGS__)
#define ERR(...)  xtd::log::get().write(xtd::log::type::error, here(), __VA_ARGS__)
#define WARNING(...)  xtd::log::get().write(xtd::log::type::warning, here(), __VA_ARGS__)
#define INFO(...) xtd::log::get().write(xtd::log::type::info, here(), __VA_ARGS__)
#define DBG(...)  xtd::log::get().write(xtd::log::type::debug, here(), __VA_ARGS__)

namespace xtd{

  class log{
  public:
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
    
    class message{
    public:
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

    class log_target{
    public:
      virtual ~log_target() = default;
      using pointer_type = std::shared_ptr<log_target>;
      using vector_type = std::vector<pointer_type>;
      virtual void operator()(const message::pointer_type&) = 0;
    };

#if (XTD_LOG_TARGET_SYSLOG)
    class syslog_target : public log_target{
    public:

      syslog_target(){ openlog(nullptr, LOG_PID | LOG_NDELAY, 0); }
      virtual ~syslog_target(){ closelog(); }
      void operator()(const message::pointer_type& oMessage) override {
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
    };
#endif

#if (XTD_LOG_TARGET_WINDBG)
    class win_dbg_target : public log_target{
    public:
      virtual ~win_dbg_target(){}
      void operator()(const message::pointer_type& oMessage) override {
        OutputDebugStringA(oMessage->_text.c_str());
      }
    };
#endif


#if (XTD_LOG_TARGET_COUT)
    class std_cout_target : public log_target{
    public:
      virtual ~std_cout_target() = default;
      void operator()(const message::pointer_type& oMessage) override{
        std::cout << oMessage->_text << std::endl;
      }
    };
#endif

    void callback_thread(){
      while ( !_Exit ){
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
        if (oCallback) {
          oCallback();
        }
      }
    }

    log() : _Messages(), _Callbacks(), _CallbackThread(), _CallbackLock(), _CallbackCheck(), _LogTargets(){
#if (XTD_LOG_TARGET_SYSLOG)
      _LogTargets.emplace_back(new syslog_target);
#endif
#if (XTD_LOG_TARGET_WINDBG)
      _LogTargets.emplace_back(new win_dbg_target);
#endif
#if (XTD_LOG_TARGET_COUT)
      _LogTargets.emplace_back(new std_cout_target);
#endif
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
    log_target::vector_type _LogTargets;
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
            for (auto oTarget : _LogTargets){
              (*oTarget)(oMessage);
            }
          });
        }
        _CallbackCheck.notify_one();
      }

  };


}
