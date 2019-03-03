
#include <iostream>

#include <thread>

#include <xtd/rpc/rpc.hpp>
#include "event_trace_rpc.hpp"
#include <xtd/xstring.hpp>


BEGIN_RPC_INTERFACE(FlopJam)
  RPC_CALL(ping, void());
  RPC_CALL(echo, std::string(std::string));
  RPC_CALL(shutdown, void());
END_RPC_INTERFACE()

int main(int argc, char * argv[]) {

    auto oServer = xtd::rpc::make_server<FlopJam, xtd::rpc::dummy_transport>();

    oServer.echo = [](xtd::rpc::server_context&, std::string s) { return s; };
    
    /*
    oServer.start(true);

    std::thread oThread([]() {
      auto oClient = xtd::rpc::make_client<FlopJam, xtd::rpc::dummy_transport>();
      auto sRet = oClient.echo("Hello!");
      oClient.shutdown();
    });


    oServer.start(true);
    */
    
    return 0;

}


#if 0
#include <xtd/process.hpp>
#include <xtd/event_trace.hpp>
#include "event_trace_rpc.hpp"


namespace evt = xtd::event_trace;

int debug(int argc, char * argv[]) {
  evt::rpc::server  oServer(xtd::socket::ipv4address("0.0.0.0", 9977));
  oServer.get<evt::rpc::quit>().attach([](){return 0; });
  /*
  oServer.attach<evt::rpc::add_name>([](std::string s){return 0; });
  oServer.attach<evt::rpc::post_message>([](xtd::event_trace::message_type, evt::rpc::function_name_hash, evt::rpc::file_name_hash, int line, xtd::process::pid_type,
                                           std::thread::id, xtd::tstring){  });
  */
  oServer.start_server();
}

int main(int argc, char * argv[]){
  for (int i=1 ; i<argc ; ++i){
    xtd::cstring sArg=argv[i];
    if ("-debug" == sArg) return debug(argc, argv);
  }
}

#endif