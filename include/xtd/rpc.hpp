#pragma once


namespace xtd{
  namespace rpc{

    template <typename _TransportT, class ... _Calls> class client;

    class protocol_exception : public xtd::exception{
    public:
      using _super_t = xtd::exception;

      template <typename ... _ArgTs> protocol_exception(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}
    };
    class malformed_payload : public protocol_exception{
    public:
      using _super_t = rpc::protocol_exception;
      template <typename ... _ArgTs> malformed_payload(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}
    };
    class bad_call : public protocol_exception{
    public:
      using _super_t = rpc::protocol_exception;
      template <typename ... _ArgTs> bad_call(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}
    };


    class payload : public std::vector<uint8_t>{
    public:

      using _super_t = std::vector<uint8_t>;

      template <typename ... _ArgTs> payload(_ArgTs...oArgs) : _super_t(std::forward<_ArgTs>(oArgs)...){}

      template <typename _Ty> const _Ty& peek() const{
        assert(_super_t::size() >= sizeof(_Ty));
        if (_super_t::size() < sizeof(_Ty)){
          throw malformed_payload(here(), "Invalid peek");
        }
        return *reinterpret_cast<const _Ty*>(&at(0));
      }
    };

/** generic-form marshaler_base
Used to recursively marshal a parameter pack of data into a payload
*/
    template <typename...> class marshaler_base;

///marshaler_base specialization of no type
    template <> class marshaler_base<>{
    public:
      static void marshal(payload&){ /*this specialization is a recursion terminator so has no implementation*/}
      static void unmarshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
    };

/** marshaler_base specialization of any type
throws a static assertion if used with a non-pod type indicating that a specialization for the type is missing
@tparam _Ty the type of value to marshal and unmarshal from the payload
*/
    template <typename _Ty>
    class marshaler_base<_Ty>{
    public:
      static void marshal(payload& oPayload, const _Ty& val){
        static_assert(std::is_pod<_Ty>::value, "Invalid specialization");
        oPayload.insert(oPayload.end(), reinterpret_cast<const uint8_t*>(&val), sizeof(_Ty) + reinterpret_cast<const uint8_t*>(&val));
      }
      static _Ty unmarshal(payload& oPayload){
        static_assert(std::is_pod<_Ty>::value, "Invalid specialization");
        _Ty oRet(*reinterpret_cast<_Ty*>(&oPayload[0]));
        oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(_Ty));
        return oRet;
      }
    };

/// marshaler_base specialization for static arrays
    template <typename _Ty, size_t _Len>
    class marshaler_base<_Ty(&)[_Len] >{
    public:
      static void marshal(payload& oPayload, const _Ty(&val)[_Len]){
        marshaler_base<size_t>::marshal(oPayload, _Len);
        for (const auto & oItem : val) marshaler_base<_Ty>::marshal(oPayload, oItem);
      }
    };


/// marshaler_base specialization for std::vector
    template <typename _Ty> class marshaler_base<std::vector < _Ty>> {
      public:
      static void marshal(payload& oPayload, const std::vector<_Ty>& val){
        marshaler_base<size_t>::marshal(oPayload, val.size());
        for (const auto & oItem : val) marshaler_base<_Ty>::marshal(oPayload, oItem);
      }
      static std::vector < _Ty> unmarshal(payload& oPayload){
        std::vector < _Ty> oRet;
        auto iSize = marshaler_base<std::size_t>::unmarshal(oPayload);
        for (std::size_t i = 0; i < iSize; i++){
          oRet.push_back(marshaler_base<_Ty>::unmarshal(oPayload));
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

  /** generic-form marshaler used by clients and servers
  @tparam _SkipByVal permits selective marshaling and unmarshaling based on the type. For example, const values can be marshaled on the client side and unmarshaled on the server side but not unmarshaled on the client side
  @tparam ... parameter pack of types to recursively marshal or unmarshal
  */
    template <bool _SkipByVal, typename ...> class marshaler;

  /// marshaler specialization with no type
    template <bool _SkipByVal> class marshaler<_SkipByVal>{
    public:
      static void marshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
      static void unmarshal(payload&){/*this specialization is a recursion terminator so has no implementation*/}
    };

  /** marshaler specializaiton to skip marshaling/unmarshaling a constant type
  This specialization is chosen by the compiler for constant reference types that should be skipped
  */
    template <typename _Ty, typename..._ArgTs> class marshaler<true, const _Ty&, _ArgTs...>{
    public:
      static void marshal(payload& oPayload, const _Ty&, _ArgTs...oArgs){
        marshaler<true, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, const _Ty&, _ArgTs...oArgs){
        marshaler<true, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
    };


  /** marshaler specialization to skip marshaling/unmarshaling a by-value type
  This specialization is chosen by the compiler for by-value types that should be skipped
  */
    template <typename _Ty, typename ..._ArgTs> class marshaler<true, _Ty, _ArgTs...>{
    public:
      static void marshal(payload& oPayload, const _Ty&, _ArgTs...oArgs){
        marshaler<true, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, const _Ty&, _ArgTs...oArgs){
        marshaler<true, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
    };


  /** marshaler specialization to process marshaling/unmarshaling a non-const reference type
  This specialization is chosen by the compiler for references types that should not be skipped
  */
    template <typename _Ty, typename ..._ArgTs> class marshaler<false, _Ty&, _ArgTs...>{
    public:
      static void marshal(payload& oPayload, const _Ty& value, _ArgTs...oArgs){
        marshaler_base<_Ty>::marshal(oPayload, value);
        marshaler<false, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, _Ty& value, _ArgTs...oArgs){
        value = marshaler_base<_Ty>::unmarshal(oPayload);
        marshaler<false, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
    };

  /** marshaler specialization to process marshaling/unmarshaling a by-value type
  This specialization is chosen by the compiler for by-value types that should not be skipped
  */
    template <typename _Ty, typename ..._ArgTs> class marshaler<false, _Ty, _ArgTs...>{
    public:
      static void marshal(payload& oPayload, const _Ty& value, _ArgTs...oArgs){
        marshaler_base<_Ty>::marshal(oPayload, value);
        marshaler<false, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
      static void unmarshal(payload& oPayload, _Ty& value, _ArgTs...oArgs){
        value = marshaler_base<_Ty>::unmarshal(oPayload);
        marshaler<false, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
      }
    };


    template <typename _DeclT, typename _CallT> class rpc_call;

    template <typename _DeclT, typename _ReturnT, typename ... _ArgTs> class rpc_call<_DeclT, _ReturnT(_ArgTs...)>{
    public:
      using return_type = _ReturnT;
      using function_type = std::function<_ReturnT(_ArgTs...)>;
      using upload_marshaler_type = marshaler<false, size_t, _ArgTs...>;
      using download_marshaler_type = marshaler<true, _ArgTs..., _ReturnT&>;
    };


  //transport should keep accepting and processing payloads until callback returns false
    using server_payload_handler_callback_type = std::function<bool(payload&)>;


    class tcp_transport{
      socket::ipv4address _Address;
      xtd::socket::ipv4_tcp_stream _Socket;
      std::thread _ServerThread;
      std::atomic<bool> _ServerExit;

      static void client_handler(socket::ipv4_tcp_stream&& oSocket, server_payload_handler_callback_type oCallback){
        auto PayloadSize = oSocket.read<size_t>();
        payload oPayload(PayloadSize, 0);
        oSocket.read<typename payload::_super_t>(oPayload);
        oCallback(oPayload);
        oSocket.write(oPayload.size());
        oSocket.write<typename payload::_super_t>(oPayload);
      }

    public:
      using pointer_type = std::shared_ptr<tcp_transport>;

      tcp_transport(const socket::ipv4address& oAddress) : _Address(oAddress), _Socket(), _ServerThread(), _ServerExit(false){}

      void start_server(server_payload_handler_callback_type oCallback){
        _ServerThread = std::thread([&, this, oCallback](){
          _Socket.bind(_Address);
          while (!_ServerExit.load()){
            _Socket.listen();
            auto oClientSocket = _Socket.accept<socket::ipv4_tcp_stream>();
            std::thread oClientThread(&tcp_transport::client_handler, std::move(oClientSocket), oCallback);
            oClientThread.detach();
          }
        });

      }
      void stop_server(){
        _ServerExit = true;
        _ServerThread.join();
      }
      void transact(payload& oPayload){
        payload oTmpPayload;
        marshaler<false, size_t>::marshal(oTmpPayload, oPayload.size());
        marshaler<false, typename payload::_super_t>::marshal(oTmpPayload, oPayload);
        _Socket.write<typename payload::_super_t>(oTmpPayload);
        _Socket.read<typename payload::_super_t>(oTmpPayload);

      }

    };

    class null_transport{
    public:
      using pointer_type = std::shared_ptr<null_transport>;

      void start_server(server_payload_handler_callback_type oCallback){

      }
      void stop_server(){
        //TODO: disconnect clients and stop the server thread
        TODO("disconnect clients and stop the server thread")
      }
      void transact(payload& oPayload){
        //TODO: send the payload to the server and return the resulting payload
        TODO("send the payload to the server and return the resulting payload")
      }
    };


    template <class _TransportT, class _DeclT, class ... _Calls> class server_impl;


    template <class _TransportT, class _DeclT> class server_impl<_TransportT, _DeclT>{

    public:

      bool call_handler(payload&){
        return true; 
      }

      typename _TransportT::pointer_type _Transport;

      server_impl(typename _TransportT::pointer_type& oTransport) : _Transport(oTransport){}

      template <typename ... _XportCtorTs> server_impl(_XportCtorTs&&...oParams) : _Transport(new _TransportT(std::forward<_XportCtorTs>(oParams)...)){}

      void start_server(){ _Transport->start_server([this](payload& oPayload)->bool{ return static_cast<_DeclT*>(this)->call_handler(oPayload); }); }

      bool invoke(payload& oPayload){
        TODO("empty out payload and return a marshaled 'invalid call' exception")
        return false;
      }

    };


    template <class _TransportT, class _DeclT, class _HeadT, class ... _TailT>
    class server_impl<_TransportT, _DeclT, _HeadT, _TailT...>
      : public server_impl<_TransportT, _DeclT, _TailT...>{
    protected:
      using _super_t = server_impl<_TransportT, _DeclT, _TailT...>;
      using function_type = typename _HeadT::function_type;
      function_type _Function;

      template <typename _CallT, typename _Param> void _attach(typename std::enable_if<std::is_same<_CallT, _HeadT>::value, _Param>::type oParam){
        _Function = oParam;
      }

      template <typename _CallT, typename _ParamT> void _attach(_ParamT oCallImpl){ _super_t::template _attach<_CallT, _ParamT>(oCallImpl); }

      template <typename ... _XportCtorTs> server_impl(_XportCtorTs&&...oParams) : _super_t(std::forward<_XportCtorTs>(oParams)...){}

    public:

      bool call_handler(payload&){
        return true;
      }

      bool invoke(payload& oPayload){
        if (oPayload.peek<decltype(typeid(_HeadT).hash_code())>() != typeid(_HeadT).hash_code()){
          return _super_t::invoke(oPayload);
        }
        TODO("unpack parameters and call _Function")
        return false;
      }


      template <typename _CallT, typename _ParamT> void attach(_ParamT oCallImpl){ _attach<_CallT, _ParamT>(oCallImpl); }

    };

    template <class _TransportT, class ... _Calls> class server : public server_impl<_TransportT, server<_TransportT, _Calls...>, _Calls...>{
      using _super_t = server_impl<_TransportT, server<_TransportT, _Calls...>, _Calls...>;
    public:
      using client_type = client<_TransportT, _Calls...>;
      template <typename ... _XportCtorTs> server(_XportCtorTs&&...oParams) : _super_t(std::forward<_XportCtorTs>(oParams)...){}

    };


    template<typename _TransportT >
    class client<_TransportT>{
    protected:
      _TransportT _Transport;

    public:
      using server_type = server<_TransportT>;
      template<typename ... _XportCtorTs> explicit client(_XportCtorTs &&...oArgs) : _Transport(std::forward<_XportCtorTs>(oArgs)...){}

      _TransportT& transport(){ return _Transport; }
      const _TransportT& transport() const { return _Transport; }

      void connect(){ _Transport.connect(); }

    };


    template<typename _TransportT, class _HeadT, class ..._TailT >
    class client<_TransportT, _HeadT, _TailT...> : public client<_TransportT, _TailT...>{
    protected:
      using _super_t = client<_TransportT, _TailT...>;

      template <typename _CallT, typename ..._ParamTs>
      typename _HeadT::return_type _call(typename std::enable_if<std::is_same<_CallT, _HeadT>::value, const std::type_info&>::type, _ParamTs&&... oParams) {
        payload oPayload;
        typename _HeadT::return_type oRet;
        _HeadT::upload_marshaler_type::marshal(oPayload, typeid(_CallT).hash_code(), std::forward<_ParamTs>(oParams)...);
        _super_t::_Transport.transact(oPayload);
        _HeadT::download_marshaler_type::unmarshal(oPayload, std::forward<_ParamTs>(oParams)..., oRet);
        return oRet;
      }

      template <typename _CallT, typename ..._ParamTs>
      typename _CallT::return_type _call(typename std::enable_if<!std::is_same<_CallT, _HeadT>::value, const std::type_info&>::type oType, _ParamTs&&... oParams) {
        return _super_t::template _call<_CallT>(oType, std::forward<_ParamTs>(oParams)...);
      }
      
    public:
      using server_type = server<_TransportT, _HeadT, _TailT...>;
      template<typename ... _XportCtorTs> explicit client(_XportCtorTs&&...oArgs) : client<_TransportT, _TailT...>(std::forward<_XportCtorTs>(oArgs)...){}



      template <typename _Ty, typename ... _ParamTs>
      typename _Ty::return_type call(_ParamTs&&...oParams) {
        return _call<_Ty>(typeid(_Ty), std::forward<_ParamTs>(oParams)...);
      }


    };

  }
}
