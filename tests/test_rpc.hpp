/** @file
xtd::rpc system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/rpc.hpp>

#include <string>
#include <vector>

class test_rpc : public ::testing::Test{
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)> {};

  using server_type = xtd::rpc::rpc_server<xtd::rpc::tcp_transport, Add, Echo, Average>;
  using client_type = typename server_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

  static const xtd::socket::ipv4address& address(){
    static xtd::socket::ipv4address oAddress("127.0.0.1", 19977);
    return oAddress;
  }

  static server_pointer_type& get_server(){
    static server_pointer_type oServer(new server_type(address()));
    return oServer;
  }

  static void SetUpTestCase(){
    get_server()->get<Add>().attach([](int a, int b) { return a + b; });
    get_server()->get<Echo>().attach([](const std::string& sval) -> std::string { return std::string(sval); });
    get_server()->get<Average>().attach([](const std::vector<double>& oVals) -> double {
      double dRet = 0;
      for (auto & oVal : oVals) { dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
    get_server()->start_server();
  }

  static void TearDownTestCase() {
    get_server()->stop_server();
    get_server().reset();
  }

};

TEST_F(test_rpc, payload){
  using namespace xtd::rpc;
  payload oPayload;
  // a default payload reserves a leading size_t length header used for framing
  EXPECT_EQ(oPayload.size(), sizeof(size_t));
  oPayload.push_back(123);
  EXPECT_EQ(oPayload.size(), sizeof(size_t) + 1);
}

TEST_F(test_rpc, add_test){
  test_rpc::client_type oClient(test_rpc::address());
  EXPECT_EQ(oClient.call<test_rpc::Add>(2, 3), 5);
  EXPECT_EQ(oClient.call<test_rpc::Add>(-4, 9), 5);
}

TEST_F(test_rpc, echo_test){
  test_rpc::client_type oClient(test_rpc::address());
  EXPECT_EQ(oClient.call<test_rpc::Echo>(std::string("Hello?")), std::string("Hello?"));
  EXPECT_EQ(oClient.call<test_rpc::Echo>(std::string("")), std::string(""));
}

TEST_F(test_rpc, average_test){
  test_rpc::client_type oClient(test_rpc::address());
  EXPECT_DOUBLE_EQ(oClient.call<test_rpc::Average>(std::vector<double>{2.0, 4.0, 6.0}), 4.0);
}


class test_rpc_null : public ::testing::Test {
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)> {};
  class Boom : public xtd::rpc::rpc_call<Boom, int(int)> {};

  using server_type = xtd::rpc::rpc_server<xtd::rpc::null_transport, Add, Echo, Average, Boom>;
  using client_type = typename server_type::client_type;
};

TEST_F(test_rpc_null, round_trip){
  auto oState = std::make_shared<xtd::rpc::null_transport::shared_state>();
  test_rpc_null::server_type oServer(oState);
  oServer.get<test_rpc_null::Add>().attach([](int a, int b) { return a + b; });
  oServer.get<test_rpc_null::Echo>().attach([](const std::string& sval) -> std::string { return std::string(sval); });
  oServer.get<test_rpc_null::Average>().attach([](const std::vector<double>& oVals) -> double {
    double dRet = 0;
    for (auto & oVal : oVals) { dRet += oVal; }
    dRet /= oVals.size();
    return dRet;
  });
  oServer.start_server();

  test_rpc_null::client_type oClient(oState);
  EXPECT_EQ(oClient.call<test_rpc_null::Add>(2, 3), 5);
  EXPECT_EQ(oClient.call<test_rpc_null::Add>(-4, 9), 5);
  EXPECT_EQ(oClient.call<test_rpc_null::Echo>(std::string("Hello?")), std::string("Hello?"));
  EXPECT_EQ(oClient.call<test_rpc_null::Echo>(std::string("")), std::string(""));
  EXPECT_DOUBLE_EQ(oClient.call<test_rpc_null::Average>(std::vector<double>{2.0, 4.0, 6.0}), 4.0);

  oServer.stop_server();
}

TEST_F(test_rpc_null, exception_propagation){
  auto oState = std::make_shared<xtd::rpc::null_transport::shared_state>();
  test_rpc_null::server_type oServer(oState);
  oServer.get<test_rpc_null::Boom>().attach([](int) -> int {
    throw xtd::exception(here(), "boom");
  });
  oServer.start_server();

  test_rpc_null::client_type oClient(oState);
  EXPECT_THROW(oClient.call<test_rpc_null::Boom>(0), xtd::exception);

  oServer.stop_server();
}

#if (XTD_OS_WINDOWS & XTD_OS)
class test_rpc_pipe : public ::testing::Test {
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};

  using server_type = xtd::rpc::rpc_server<xtd::rpc::anonymous_pipe_transport, Add, Echo>;
  using client_type = typename server_type::client_type;
};

TEST_F(test_rpc_pipe, round_trip){
  auto oServerPipe = xtd::windows::pipe::create();
  auto oClientPipe = xtd::windows::pipe::create();

  test_rpc_pipe::server_type oServer(oServerPipe, oClientPipe);
  oServer.get<test_rpc_pipe::Add>().attach([](int a, int b) { return a + b; });
  oServer.get<test_rpc_pipe::Echo>().attach([](const std::string& sval) -> std::string { return std::string(sval); });
  oServer.start_server();

  test_rpc_pipe::client_type oClient(oServerPipe, oClientPipe);
  EXPECT_EQ(oClient.call<test_rpc_pipe::Add>(2, 3), 5);
  EXPECT_EQ(oClient.call<test_rpc_pipe::Echo>(std::string("Hello?")), std::string("Hello?"));

  oServer.stop_server();
}
#endif
