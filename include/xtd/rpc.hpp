/**@file
transport neutral light weight IPC/RPC library
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once

#include <xtd/xtd.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <condition_variable>

#include <xtd/socket.hpp>
#include <xtd/concurrent/hash_map.hpp>
#include <xtd/memory.hpp>
#include <xtd/debug.hpp>
#include <xtd/windows/pipe.hpp>

namespace xtd{
  namespace rpc {
    template <typename, typename> struct rpc_call;

    template <typename, typename...> struct rpc_server;
    template <typename, typename...> struct rpc_client;

    /*
     * payload
     */
    using payload_t = std::vector<uint8_t>;

    /** reserved leading id, marshaled as the first size_t of a response payload,
    used to flag that the payload carries a marshaled exception message instead of a return value. */
    static const size_t exception_marker = (std::numeric_limits<size_t>::max)();

    template <bool _skip_in_only, typename ...> struct marshaler;

    template <bool _skip_in_only> struct marshaler<_skip_in_only> {
      static void marshal(...) {}
      static void unmarshal(...) {}
    };
    //skip const references
    template <typename _head_t, typename ... _tail_ts> struct marshaler<true, const _head_t&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    //skip byval
    template <typename _head_t, typename ... _tail_ts> struct marshaler<true, _head_t, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, const _head_t&, _tail_ts&&...oTail) {
        marshaler<true, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    //POD reference
    template <typename _head_t, typename ... _tail_ts> struct marshaler<false, _head_t&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t& oHead, _tail_ts&&...oTail) {
        static_assert(std::is_pod<_head_t>::value, "Invalid POD type for marshal");
        auto ptr = reinterpret_cast<const uint8_t*>(&oHead);
        oPayload.insert(oPayload.end(), ptr, ptr + sizeof(_head_t));
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, _head_t& oHead, _tail_ts&&...oTail) {
        static_assert(std::is_pod<_head_t>::value, "Invalid POD type for unmarshal");
        oHead = *reinterpret_cast<const _head_t*>(&oPayload.at(0));
        oPayload.erase(oPayload.begin(), oPayload.begin() + sizeof(_head_t));
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };


    //POD byval - delegate to POD reference
    template <bool _skip_in_only, typename _head_t, typename ... _tail_ts> struct marshaler<_skip_in_only, _head_t, _tail_ts...> {
      static void marshal(payload_t& oPayload, const _head_t& oHead, _tail_ts&&...oTail) {
        marshaler<_skip_in_only, _head_t&, _tail_ts...>::marshal(oPayload, oHead, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, _head_t& oHead, _tail_ts&&...oTail) {
        marshaler<false, _head_t&, _tail_ts...>::unmarshal(oPayload, oHead, std::forward<_tail_ts>(oTail)...);
      }
    };

    //string&
    template <typename ... _tail_ts> struct marshaler<false, std::string&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const std::string& oHead, _tail_ts&&...oTail) {
        marshaler<false, size_t>::marshal(oPayload, oHead.size());
        std::copy(oHead.begin(), oHead.end(), std::back_inserter(oPayload));
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, std::string& oHead, _tail_ts&&...oTail) {
        size_t len;
        marshaler<false, size_t>::unmarshal(oPayload, len);
        oHead.reserve(len);
        oHead.resize(0, 0);
        std::copy(oPayload.begin(), oPayload.begin() + len, std::back_inserter(oHead));
        oPayload.erase(oPayload.begin(), oPayload.begin() + len);
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };
    //vector
    template <typename _item_t, typename ... _tail_ts> struct marshaler<false, std::vector<_item_t>&, _tail_ts...> {
      static void marshal(payload_t& oPayload, const std::vector<_item_t>& oHead, _tail_ts&&...oTail) {
        marshaler<false, size_t>::marshal(oPayload, oHead.size());
        for (const auto & oItem : oHead) marshaler<false, _item_t&>::marshal(oPayload, oItem);
        marshaler<false, _tail_ts...>::marshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
      static void unmarshal(payload_t& oPayload, std::vector<_item_t>& oHead, _tail_ts&&...oTail) {
        size_t len;
        marshaler<false, size_t>::unmarshal(oPayload, len);
        oHead.reserve(len);
        oHead.clear();
        for (; len > 0; --len) {
          _item_t oItem;
          marshaler<false, _item_t&>::unmarshal(oPayload, oItem);
          oHead.push_back(oItem);
        }
        marshaler<false, _tail_ts...>::unmarshal(oPayload, std::forward<_tail_ts>(oTail)...);
      }
    };

    struct payload : std::vector<uint8_t> {
      using _super_t = std::vector<uint8_t>;
      payload() : vector(sizeof(size_t), 0) {}
      template <typename _ty> _ty peek() const {
        static_assert(std::is_pod<_ty>::value, "Invalid POD type for peek");
        return *reinterpret_cast<const _ty*>(&at(0));
      }
      void embed_length() {
        auto * iLen = reinterpret_cast<size_t*>(&at(0));
        *iLen = size();
      }
    };

    /** tcp/ip transport
    */
    class tcp_transport {
      socket::ipv4address _address;
      xtd::socket::ipv4_tcp_stream _socket;
      std::unique_ptr<std::thread> _server_thread;
      std::atomic<bool> _stop_server;
      std::vector<std::thread> _clients;
      bool _client_connected;

    public:
      using pointer_type = std::shared_ptr<tcp_transport>;

      tcp_transport(const socket::ipv4address& oAddress)
        : _address(oAddress), _socket(), _server_thread(), _stop_server(false), _clients(), _client_connected(false) {}

      ~tcp_transport() {
        if (_server_thread) stop_server();
      }

      template <typename _server_t> void start_server(_server_t& oServer) {
        _stop_server = false;
        std::shared_ptr<std::promise<void>> oServerStarted(new std::promise<void>);
        _server_thread = xtd::make_unique<std::thread>([this, &oServer, oServerStarted] {
          _socket.bind(_address);
          _socket.listen();
          oServerStarted->set_value();
          while (!_stop_server) {
            std::shared_ptr<xtd::socket::ipv4_tcp_stream> oClientSocket;
            try {
              oClientSocket = std::make_shared<xtd::socket::ipv4_tcp_stream>(_socket.accept<xtd::socket::ipv4_tcp_stream>());
            } catch (const xtd::exception&) {
              break; // listen socket closed by stop_server() (or accept failure); leave the accept loop
            }
            if (_stop_server) break;
            _clients.emplace_back([this, oClientSocket, &oServer] {
              bool ExitThread = false;
              bool bResponseReady = false;
              payload oPayload;
              oClientSocket->onError.connect([&ExitThread] {
                ERR("Socket error");
                ExitThread = true;
              });
              oClientSocket->onRead.connect([&] {
                oClientSocket->read<payload::_super_t>(oPayload);
                size_t iPayloadSize;
                marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize); // strip length header, leaving [call-id][args]
                oServer.invoke(oPayload);
                bResponseReady = true;
              });
              oClientSocket->onWrite.connect([&] {
                if (bResponseReady) {
                  oClientSocket->write<payload::_super_t>(oPayload);
                  bResponseReady = false;
                  oPayload = payload();
                }
              });
              try {
                while (!_stop_server && !ExitThread) {
                  oClientSocket->select(250);
                }
              } catch (const xtd::exception&) {
                // client disconnected or socket error; end this worker cleanly
              }
            });
          }
          for (auto& oClientThread : _clients) {
            if (oClientThread.joinable()) oClientThread.join();
          }
          _clients.clear();
        });
        oServerStarted->get_future().get();
      }
      void stop_server() {
        _stop_server = true;
        _socket.close();
        if (_server_thread && _server_thread->joinable()) {
          _server_thread->join();
        }
      }

      void transact(payload& oPayload) {
        if (!_client_connected) {
          _socket.connect(_address);
          _client_connected = true;
        }
        _socket.write<typename payload::_super_t>(oPayload);
        _socket.read<typename payload::_super_t>(oPayload);
        size_t iPayloadSize;
        marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize); // strip length header, leaving [call-id][return]
      }

    };

    /*
     * anonymous_pipe_transport
     */
    struct anonymous_pipe_transport {
      using pointer_type = std::shared_ptr<anonymous_pipe_transport>;

      ~anonymous_pipe_transport() {
        if (_running) stop_server();
      }

      anonymous_pipe_transport(xtd::windows::pipe::shared_ptr& oServerPipe, xtd::windows::pipe::shared_ptr& oClientPipe) : _server_pipe{ oServerPipe }, _client_pipe{ oClientPipe }{}

      void stop_server() {
        if (!_running) throw xtd::exception(here(), "Pipe server not running");
        _running = false;
        _stop_server_thread->set_value();
        _server_thread->join();
      }

      void transact(payload& oPayload) {
        oPayload.embed_length();
        _server_pipe->write<uint8_t>(oPayload);
        size_t iPayloadSize;
        while (!_client_pipe->peek(iPayloadSize)) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        oPayload.resize(iPayloadSize);
        _client_pipe->read(oPayload);
        marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize);
      }

      template <typename _server_t> void start_server(_server_t& oServer) {
        if (_running) throw xtd::exception(here(), "Pipe server already running");
        _running = true;
        _stop_server_thread = std::make_unique<std::promise<void>>();
        _server_thread = std::make_unique<std::thread>([this, &oServer]() {
          auto oFuture = _stop_server_thread->get_future();

          size_t iPayloadSize;
          payload oPayload;
          for (; std::future_status::timeout == oFuture.wait_for(std::chrono::milliseconds(1));) {
            if ((iPayloadSize = _server_pipe->bytes_available()) < sizeof(size_t)) continue;
            oPayload.resize(iPayloadSize);
            _server_pipe->read(oPayload);
            marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize);
            oServer.invoke(oPayload);
            oPayload.embed_length();
            _client_pipe->write<uint8_t>(oPayload);
          }

        });
      }
    private:
      xtd::windows::pipe::shared_ptr _server_pipe;
      xtd::windows::pipe::shared_ptr _client_pipe;
      std::unique_ptr<std::thread> _server_thread;
      std::unique_ptr<std::promise<void>> _stop_server_thread;
      bool _running = false;
    };

    /*
     * null_transport - in-process loopback (no IPC), useful for unit-testing rpc logic
     */
    struct null_transport {
      using pointer_type = std::shared_ptr<null_transport>;
      struct shared_state { std::function<bool(payload&)> handler; };

      null_transport(std::shared_ptr<shared_state> oState) : _state(oState) {}

      template <typename _server_t> void start_server(_server_t& oServer) {
        _state->handler = [&oServer](payload& oPayload) -> bool {
          size_t iPayloadSize;
          marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize); // strip length header, leaving [call-id][args]
          return oServer.invoke(oPayload);
        };
      }
      void stop_server() { _state->handler = nullptr; }

      void transact(payload& oPayload) {
        if (!_state->handler) throw xtd::exception(here(), "null_transport server not running");
        oPayload.embed_length();
        _state->handler(oPayload);
        size_t iPayloadSize;
        marshaler<false, size_t>::unmarshal(oPayload, iPayloadSize); // strip response length header
      }
    private:
      std::shared_ptr<shared_state> _state;
    };


    /*
     * rpc_client
     */
    template <typename _transport_t> struct rpc_client < _transport_t> : _transport_t {
      template <typename ... _arg_ts> rpc_client(_arg_ts&&...oArgs) : _transport_t(std::forward<_arg_ts>(oArgs)...) {}
      template <typename _impl_t> using client_from_impl = rpc_client < _transport_t>;
    };

    template <typename _transport_t, typename _head_t, typename ... _tail_ts> struct rpc_client<_transport_t, _head_t, _tail_ts...> : rpc_client<_transport_t, _tail_ts...> {
      using transport_type = _transport_t;
      using _super_t = rpc_client<_transport_t, _tail_ts...>;
      using _this_t = rpc_client<transport_type, _head_t, _tail_ts...>;
      template <typename ... _arg_ts> rpc_client(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}

      template <typename _impl_t> using client_from_impl = typename std::conditional< std::is_same<_impl_t, _head_t>::value, _this_t, typename _super_t::template client_from_impl<_impl_t>>::type;

      template <typename _ty, typename ... _arg_ts> typename _ty::return_type call(_arg_ts&&...oArgs) {
        return static_cast<client_from_impl<_ty>&>(*this).template _call<typename _ty::return_type>(std::forward<_arg_ts>(oArgs)...);
      }

    protected:
      template <typename, typename...> friend struct rpc_client;

      template <typename _return_t, typename ... _arg_ts> _return_t _call(_arg_ts&&...oArgs) {
        payload oPayload;
        marshaler<false, size_t>::marshal(oPayload, typeid(_head_t).hash_code());
        marshaler<false, _arg_ts...>::marshal(oPayload, std::forward<_arg_ts>(oArgs)...);
        transport_type::transact(oPayload);
        if (exception_marker == oPayload.peek<size_t>()) {
          size_t iMarker;
          marshaler<false, size_t>::unmarshal(oPayload, iMarker);
          std::string sWhat;
          marshaler<false, std::string&>::unmarshal(oPayload, sWhat);
          throw xtd::exception(here(), sWhat);
        }
        _return_t oRet;
        size_t icalltypeid;
        marshaler<false, size_t>::unmarshal(oPayload, icalltypeid);
        marshaler<false, _return_t&>::unmarshal(oPayload, oRet);
        marshaler<true, _arg_ts...>::unmarshal(oPayload, std::forward<_arg_ts>(oArgs)...);
        return oRet;
      }

    };



    namespace _ {
      /*
       * invoker
       */
      template <typename _function_t, typename _return_t, typename ... _fnarg_ts> struct invoker;

      template <typename _function_t, typename _return_t> struct invoker<_function_t, _return_t> {

        template <typename ... _arg_ts>
        static bool invoke(_function_t& oFN, payload& oPayload, _arg_ts&&...oArgs) {
          _return_t oRet = oFN(std::forward<_arg_ts>(oArgs)...);
          oPayload = payload();
          marshaler<false, size_t>::marshal(oPayload, typeid(typename _function_t::impl_type).hash_code());
          marshaler<false, _return_t>::marshal(oPayload, oRet);
          return true;
        }
      };

      template <typename _function_t, typename _return_t, typename _head_t, typename ... _tail_ts> struct invoker<_function_t, _return_t, _head_t, _tail_ts...> {

        template <typename ... _arg_ts>
        static bool invoke(_function_t& oFN, payload& oPayload, _arg_ts&&...oArgs) {
          typename std::remove_reference<_head_t>::type oHead;
          marshaler<false, _head_t>::unmarshal(oPayload, oHead);
          return invoker<_function_t, _return_t, _tail_ts...>::template invoke(oFN, oPayload, std::forward<_arg_ts>(oArgs)..., oHead);
        }
      };
    }

    /*
     * rpc_server
     */

    template <typename _transport_t> struct rpc_server < _transport_t> : _transport_t {
      template <typename _impl_t> using server_from_impl = rpc_server < _transport_t>;

      template <typename ... _arg_ts> rpc_server(_arg_ts&&...oArgs) : _transport_t(std::forward<_arg_ts>(oArgs)...) {}
    protected:
      bool invoke(payload& oPayload) {
        oPayload = payload();
        marshaler<false, size_t>::marshal(oPayload, exception_marker);
        std::string sWhat("xtd::rpc: no handler for call");
        marshaler<false, std::string&>::marshal(oPayload, sWhat);
        return true;
      }
    };

    template <typename _transport_t, typename _head_t, typename ... _tail_ts> struct rpc_server<_transport_t, _head_t, _tail_ts...> : rpc_server<_transport_t, _tail_ts...> {
      using transport_type = _transport_t;
      using _this_t = rpc_server<_transport_t, _head_t, _tail_ts...>;
      using _super_t = rpc_server<_transport_t, _tail_ts...>;
      using client_type = rpc_client<transport_type, _head_t, _tail_ts...>;
      using call_type = _head_t;
      template <typename _impl_t> using server_from_impl = typename std::conditional< std::is_same<_impl_t, _head_t>::value, _this_t, typename _super_t::template server_from_impl<_impl_t>>::type;

      template <typename ... _arg_ts> rpc_server(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}

      template <typename ... _arg_ts> void attach(_arg_ts&&...oArgs) {
        _call.attach(std::forward<_arg_ts>(oArgs)...);
      }

      template <typename _impl_t> server_from_impl<_impl_t>& get() { return static_cast<server_from_impl<_impl_t>&>(*this); }

      void start_server() { transport_type::template start_server<_this_t>(*this); }
      void stop_server() { transport_type::stop_server(); }

    protected:
      friend class tcp_transport;
      friend struct anonymous_pipe_transport;
      friend struct null_transport;
      call_type _call;

      bool invoke(payload& oPayload) {
        if (typeid(_head_t).hash_code() != oPayload.peek<size_t>()) return _super_t::invoke(oPayload);
        size_t iCallID;
        marshaler<false, size_t>::unmarshal(oPayload, iCallID);
        try {
          return _call.invoke(oPayload);
        } catch (const std::exception& ex) {
          oPayload = payload();
          marshaler<false, size_t>::marshal(oPayload, exception_marker);
          std::string sWhat(ex.what());
          marshaler<false, std::string&>::marshal(oPayload, sWhat);
          return true;
        }
      }
    };

    /*
   * rpc_call
   */

    template <typename _impl_t, typename _return_t, typename ... _fnarg_ts> struct rpc_call<_impl_t, _return_t(_fnarg_ts...)> : std::function<_return_t(_fnarg_ts...)> {
      using _super_t = std::function<_return_t(_fnarg_ts...)>;
      using _my_t = rpc_call<_impl_t, _return_t(_fnarg_ts...)>;
      using return_type = _return_t;
      using impl_type = _impl_t;
      template <typename ... _arg_ts> rpc_call(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...) {}


      template <typename ... _arg_ts> void attach(_arg_ts&&...oArgs) {
        *this = _super_t(std::forward<_arg_ts>(oArgs)...);
      }

    protected:
      template <typename, typename...> friend struct rpc_server;

      bool invoke(payload& oPayload) {
        return _::invoker<_my_t, _return_t, _fnarg_ts...>::invoke(*this, oPayload);
      }
    };

  }
}
