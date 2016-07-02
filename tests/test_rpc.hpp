/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


class test_rpc : public ::testing::Test{
public:
  class Add : public xtd::rpc::call<int(int, int)> {};
  class Echo : public xtd::rpc::call<std::string(std::string)> {};
  class Average : public xtd::rpc::call<double(std::vector<double>)> {};

  using contract_type = xtd::rpc::contract<xtd::rpc::null_transport, Add, Echo, Average>;
  using server_type = contract_type::server_type;
  using client_type = contract_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

  static server_pointer_type _RPCServer;

  static void SetUpTestCase(){
    _RPCServer = server_pointer_type(new server_type("127.0.0.1", 9494));

    _RPCServer->Attach<Add>([](int a, int b) { return a+b; });
    _RPCServer->Attach<Echo>([](const std::string& sval) -> std::string { return std::string(sval); });
    _RPCServer->Attach<Average>([](const std::vector<double>& oVals) -> double {
      double dRet = 0;
      for (auto & oVal : oVals) { dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
    _RPCServer->start_server();
  }

  static void TearDownTestCase() {

  }

};

test_rpc::server_pointer_type test_rpc::_RPCServer;

TEST_F(test_rpc, oink){
  ASSERT_TRUE(true);
}

TEST_F(test_rpc, DISABLED_oink){
  return;
  std::this_thread::sleep_for(std::chrono::seconds(3));

  test_rpc::client_type oClient("127.0.0.1", 9494);
  oClient.connect();
  for (int i=0 ; i<1000 ; ++i){

    std::string sParam = "Hello!";
    auto sRet = oClient.call<test_rpc::Echo>(sParam);
    XTD_ASSERT(sParam == sRet);
  }
  XTD_ASSERT(3 == oClient.call<test_rpc::Add>(1, 2));
}
