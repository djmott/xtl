/**@file
transport neutral light weight IPC/RPC library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

#include <xtd/socket.hpp>
#include <xtd/concurrent/hash_map.hpp>
#include <xtd/memory.hpp>
#include <xtd/debug.hpp>

namespace xtd{
  namespace rpc{
    template <typename _decl_t, typename _class_t> class rpc_call;
    /** generic-form marshaler used by clients and servers
    @tparam _skip_byval permits selective marshaling and unmarshaling based on the type. For example, const values can be marshaled on the client side and unmarshaled on the server side but not unmarshaled on the client side
    @tparam ... parameter pack of types to recursively marshal or unmarshal
    */
    template <bool _skip_byval, typename ...> class marshaler;
    /** generic-form marshaler_base
    Used to recursively marshal a parameter pack of data into a payload
    @tparam ... parameter pack of types to marshal or unmarshal
    */
    template <typename...> class marshaler_base;
    template <class _transport_t, class _decl_t, class ... _call_ts> class server_impl;
    template <typename _transport_t, class ... _call_ts> class client;




    
    /** represents an exception with the RPC protocol
     * typically occurs as the result of failing to parse payloads
    */
    class protocol_exception : public xtd::exception{
    public:
      using _super_t = xtd::exception;
      template <typename ... _arg_ts> protocol_exception(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}
    };
    class malformed_payload : public protocol_exception{
    public:
      using _super_t = rpc::protocol_exception;
      template <typename ... _arg_ts> malformed_payload(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}
    };
    class bad_call : public protocol_exception{
    public:
      using _super_t = rpc::protocol_exception;
      template <typename ... _arg_ts> bad_call(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}
    };


    /** contains the packed call data on the wire that is transported between clients and servers
*/
    class payload : public std::vector<uint8_t>{
    public:

      using _super_t = std::vector<uint8_t>;
      using invoke_handler_type = std::function<bool(payload&)>;

      template <typename ... _arg_ts> payload(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      template <typename _ty> _ty peek() const{
        if (_super_t::size() < sizeof(_ty)) throw rpc::malformed_payload(here(), "Malformed payload");
        return _ty(*reinterpret_cast<const _ty*>(&_super_t::at(0)));
      }
    };

    ///marshaler_base specialization of no type
    template <> class marshaler_base<>{
    public:
      static void marshal(payload&){ /*this specialization is a recursion terminator so has no implementation*/}
      static void unmarshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
    };

/** marshaler_base specialization of any type
throws a static assertion if used with a non-pod type indicating that a specialization for the type is missing
@tparam _ty the type of value to marshal and unmarshal from the payload
*/
    template <typename _ty>
    class marshaler_base<_ty>{
    public:
      static void marshal(payload& oPayload, const _ty& val){
        static_assert(std::is_pod<_ty>::value, "Invalid specialization");
        oPayload.insert(oPayload.end(), reinterpret_cast<const uint8_t*>(&val), sizeof(_ty) + reinterpret_cast<const uint8_t*>(&val));
      }
      static _ty unmarshal(payload& oPayload){
        static_assert(std::is_pod<_ty>::value, "Invalid specialization");
        _ty oRet(*reinterpret_cast<_ty*>(&oPayload[0]));
        oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(_ty));
        return oRet;
      }
    };

/// marshaler_base specialization for static arrays
    template <typename _ty, size_t _len>
    class marshaler_base<_ty(&)[_len] >{
    public:
      static void marshal(payload& oPayload, const _ty(&val)[_len]){
        marshaler_base<size_t>::marshal(oPayload, _len);
        for (const auto & oItem : val) marshaler_base<_ty>::marshal(oPayload, oItem);
      }
    };


/// marshaler_base specialization for std::vector
    template <typename _ty> class marshaler_base<std::vector < _ty>> {
      public:
      static void marshal(payload& oPayload, const std::vector<_ty>& val){
        marshaler_base<size_t>::marshal(oPayload, val.size());
        for (const auto & oItem : val) marshaler_base<_ty>::marshal(oPayload, oItem);
      }
      static std::vector < _ty> unmarshal(payload& oPayload){
        std::vector < _ty> oRet;
        auto iSize = marshaler_base<std::size_t>::unmarshal(oPayload);
        for (std::size_t i = 0; i < iSize; i++){
          oRet.push_back(marshaler_base<_ty>::unmarshal(oPayload));
        }
        return oRet;
      }
    };


  /// marshaler_base specialization for std::string
    template <>
    class marshaler_base<std::string>{
    public:
      static void marshal(payload & oPayload, const std::string& val){
        marshaler_base<std::size_t>::marshal(oPayload, val.size());
        oPayload.insert(oPayload.end(), val.cbegin(), val.cend());
      }
      static std::string unmarshal(payload& oPayload){

        auto iSize = marshaler_base<size_t>::unmarshal(oPayload);
        std::string oRet(oPayload.begin(), oPayload.begin() + iSize);
        oPayload.erase(oPayload.begin(), oPayload.begin() + iSize);
        return oRet;
      }
    };

  /// marshaler specialization with no type
    template <bool _skip_byval> class marshaler<_skip_byval>{
    public:
      static void marshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
      static void unmarshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
    };

  /** marshaler specializaiton to skip marshaling/unmarshaling a constant type
  This specialization is chosen by the compiler for constant reference types that should be skipped
  */
    template <typename _ty, typename..._arg_ts> class marshaler<true, const _ty&, _arg_ts...>{
    public:
      static void marshal(payload& oPayload, const _ty&, _arg_ts&&...oArgs){
        marshaler<true, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, const _ty&, _arg_ts&&...oArgs){
        marshaler<true, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
    };


  /** marshaler specialization to skip marshaling/unmarshaling a by-value type
  This specialization is chosen by the compiler for by-value types that should be skipped
  */
    template <typename _ty, typename ..._arg_ts> class marshaler<true, _ty, _arg_ts...>{
    public:
      static void marshal(payload& oPayload, const _ty&, _arg_ts&&...oArgs){
        marshaler<true, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, const _ty&, _arg_ts&&...oArgs){
        marshaler<true, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
    };


  /** marshaler specialization to process marshaling/unmarshaling a non-const reference type
  This specialization is chosen by the compiler for references types that should not be skipped
  */
    template <typename _ty, typename ..._arg_ts> class marshaler<false, _ty&, _arg_ts...>{
    public:
      static void marshal(payload& oPayload, const _ty& value, _arg_ts&&...oArgs){
        marshaler_base<_ty>::marshal(oPayload, value);
        marshaler<false, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, _ty& value, _arg_ts&&...oArgs){
        value = marshaler_base<_ty>::unmarshal(oPayload);
        marshaler<false, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
    };

  /** marshaler specialization to process marshaling/unmarshaling a by-value type
  This specialization is chosen by the compiler for by-value types that should not be skipped
  */
    template <typename _ty, typename ..._arg_ts> class marshaler<false, _ty, _arg_ts...>{
    public:
      static void marshal(payload& oPayload, const _ty& value, _arg_ts&&...oArgs){
        marshaler_base<_ty>::marshal(oPayload, value);
        marshaler<false, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, _ty& value, _arg_ts&&...oArgs){
        value = marshaler_base<_ty>::unmarshal(oPayload);
        marshaler<false, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
      }
    };


    /** super class of rpc call declarations
    concrete rpc call declarations subclass rpc_call
    @tparam _decl_t the concrete implementation type passed as a curiously recurring parameter
    @tparam _return_t return type of the rpc call
    @tparam ... list of call parameters
    */
    template <typename _decl_t, typename _return_t, typename ... _arg_ts> class rpc_call<_decl_t, _return_t(_arg_ts...)>{
      
    public:
      using return_type = _return_t;
      using function_type = std::function<_return_t(_arg_ts...)>;
      using upload_marshaler_type = marshaler<false, size_t, _arg_ts...>;
      using download_marshaler_type = marshaler<true, _arg_ts..., _return_t&>;
    };


    /** tcp/ip transport
    */
    class tcp_transport{
      socket::ipv4address _address;
      xtd::socket::ipv4_tcp_stream _socket;
      std::unique_ptr<std::thread> _server_thread;
      bool _stop_server;
      xtd::concurrent::hash_map<std::thread::id, std::thread> _clients;
      bool _client_connected;

    public:
      using pointer_type = std::shared_ptr<tcp_transport>;

      tcp_transport(const socket::ipv4address& oAddress)
        : _address(oAddress), _socket(), _server_thread(), _stop_server(false), _clients(), _client_connected(false){}

      void start_server(payload::invoke_handler_type oHandler){
        _stop_server = false;
        std::shared_ptr<std::promise<void>> oServerStarted(new std::promise<void>);
        std::shared_ptr<payload::invoke_handler_type> oInvokeHandler(new payload::invoke_handler_type(oHandler));
        _server_thread = xtd::make_unique<std::thread>([&, oInvokeHandler, oServerStarted]{
          oServerStarted->set_value();
          _socket.bind(_address);
          bool ExitThread = false;
          payload oPayload;
          while (!_stop_server && !ExitThread){
            _socket.listen();
            auto oClient = _socket.accept<xtd::socket::ipv4_tcp_stream>();
            std::shared_ptr<xtd::socket::ipv4_tcp_stream> oClientSocket(new xtd::socket::ipv4_tcp_stream(std::move(oClient)));
            std::thread oClientThread([&, oClientSocket, oInvokeHandler](){
              oClientSocket->onError.connect([&ExitThread](){
                ERR("Socket error");
                ExitThread = true;
              });
              oClientSocket->onRead.connect([&](){ 
                oClientSocket->read<payload::_super_t>(oPayload);
                if (!oHandler(oPayload)){
                  ExitThread = true;
                }
              });
              oClientSocket->onWrite.connect([&](){
                if (oPayload.size()){
                  oClientSocket->write<payload::_super_t>(oPayload);
                }
              });
              payload oPayload;
              for (;;){
                oClientSocket->select(250);
              }
            });
            oClientThread.detach();
          }
        });
        oServerStarted->get_future().get();
      }
      void stop_server(){
        //TODO: implement stop_server
        TODO("implement stop_server")
      }
      void transact(payload& oPayload){
        if (!_client_connected){
          _socket.connect(_address);
          _client_connected = true;
        }
        _socket.write<typename payload::_super_t>(oPayload);
        _socket.read<typename payload::_super_t>(oPayload);

      }

    };

    /** Dummy transport used for debugging
    */
    class null_transport{

      struct globals{
        std::thread _server_thread;
        std::mutex _call_lock;
        std::condition_variable _call_check;
        std::promise<void> _server_thread_start;
        bool _server_thread_exit = false;

        globals() : _server_thread(), _call_lock(), _call_check(), _server_thread_start(), _transport_info(){}

        struct transport_info{
          using pointer = std::shared_ptr<transport_info>;
          using stack = xtd::concurrent::stack<pointer>;
          transport_info() : _processed(), oPayload(){}
          std::promise<void> _processed;
          payload oPayload;
        };

        transport_info::stack _transport_info;

        static globals& get(){
          static globals _globals;
          return _globals;
        }
      };

    public:

      using pointer_type = std::shared_ptr<null_transport>;

      void start_server(payload::invoke_handler_type oHandler){
        std::shared_ptr<payload::invoke_handler_type> oInvokeHandler(new payload::invoke_handler_type(oHandler));
        globals::get()._server_thread = std::thread([&, oInvokeHandler]{
          globals::get()._server_thread_start.set_value();
          globals::transport_info::pointer pTransportInfo;
          while (!globals::get()._server_thread_exit){
            {
              std::unique_lock<std::mutex> oLock(globals::get()._call_lock);
              globals::get()._call_check.wait(oLock, [&]{ return globals::get()._transport_info.try_pop(pTransportInfo); });
              oLock.unlock();
              globals::get()._call_check.notify_one();
            }
            if (pTransportInfo->oPayload.size()){
              (*oInvokeHandler)(pTransportInfo->oPayload);
            }
            pTransportInfo->_processed.set_value();
          }
        });
        globals::get()._server_thread_start.get_future().get();
      }
      void stop_server(){
        globals::get()._server_thread_exit = true;
        globals::transport_info::pointer pTransportInfo(new globals::transport_info);
        {
          std::lock_guard<std::mutex> oLock(globals::get()._call_lock);
          globals::get()._transport_info.push(pTransportInfo);
          globals::get()._call_check.notify_one();
        }
        pTransportInfo->_processed.get_future().get();

        globals::get()._server_thread.join();
      }

      void transact(payload& oPayload){
        globals::transport_info::pointer pTransportInfo(new globals::transport_info);
        pTransportInfo->oPayload = oPayload;
        {
          std::lock_guard<std::mutex> oLock(globals::get()._call_lock);
          globals::get()._transport_info.push(pTransportInfo);
          globals::get()._call_check.notify_one();
        }
        pTransportInfo->_processed.get_future().get();
      }
    };


    /** super class of server implementation chain
    @tparam _transport_t the transport type to use (tcp, udp, named pipes, homing pigeons, etc.)
    @tparam _decl_t the full server declaration
    */
    template <class _transport_t, class _decl_t> class server_impl<_transport_t, _decl_t>{
    protected:

    public:
      virtual bool call_handler(payload&){
        return false;
      }

      virtual ~server_impl(){
        try{
          stop_server();
        }
        catch (const xtd::exception& ex){
          ERR("An unhandled xtd::exception occured while stopping the transport: ", ex.what());
        }
      }


      typename _transport_t::pointer_type _tranport;

      server_impl(typename _transport_t::pointer_type& oTransport) : _tranport(oTransport){}

      template <typename ... _xport_ctor_ts> server_impl(_xport_ctor_ts&&...oParams) : _tranport(new _transport_t(std::forward<_xport_ctor_ts>(oParams)...)){}

      void start_server(){ 
        _tranport->start_server([this](payload& oPayload)->bool{ return static_cast<_decl_t*>(this)->call_handler(oPayload); });
      }

      void stop_server(){ 
        _tranport->stop_server();
      }


    };


    template <class _transport_t, class _decl_t, class _head_t, class ... _tail_t>
    class server_impl<_transport_t, _decl_t, _head_t, _tail_t...>
      : public server_impl<_transport_t, _decl_t, _tail_t...>{
    protected:
      using _super_t = server_impl<_transport_t, _decl_t, _tail_t...>;
      using function_type = typename _head_t::function_type;
      function_type _function;

      template <typename _call_t, typename _param_t> void _attach(typename std::enable_if<std::is_same<_call_t, _head_t>::value, _param_t>::type oParam){
        _function = oParam;
      }

      template <typename _call_t, typename _param_t> void _attach(_param_t oCallImpl){ _super_t::template _attach<_call_t, _param_t>(oCallImpl); }



    public:


      template <typename ... _xport_ctor_ts> server_impl(_xport_ctor_ts&&...oParams)
        : _super_t(std::forward<_xport_ctor_ts>(oParams)...), _function() {}

      virtual bool call_handler(payload& oPayload) override{
        if (typeid(_head_t).hash_code() != oPayload.peek<size_t>()){
          return _super_t::call_handler(oPayload);
        }
        TODO("invoke call")
          return true;
      }

      template <typename _call_ts, typename _param_t> void attach(_param_t oCallImpl){ _attach<_call_ts, _param_t>(oCallImpl); }

    };

    template <class _transport_t, class ... _call_ts> class server : public server_impl<_transport_t, server<_transport_t, _call_ts...>, _call_ts...>{
      using _super_t = server_impl<_transport_t, server<_transport_t, _call_ts...>, _call_ts...>;
    public:
      using client_type = client<_transport_t, _call_ts...>;
      template <typename ... _export_ctor_ts> server(_export_ctor_ts&&...oParams) : _super_t(std::forward<_export_ctor_ts>(oParams)...){}
      virtual ~server(){}
    };


    template<typename _transport_t >
    class client<_transport_t>{
    protected:
      _transport_t _transport;

    public:
      using server_type = server<_transport_t>;
      template<typename ... _xport_ctor_ts> explicit client(_xport_ctor_ts &&...oArgs) : _transport(std::forward<_xport_ctor_ts>(oArgs)...){}

      _transport_t& transport(){ return _transport; }
      const _transport_t& transport() const { return _transport; }

      void connect(){ _transport.connect(); }

    };


    template<typename _transport_t, class _head_t, class ..._tail_ts >
    class client<_transport_t, _head_t, _tail_ts...> : public client<_transport_t, _tail_ts...>{
    protected:
      using _super_t = client<_transport_t, _tail_ts...>;

      template <typename _call_t, typename ..._param_ts>
      typename _head_t::return_type _call(typename std::enable_if<std::is_same<_call_t, _head_t>::value, const std::type_info&>::type, _param_ts&&... oParams) {
        payload oPayload;
        typename _head_t::return_type oRet;
        _head_t::upload_marshaler_type::marshal(oPayload, typeid(_call_t).hash_code(), std::forward<_param_ts>(oParams)...);
        _super_t::_transport.transact(oPayload);
        _head_t::download_marshaler_type::unmarshal(oPayload, std::forward<_param_ts>(oParams)..., oRet);
        return oRet;
      }

      template <typename _call_ts, typename ..._param_ts>
      typename _call_ts::return_type _call(typename std::enable_if<!std::is_same<_call_ts, _head_t>::value, const std::type_info&>::type oType, _param_ts&&... oParams) {
        return _super_t::template _call<_call_ts>(oType, std::forward<_param_ts>(oParams)...);
      }
      
    public:
      using server_type = server<_transport_t, _head_t, _tail_ts...>;
      template<typename ... _xport_ctor_ts> explicit client(_xport_ctor_ts&&...oArgs) : client<_transport_t, _tail_ts...>(std::forward<_xport_ctor_ts>(oArgs)...){}

      template <typename _ty, typename ... _param_ts>
      typename _ty::return_type call(_param_ts&&...oParams) {
        return _call<_ty>(typeid(_ty), std::forward<_param_ts>(oParams)...);
      }
    };
  }
}
