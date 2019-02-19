/** @file
xtd::rpc system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/rpc.hpp>

class test_rpc : public ::testing::Test{
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)> {};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)> {};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)> {};

  using server_type = xtd::rpc::server<xtd::rpc::null_transport , Add, Echo, Average>;
  using client_type = typename server_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

  static server_pointer_type& get_server(){
    static server_pointer_type oServer(new server_type);
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

TEST_F(test_rpc, DISABLED_payload){
  using namespace xtd::rpc;
  payload oPayload;
  oPayload.push_back(123);
  EXPECT_EQ(oPayload.size(), 1);
//  EXPECT_EQ(oPayload.peek<uint8_t>(), 123);
}

TEST_F(test_rpc, DISABLED_echo_test){
  test_rpc::client_type oClient;
  oClient.call<test_rpc::Echo>("Hello?");
}
