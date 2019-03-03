/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace rpc{
            TODO("Create generic named pipe wrapper that works on windows and linux")
#if(XTD_OS_WINDOWS & XTD_OS)
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
#endif

    }
}