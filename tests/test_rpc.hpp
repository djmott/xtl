/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


class test_rpc : public ::testing::Test{
public:
  static const uint16_t Port = 876;
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)> {};

  using server_type = xtd::rpc::server<xtd::rpc::tcp_transport , Add, Echo, Average>;
  using client_type = typename server_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

  static server_pointer_type& get_server(){
    xtd::socket::ipv4address oServerAddress("0.0.0.0", Port);
    static server_pointer_type oServer(new server_type(oServerAddress));
    return oServer;
  }

  static void SetUpTestCase(){
    get_server()->attach<Add>([](int a, int b) { return a+b; });
    get_server()->attach<Echo>([](const std::string& sval) -> std::string { return std::string(sval); });
    get_server()->attach<Average>([](const std::vector<double>& oVals) -> double {
      double dRet = 0;
      for (auto & oVal : oVals) { dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
    get_server()->start_server();
  }

  static void TearDownTestCase() {
    get_server()->stop_server();
  }

};

TEST_F(test_rpc, payload){
  using namespace xtd::rpc;
  payload oPayload;
  oPayload.push_back(123);
  EXPECT_EQ(oPayload.size(), 1);
  EXPECT_EQ(oPayload.peek<uint8_t>(), 123);
}

TEST_F(test_rpc, echo_test){
  test_rpc::client_type oClient(xtd::socket::ipv4address("127.0.0.1", test_rpc::Port));
  oClient.call<test_rpc::Echo>("Hello?");
}
