/** @file
Light weight transport neutral RPC client and server
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

namespace xtd {
  namespace rpc {

    class communication_exception : public xtd::exception{
    public:
      communication_exception(const source_location& loc, const xtd::string& sWhat) : xtd::exception(loc, sWhat){}
      communication_exception(const communication_exception& ex) : xtd::exception(ex){}
      communication_exception(communication_exception&& ex) : xtd::exception(std::move(ex)){}
    };

    class protocol_exception : public xtd::exception {
    public:
      protocol_exception(const source_location& loc, const xtd::string& sWhat) : xtd::exception(loc, sWhat) {}
      protocol_exception(const protocol_exception& ex) : xtd::exception(ex) {}
      protocol_exception(protocol_exception&& ex) : xtd::exception(std::move(ex)) {}
    };

    template <typename _Ty> class call;

    namespace _ {

      using payload_type = std::vector<uint8_t>;


      template <typename...> class marshaler_base;

      template <> class marshaler_base<> {
      public:
        static void marshal(payload_type&) {}
        static void unmarshal(payload_type&) {}
      };


      template <typename _Ty>
      class marshaler_base<_Ty> {
      public:
        static void marshal(payload_type& oPayload, const _Ty& val) {
          static_assert(std::is_pod<_Ty>::value, "Invalid specialization");
          oPayload.insert(oPayload.end(), reinterpret_cast<const uint8_t*>(&val), sizeof(_Ty) + reinterpret_cast<const uint8_t*>(&val));
        }
        static _Ty unmarshal(payload_type& oPayload){
          static_assert(std::is_pod<_Ty>::value, "Invalid specialization");
          XTD_ASSERT(oPayload.size() >= sizeof(_Ty));
          _Ty oRet(*reinterpret_cast<_Ty*>(&oPayload[0]));
          oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(_Ty));
          return oRet;
        }
      };

#if (XTD_HAS_UUID)
      template <>
      class marshaler_base<unique_id&> {
      public:
        static void marshal(payload_type &oPayload, const unique_id &val) {
          oPayload.insert(oPayload.end(), reinterpret_cast<const uint8_t *>(&val),
                          sizeof(unique_id) + reinterpret_cast<const uint8_t *>(&val));
        }

        static unique_id unmarshal(payload_type &oPayload) {
          XTD_ASSERT(oPayload.size() >= sizeof(unique_id));
          unique_id oRet(*reinterpret_cast<unique_id *>(&oPayload[0]));
          oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(unique_id));
          return oRet;
        }
      };
#endif


      template <typename _Ty, size_t _Len>
      class marshaler_base<_Ty(&)[_Len] > {
      public:
        static void marshal(payload_type& oPayload, const _Ty(&val)[_Len]) {
          marshaler_base<size_t>::marshal(oPayload, _Len);
          for (const auto & oItem : val) marshaler_base<_Ty>::marshal(oPayload, oItem);
        }
      };


      template <typename _Ty>
      class marshaler_base<std::vector < _Ty>> {
      public:
        static void marshal(payload_type& oPayload, const std::vector<_Ty>& val) {
          marshaler_base<size_t>::marshal(oPayload, val.size());
          for (const auto & oItem : val) marshaler_base<_Ty>::marshal(oPayload, oItem);
        }
        static std::vector < _Ty> unmarshal(payload_type& oPayload) {
          std::vector < _Ty> oRet;
          auto iSize = marshaler_base<std::size_t>::unmarshal(oPayload);
          for (std::size_t i = 0; i < iSize; i++) {
            oRet.push_back(marshaler_base<_Ty>::unmarshal(oPayload));
          }
          return oRet;
        }
      };


      template <>
      class marshaler_base<std::string> {
      public:
        static void marshal(payload_type & oPayload, const std::string& val) {
          marshaler_base<std::size_t>::marshal(oPayload, val.size());
          oPayload.insert(oPayload.end(), val.cbegin(), val.cend());
        }
        static std::string unmarshal(payload_type& oPayload) {

          auto iSize = marshaler_base<size_t>::unmarshal(oPayload);
          std::string oRet(oPayload.begin(), oPayload.begin() + iSize);
          oPayload.erase(oPayload.begin(), oPayload.begin() + iSize);
          return oRet;
        }
      };

      template <bool _SkipByVal, typename ...> class marshaler;

      template <bool _SkipByVal> class marshaler<_SkipByVal> {
      public:
        static void marshal(payload_type&) {}
        static void unmarshal(payload_type&) {}
      };

      template <typename _Ty, typename..._ArgTs> class marshaler<true, const _Ty&, _ArgTs...> {
      public:
        static void marshal(payload_type& oPayload, const _Ty&, _ArgTs...oArgs) {
          marshaler<true, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
        static void unmarshal(payload_type& oPayload, const _Ty&, _ArgTs...oArgs) {
          marshaler<true, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
      };


      template <typename _Ty, typename ..._ArgTs> class marshaler<true, _Ty, _ArgTs...> {
      public:
        static void marshal(payload_type& oPayload, const _Ty&, _ArgTs...oArgs) {
          marshaler<true, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
        static void unmarshal(payload_type& oPayload, const _Ty&, _ArgTs...oArgs) {
          marshaler<true, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
      };


      template <typename _Ty, typename ..._ArgTs> class marshaler<false, _Ty&, _ArgTs...> {
      public:
        static void marshal(payload_type& oPayload, const _Ty& value, _ArgTs...oArgs) {
          marshaler_base<_Ty>::marshal(oPayload, value);
          marshaler<false, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
        static void unmarshal(payload_type& oPayload, _Ty& value, _ArgTs...oArgs) {
          value = marshaler_base<_Ty>::unmarshal(oPayload);
          marshaler<false, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
      };

      template <typename _Ty, typename ..._ArgTs> class marshaler<false, _Ty, _ArgTs...> {
      public:
        static void marshal(payload_type& oPayload, const _Ty& value, _ArgTs...oArgs) {
          marshaler_base<_Ty>::marshal(oPayload, value);
          marshaler<false, _ArgTs...>::marshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
        static void unmarshal(payload_type& oPayload, _Ty& value, _ArgTs...oArgs) {
          value = marshaler_base<_Ty>::unmarshal(oPayload);
          marshaler<false, _ArgTs...>::unmarshal(oPayload, std::forward<_ArgTs>(oArgs)...);
        }
      };

      class invoker_base {
      public:
        using pointer_type = std::shared_ptr<invoker_base>;
        using vector_type = std::vector<pointer_type>;
        virtual void invoke(payload_type&) = 0;
      };

      template <int Index, typename _ReturnT, typename ... _ArgTs> class invoke_helper;

      template <typename _ReturnT, typename ... _ArgTs>
      class invoker : public invoker_base {
      public:
        using handler_type = std::function<_ReturnT(_ArgTs...)>;
        handler_type _Handler;
        virtual void invoke(payload_type& oPayload) override {
          invoke_helper<sizeof...(_ArgTs), _ReturnT, _ArgTs...>::invoke(oPayload, _Handler);
        }
      };

      template <int Index, typename _ReturnT, typename _HeadT, typename ..._TailTs> 
      class invoke_helper<Index, _ReturnT, _HeadT, _TailTs...> {
      public:
        template <typename _HandlerT, typename ... _ArgTs>
        static void invoke(payload_type& oPayload, const _HandlerT& oHandler, _ArgTs&&...oArgs) {
          auto oParam = marshaler_base<_HeadT>::unmarshal(oPayload);
          invoke_helper<Index - 1, _ReturnT, _TailTs...>::invoke(oPayload, oHandler, std::forward<_ArgTs>(oArgs)..., oParam);
          marshaler<true, _HeadT>::marshal(oPayload, oParam);
        }
      };

      template <typename _ReturnT>
      class invoke_helper<0, _ReturnT> {
      public:
        template <typename _HandlerT, typename ... _ArgTs>
        static void invoke(payload_type& oPayload, const _HandlerT& oHandler, _ArgTs&&...oArgs) {
          XTD_ASSERT(0 == oPayload.size());
          _ReturnT oRet = oHandler(std::forward<_ArgTs>(oArgs)...);
          marshaler_base<_ReturnT>::marshal(oPayload, oRet);
        }
      };

      template <int _Counter, typename...> class IndexOfTypeID;

      template <int _Counter> class IndexOfTypeID<_Counter> {
      public:
        static int Value(std::size_t) { return -1; }
      };

      template <int _Counter, typename _HeadT, typename ... _TailT> class IndexOfTypeID<_Counter, _HeadT, _TailT...> {
      public:
        static int Value(std::size_t iHash) { return (typeid(_HeadT).hash_code() == iHash) ? _Counter : IndexOfTypeID<1 + _Counter, _TailT...>::Value(iHash); }
      };

      template <typename ...> class InvokerVector;

      template <> class InvokerVector<> {
      public:
        static void Create(invoker_base::vector_type&) {}
      };

      template <typename _HeadT, typename ... _TailT>
      class InvokerVector<_HeadT, _TailT...> {
      public:
        static void Create(typename invoker_base::vector_type& oItems) {
          oItems.emplace_back(typename invoker_base::pointer_type(new typename _HeadT::invoker_type));
          InvokerVector<_TailT...>::Create(oItems);
        }
      };
    }

    template <typename _ReturnT, typename ... _ArgsT> 
    class call<_ReturnT(_ArgsT...)> {
    public:
      using return_type = _ReturnT;
      using handler_type = std::function<_ReturnT(_ArgsT...)>;
      using invoker_type = _::invoker<_ReturnT, _ArgsT...>;
    };

    class disconnect : public call<int()> {};

    class null_transport{
    public:
      using client_type = null_transport;
      template <typename ... _ArgTs> null_transport(_ArgTs&&...oArgs){}
      virtual void start_server(){}
      virtual void stop_server(){}
      virtual void handle_client(client_type&& oClient){}
      virtual void connect(){}
      template <typename _Ty>
      void read(_Ty&){}
      template <typename _Ty>
      void write(const _Ty&){}
    };

    class ipv4_tcp_transport : public xtd::socket::ipv4_tcp_stream{
    public:
      using client_type = xtd::socket::ipv4_tcp_stream;
      using _super_t = xtd::socket::ipv4_tcp_stream;

      ipv4_tcp_transport(const char * sIP, uint16_t iPort) : _Address(sIP, iPort) {}
      virtual ~ipv4_tcp_transport() = default;

      virtual void start_server() {
        _super_t::bind(_Address);
        for (;;) {
          _super_t::listen();
          handle_client(std::move(_super_t::accept<_super_t>()));
        }
      }

      virtual void stop_server() { _super_t::close(); }

      virtual void handle_client(client_type&&) = 0;

      virtual void connect() { _super_t::connect(_Address); }

    protected:
      _super_t::address_type _Address;
    };


    template <typename _TransportT, bool _MarshalConst, bool _UnmarshalConst, typename ... _CallTs>
    class server : private _TransportT {
    public:
      using transport_type = _TransportT;

      template <typename ... _TransportArgs>
      server(_TransportArgs&&...oArgs) : _TransportT(std::forward<_TransportArgs>(oArgs)...) {
        _::InvokerVector<_CallTs...>::Create(_Invokers);
      }
      server() = delete;
      server(const server&) = delete;
      virtual ~server() = default;

      template <typename _Ty, typename _ArgT> void Attach(_ArgT&& oArg) {
        static const int index = xtd::tuple<_CallTs...>::template index_of<_Ty>::value;
        auto pInvoker = static_cast<typename _Ty::invoker_type*>(_Invokers[index].get());
        pInvoker->_Handler = typename _Ty::handler_type(std::forward<_ArgT>(oArg));
      }

      void start_server() override { _TransportT::start_server(); }

      void stop_server() override { _TransportT::stop_server(); }

      void handle_client(typename _TransportT::client_type&& oClient) override {
        _::payload_type oPayload;
        try{
          for (;;) {
            oClient.read(oPayload);
            if (oPayload.size() < sizeof(std::size_t)) {
              ERR("Invalid payload");
              TODO("return error to client");
              break;
            }
            auto iCallIndex = _::marshaler_base<std::size_t>::unmarshal(oPayload);
            if ((std::size_t) - 1 == iCallIndex) {
              ERR("Invalid call type");
              TODO("return error to client");
              break;
            }
            if (iCallIndex == xtd::tuple<_CallTs...>::template index_of<disconnect>::value) break;
            auto & oInvoker = _Invokers[iCallIndex];
            oInvoker->invoke(oPayload);
            oPayload.insert(oPayload.begin(), reinterpret_cast<const uint8_t*>(&iCallIndex), reinterpret_cast<const uint8_t*>(&iCallIndex) + sizeof(iCallIndex));
            oClient.write(oPayload);
          }
        }
        catch (const xtd::socket::exception& ex){
          ERR("A socket communication exception occurred: ", ex.what());
        }
      }

    protected:
      _::invoker_base::vector_type _Invokers;
    };

    template <typename _TransportT, bool _MarshalConst, bool _UnmarshalConst, typename ... _CallTs>
    class client : private _TransportT {
    public:
      using transport_type = _TransportT;

      template <typename ... _TransportArgs>
      client(_TransportArgs...oArgs) : _TransportT(oArgs...) {}

      client() = delete;
      client(const client&) = delete;

      template <typename _CallT, typename ... _CallArgsT>
      typename _CallT::return_type call(_CallArgsT&&...oArgs) {
        _::payload_type oPayload;
        auto iCallIndex = IndexOf(typeid(_CallT), typeid(_CallTs)...);
        _::marshaler<false,  size_t, _CallArgsT...>::marshal(oPayload, iCallIndex, std::forward<_CallArgsT>(oArgs)...);
        _TransportT::write(oPayload);
        oPayload.clear();
        _TransportT::read(oPayload);
        auto iCallID = _::marshaler_base<std::size_t>::unmarshal(oPayload);
        if (iCallID != iCallIndex){
          TODO("Handle error");
        }
        auto oRet = _::marshaler_base<typename _CallT::return_type>::unmarshal(oPayload);
        _::marshaler<true, _CallArgsT...>::unmarshal(oPayload, std::forward<_CallArgsT>(oArgs)...);
        return oRet;
      }

      void connect() override {
        _TransportT::connect();
      }


    protected:

      template <typename ... _TailT>
      size_t IndexOf(const std::type_info& src, const std::type_info& head, _TailT&&...oTail) const {
        if (src == head) return 0;
        auto iRet = IndexOf(src, std::forward<_TailT>(oTail)...);
        return ((std::size_t)-1 == iRet) ? -1 : 1 + iRet;
      }

      size_t IndexOf(const std::type_info&) const {
        return -1;
      }

      void handle_client(typename transport_type::client_type&&) override {}
    };

    template <typename _TransportT, typename ... _CallTs> class contract {
    public:
      using client_type = rpc::client<_TransportT, true, false, disconnect, _CallTs...>;
      using server_type = rpc::server<_TransportT, false, true, disconnect, _CallTs...>;
    };
  }
}


#if !defined(__TEST_RPC_SERVER__)
#define __TEST_RPC_SERVER__ 0
#endif


#if !defined(__TEST_RPC_CLIENT__)
#define __TEST_RPC_CLIENT__ 0
#endif

#if __TEST_RPC_SERVER__ || __TEST_RPC_CLIENT__

namespace rpctest {
using namespace xtd::rpc;
class Add : public call<int(int, int)> {};
class Echo : public call<std::string(std::string)> {};
class Average : public call<double(std::vector<double>)> {};

using contract_type = contract<ipv4_tcp_transport, Add, Echo, Average>;
using server_type = contract_type::server;
using client_type = contract_type::client;

}

#endif

#if 0

int main() {
using namespace rpctest;
server_type oServer("127.0.0.1", 9494);

oServer.Attach<Add>([](int a, int b) { return a+b; });
oServer.Attach<Echo>([](const std::string& sval) -> std::string { return std::string(sval); });
oServer.Attach<Average>([](const std::vector<double>& oVals) -> double {
double dRet = 0;
for (auto & oVal : oVals) { dRet += oVal; }
dRet /= oVals.size();
return dRet;
});
oServer.StartServer();
}

#endif

#if 0
int main(){
using namespace rpctest;
Sleep(2000);
client_type oClient("127.0.0.1", 9494);
oClient.Connect();
for (int i=0 ; i<1000 ; ++i){
RPC_DBG("Calling Echo ", xtd::cstring::Base<10>(i));
std::string sParam = "Hello!";
auto sRet = oClient.call<Echo>(sParam);
XTD_ASSERT(sParam == sRet);
}
XTD_ASSERT(3 == oClient.call<Add>(1, 2));
}
#endif
