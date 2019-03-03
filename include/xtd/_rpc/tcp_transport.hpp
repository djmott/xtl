/**@file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

namespace xtd{
    namespace rpc{
        
    /** tcp/ip transport
    */
    class tcp_transport {
      socket::ipv4address _address;
      xtd::socket::ipv4_tcp_stream _socket;
      std::unique_ptr<std::thread> _server_thread;
      bool _stop_server;
      xtd::concurrent::hash_map<std::thread::id, std::thread> _clients;
      bool _client_connected;

    public:
      using pointer_type = std::shared_ptr<tcp_transport>;

      tcp_transport(const tcp_transport&) = delete;

      tcp_transport(const socket::ipv4address& oAddress)
        : _address(oAddress), _socket(), _server_thread(), _stop_server(false), _clients(), _client_connected(false) {}

      template <typename _server_t> void start_server(_server_t& oServer) {
        _stop_server = false;
        std::shared_ptr<std::promise<void>> oServerStarted(new std::promise<void>);
        _server_thread = xtd::make_unique<std::thread>([&oServer, &oServerStarted, this] {
          oServerStarted->set_value();
          _socket.bind(_address);
          bool ExitThread = false;
          payload oPayload;
          while (!_stop_server && !ExitThread) {
            _socket.listen();
            auto oClient = _socket.accept<xtd::socket::ipv4_tcp_stream>();
            std::shared_ptr<xtd::socket::ipv4_tcp_stream> oClientSocket(new xtd::socket::ipv4_tcp_stream(std::move(oClient)));
            std::thread oClientThread([&oServer, &oClientSocket, &ExitThread, &oPayload, this]() {
              oClientSocket->onError.connect([&ExitThread]() {
                throw xtd::socket::exception(here(), "socket connect failure");
                ExitThread = true;
              });
              oClientSocket->onRead.connect([&oServer, &oClientSocket, &oPayload, this]() {
                oClientSocket->read<payload::_super_t>(oPayload);
                oServer.invoke(oPayload);
              });
              oClientSocket->onWrite.connect([&oServer, &oClientSocket, &oPayload, this]() {
                if (oPayload.size()) {
                  oClientSocket->write<payload::_super_t>(oPayload);
                }
              });
              payload oPayload;
              for (;;) {
                oClientSocket->select(250);
              }
            });
            oClientThread.detach();
          }
        });
        oServerStarted->get_future().get();
      }
      void stop_server() {
        //TODO: implement stop_server
        TODO("implement stop_server")
      }

      void transact(payload& oPayload) {
        if (!_client_connected) {
          _socket.connect(_address);
          _client_connected = true;
        }
        _socket.write<typename payload::_super_t>(oPayload);
        _socket.read<typename payload::_super_t>(oPayload);

      }

    };

    }
}