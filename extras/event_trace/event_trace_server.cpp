
#include <xtd/socket.hpp>
#include <xtd/rpc.hpp>
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
