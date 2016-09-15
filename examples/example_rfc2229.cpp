/** @file
 * Demonstrates the dict protocol https://tools.ietf.org/html/rfc2229
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 *
*/

#include <xtd/xtd.hpp>
#include <xtd/grammars/rfc2229.hpp>
#include <xtd/socket.hpp>
#include <xtd/debug.hpp>

struct client : xtd::socket::ipv4_tcp_stream{
  using _super_t = xtd::socket::ipv4_tcp_stream;

  client() : _super_t() {
    _super_t::onRead.connect([this](){ OnRead(); });
    _super_t::onWrite.connect([this](){ OnWrite(); });
    _super_t::onError.connect([this](){ OnError(); });
  }


  void OnRead(){
    std::vector<char> sResponse(_super_t::bytes_available(), 0);
    _super_t::read(sResponse);
    DUMP(sResponse.size());
  }
  void OnWrite(){

  }
  void OnError(){

  }
};

int main(){
  try {
    client oClient;
    oClient.connect(xtd::socket::ipv4address("216.18.20.172", 2628));
    oClient.select(5000);
    return 0;
  }
  catch(const xtd::socket::exception& ex){
    ERR("A socket exception ", ex.errnum(), " occurred: ", ex.what());
  }
  catch(const xtd::exception& ex){
    ERR("An xtd::exception occurred: ", ex.what());
  }
  return -1;
}
