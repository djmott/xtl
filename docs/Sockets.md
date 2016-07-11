Sockets
=======
The XTL socket library provides low-level and high-level abstractions around sockets that can be mixed and matched to achieve a multitude of interfaces. The various socket behaviors are decomposed into independent policies which are composed at compile time into complex concrete types. Sockets are well suited for a hierarchy generation pattern because various socket types share behaviors in unique ways. The hierarchy generation pattern permits the composition of constituent behaviors without resorting to multiple inheritance. The hierarchy generation pattern also permits concrete compositions to contain only the interface elements that make sense and should be present for a particular socket type.

This concept is probably best explained from the highest level interfaces that will most commonly be used in applications. Here's a pre-defined typedef for an IPv4 UDP socket:

```{.cpp}
using ipv4_udp_socket = socket_base<ipv4address, socket_type::datagram, socket_protocol::udp, ip_options>;
```

The four constituent components that compose an `ipv4_udp_socket` are `ipv4address`, `socket_type::datagram`, `socket_protocol::udp` and `ip_options`. The `socket_base` template composes these individual behavioral components in a linear object hierarchy that avoids multiple inheritance. Some of these components are used in the IPv4 TCP socket:

```{.cpp}
using ipv4_tcp_stream = socket_base<ipv4address, socket_type::stream, socket_protocol::tcp, ip_options, connectable_socket, bindable_socket, listening_socket, selectable_socket>;
```

Additional behavioral policies can be added or removed as desired to achieve a variety of custom interfaces. For example, the `connectable_socket` behavior produces a `connect` method typically for TCP clients which the `bindable_socket` and `listening_socket` provide `bind` and `listen` respectively, typically for TCP servers. So, the predefined `ipv4_tcp_stream` type can be used as both a client and server. If so desired, these behaviors could be declared in separate interfaces to produce independent client and server socket types.
