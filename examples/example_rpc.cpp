/** @file
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/rpc.hpp>



class rpc_interface{
public:
  class Add : public xtd::rpc::rpc_call<Add, int(int, int)>{};
  class Echo : public xtd::rpc::rpc_call<Echo, std::string(std::string)>{};
  class Average : public xtd::rpc::rpc_call<Average, double(std::vector<double>)>{};

  using server_type = xtd::rpc::server<xtd::rpc::tcp_transport, Add, Echo, Average>;
  using client_type = typename server_type::client_type;

  using server_pointer_type = std::shared_ptr<server_type>;

};
/*

  static server_pointer_type& get_server(){
    static server_pointer_type oServer(new server_type);
    return oServer;
  }

  static void SetUpTestCase(){
    get_server()->attach<Add>([](int a, int b){ return a + b; });
    get_server()->attach<Echo>([](const std::string& sval) -> std::string{ return std::string(sval); });
    get_server()->attach<Average>([](const std::vector<double>& oVals) -> double{
      double dRet = 0;
      for (auto & oVal : oVals){ dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
    get_server()->start_server();
  }

  static void TearDownTestCase(){
    get_server()->stop_server();
  }

};



TEST_F(test_rpc, echo_test){
  test_rpc::client_type oClient;
  oClient.call<test_rpc::Echo>("Hello?");
}

*/


int main(){
  try{
    rpc_interface::server_type oServer(xtd::socket::ipv4address("0.0.0.0", 9977));
    oServer.attach<rpc_interface::Add>([](int a, int b){ return a + b; });
    oServer.attach<rpc_interface::Echo>([](const std::string& sval) -> std::string{ return std::string(sval); });
    oServer.attach<rpc_interface::Average>([](const std::vector<double>& oVals) -> double{
      double dRet = 0;
      for (auto & oVal : oVals){ dRet += oVal; }
      dRet /= oVals.size();
      return dRet;
    });
    oServer.start_server();

    rpc_interface::client_type oClient(xtd::socket::ipv4address("0.0.0.0", 9977));
    oClient.call<rpc_interface::Echo>("Hello?");

    oServer.stop_server();

    return 0;
  }
  catch (const xtd::exception& ex){
    FATAL("An xtd::exception occurred: ", ex.what());
  }
}
