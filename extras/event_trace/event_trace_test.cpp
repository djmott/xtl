
#include <xtd/socket.hpp>
#include <xtd/rpc.hpp>
#include <xtd/process.hpp>
#include <xtd/event_trace.hpp>
#include "event_trace_rpc.hpp"

namespace evt = xtd::event_trace;

int debug(int argc, char * argv[]) {
  evt::rpc::client oClient(xtd::socket::ipv4address("127.0.0.1", 9977));
  oClient.call<evt::rpc::quit>();


}

int main(int argc, char * argv[]){
  for (int i=1 ; i<argc ; ++i){
    xtd::cstring sArg=argv[i];
    if ("-debug" == sArg) return debug(argc, argv);
  }
}
