#pragma once

namespace  xtd {
  namespace event_trace {


    struct rpc {
      using file_name_hash = size_t;
      using function_name_hash = size_t;
      using pid_type = size_t;

      struct quit : xtd::rpc::rpc_call<quit, int()>{};
      struct add_name : xtd::rpc::rpc_call<add_name, int(std::string)> {};
      struct post_message : xtd::rpc::rpc_call<post_message,
        int(xtd::event_trace::message_type, function_name_hash, file_name_hash, int line, xtd::process::pid_type,
            pid_type, std::string )>{};

      using server = xtd::rpc::rpc_server<xtd::rpc::tcp_transport, quit, add_name, post_message>;
      using client = typename server::client_type;

    };

  }
}