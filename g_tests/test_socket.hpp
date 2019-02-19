/** @file
xtd::socket system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <future>
#include <thread>

#include <xtd/socket.hpp>

TEST(test_socket, ipv4_initialization){

  ASSERT_NO_THROW(xtd::socket::ipv4_tcp_stream oSocket);
}

TEST(test_socket, socket_options){
  xtd::socket::ipv4_tcp_stream oSocket;
  ASSERT_NO_THROW(oSocket.keep_alive());
  ASSERT_NO_THROW(oSocket.keep_alive(true));
  ASSERT_NO_THROW(oSocket.keep_alive(false));
}

TEST(test_socket, ip_options){
  xtd::socket::ipv4_tcp_stream oSocket;
#if (XTD_OS_WINDOWS & XTD_OS)
  ASSERT_NO_THROW(oSocket.dont_fragment());
  ASSERT_NO_THROW(oSocket.dont_fragment(true));
  ASSERT_NO_THROW(oSocket.dont_fragment(false));
#endif
}

TEST(test_socket, tcp_options){
  xtd::socket::ipv4_tcp_stream oSocket;
  ASSERT_NO_THROW(oSocket.no_delay());
  ASSERT_NO_THROW(oSocket.no_delay(true));
  ASSERT_NO_THROW(oSocket.no_delay(false));
}

TEST(test_socket, udp_options){
  xtd::socket::ipv4_udp_socket oSocket;
#if (XTD_OS_WINDOWS & XTD_OS)
  ASSERT_NO_THROW(oSocket.no_checksum());
  ASSERT_NO_THROW(oSocket.no_checksum(true));
  ASSERT_NO_THROW(oSocket.no_checksum(false));
#endif
}

TEST(test_socket, ipv4_bind_and_listen){
  xtd::socket::ipv4_tcp_stream oSocket;
  ASSERT_NO_THROW(oSocket.bind(xtd::socket::ipv4address("0.0.0.0", 12345)));
  ASSERT_NO_THROW(oSocket.listen());
}


TEST(test_socket, ipv4_connect){

  auto do_ipv4_connect_test = []()->void {
    static constexpr uint16_t iPort = 9977;
    std::promise<bool> oPromise;
    std::thread oServerThread([&oPromise](uint16_t iPort) {
      try {
        xtd::socket::ipv4_tcp_stream oServer;
        oServer.bind(xtd::socket::ipv4address("0.0.0.0", iPort));
        oServer.listen();
        auto oClientConnection = oServer.accept<xtd::socket::ipv4_tcp_stream>();
        oClientConnection.close();
        oPromise.set_value(true);
      }
      catch (...) {
        oPromise.set_exception(std::current_exception());
      }
    }, iPort);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    xtd::socket::ipv4_tcp_stream oClient;
    oClient.connect(xtd::socket::ipv4address("127.0.0.1", iPort));
    oClient.close();
    oServerThread.join();
    ASSERT_TRUE(oPromise.get_future().get());
  };


  ASSERT_NO_THROW(do_ipv4_connect_test());

}



TEST(test_socket, ipv4_stream){

  auto do_ipv4_write_server_test = [](){

    static constexpr uint16_t iPort = 8855;
    std::promise<int> oPromise;
    std::thread oServerThread([&oPromise](uint16_t iPort){
      try{
        xtd::socket::ipv4_tcp_stream oServer;
        oServer.bind(xtd::socket::ipv4address("0.0.0.0", iPort));
        oServer.listen();
        auto oClientConnection = oServer.accept<xtd::socket::ipv4_tcp_stream>();
        auto iRet = oClientConnection.read<int>();
        oClientConnection.close();
        oPromise.set_value(iRet);
      }
      catch (...){
        oPromise.set_exception(std::current_exception());
      }
    }, iPort);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    xtd::socket::ipv4_tcp_stream oClient;
    oClient.connect(xtd::socket::ipv4address("127.0.0.1", iPort));
    oClient.write(12345);
    oClient.close();

    oServerThread.join();
    ASSERT_EQ(oPromise.get_future().get(), 12345);
  };

  ASSERT_NO_THROW(do_ipv4_write_server_test());
}


