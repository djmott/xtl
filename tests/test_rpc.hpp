/**
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/


class test_rpc : public ::testing::Test{
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)> {};

  using server_type = xtd::rpc::server<xtd::rpc::null_transport, Add, Echo, Average>;
  using client_type = typename server_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

  static server_pointer_type _RPCServer;

  static void SetUpTestCase(){
    _RPCServer = server_pointer_type(new server_type);

    _RPCServer->attach<Add>([](int a, int b) { return a+b; });
    _RPCServer->attach<Echo>([](const std::string& sval) -> std::string { return std::string(sval); });
    _RPCServer->attach<Average>([](const std::vector<double>& oVals) -> double {
      double dRet = 0;
      for (auto & oVal : oVals) { dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
  }

  static void TearDownTestCase() {

  }

};
