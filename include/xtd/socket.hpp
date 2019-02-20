/** @file
general purpose socket communication
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#pragma once
#include <xtd/xtd.hpp>

#if (XTD_OS_WINDOWS & XTD_OS)
  #include <winsock2.h>
  #include <windows.h>
  #include <Ws2ipdef.h>
  #include <ws2tcpip.h>
  static_assert(_WIN32_WINNT >= 0x500, "unsupported target Windows version");
  #define poll WSAPoll
#endif

#if (XTD_COMPILER_MINGW == XTD_COMPILER)
  #include <mswsock.h>
#endif

#if (XTD_OS_UNIX & XTD_OS)
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <netinet/in.h>
  #include <netinet/udp.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <poll.h>
  #include <unistd.h>
#endif

#include <type_traits>
#include <memory>


#include <xtd/exception.hpp>
#include <xtd/callback.hpp>
#include <xtd/meta.hpp>

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
  #pragma comment(lib, "ws2_32")
#endif

namespace xtd{

    namespace socket{
    /// @addtogroup Sockets
    /// @{
#if (!DOXY_INVOKED)
#if (XTD_COMPILER_MINGW == XTD_COMPILER)
    using POLLFD=WSAPOLLFD;
#elif (XTD_OS_UNIX & XTD_OS)
  using SOCKET = int;
  #define closesocket close
  #define ioctlsocket ioctl
#elif (XTD_OS_WINDOWS & XTD_OS)
    using POLLFD = pollfd;
#endif
#endif
    ///Represents an socket error
    class exception : public xtd::crt_exception{
    public:
      /// constructors
      /// @{
      exception(const source_location& loc, const std::string& swhat) : crt_exception(loc, swhat){}
      exception(const exception& ex) : crt_exception(ex){}
      explicit exception(exception&& ex) : crt_exception(std::move(ex)){}
      /// @}

      template <typename _return_t, typename _expression_t>
      inline static _return_t _throw_if(const xtd::source_location& source, _return_t ret, _expression_t exp, const char* expstr){
        if (exp(ret)){
          throw exception(source, expstr);
        }
        return ret;
      }

    };

#if (!DOXY_INVOKED)
    namespace _{
      template <typename _ty, int level, int optname> class socket_option{
      public:
        using value_type = _ty;
        static value_type get(SOCKET s){
          value_type iRet;
#if  (XTD_COMPILER_MSVC & XTD_COMPILER)
          int iSize = sizeof(value_type);
#else
          std::remove_pointer<typename xtd::template get_parameter<4, decltype(getsockopt)>::type>::type iSize = sizeof(value_type);
#endif
          socket::exception::throw_if(getsockopt(s, level, optname, reinterpret_cast<char*>(&iRet), &iSize), [](int i){ return (i<0); });
          return iRet;
        }
        static void set(SOCKET s, value_type newval){
          socket::exception::throw_if(setsockopt(s, level, optname, reinterpret_cast<char*>(&newval), sizeof(newval)), [](int i){ return (i<0); });
        }
      };
      template <int level, int optname> class socket_option<std::string, level, optname>{
      public:
        using value_type = std::string;
        static value_type get(SOCKET s){
          value_type iRet;
          int iSize = 0;
          socket::exception::throw_if(getsockopt(s, level, optname, reinterpret_cast<char*>(&iRet), &iSize), [](int i){ return (i<0); });
          return iRet;
        }
        static void set(SOCKET s, value_type newval){
          socket::exception::throw_if(setsockopt(s, level, optname, reinterpret_cast<char*>(&newval), sizeof(newval)), [](int i){ return (i<0); });
        }
      };


    }
#endif

    ///IPv4 address wrapper around sockaddr_in
    class ipv4address : public sockaddr_in{
    public:
      /// ipv4 address familt
      static const int address_family = AF_INET;
      /**
       * constructor
       * @param sIP IP address
       * @param iPort port
       */
      ipv4address(const char * sIP, uint16_t iPort){
        #if (NTDDI_VERSION >= NTDDI_VISTA)
          sin_family = AF_INET;
          inet_pton(AF_INET, sIP, &sin_addr.s_addr);
          sin_port = htons(iPort);
        #else
          sin_family = AF_INET;
          sin_addr.s_addr = inet_addr(sIP);
          sin_port = htons(iPort);
        #endif
      }
      ipv4address(const ipv4address& src){
        memcpy(this, &src, sizeof(ipv4address));
      }
      ipv4address& operator=(const ipv4address& src){
        if (&src != this) memcpy(this, &src, sizeof(ipv4address));
        return *this;
      }
    };

    ///IPv6 address wrapper around sockaddr_in6
    class ipv6address : public sockaddr_in6{
    public:
      /// ipv6 address family
      static const int address_family = AF_INET6;
      TODO("Implement proper ipv6 address");
    };

    /// socket communication styles
    enum class socket_type{
      stream = SOCK_STREAM, ///< stream reliable FIFO stream on a remote socket
      datagram = SOCK_DGRAM, ///< datagram unreliable connectionless broadcast
      raw = SOCK_RAW, ///< raw interface level data
    };

    /// IP based protocols
    enum class socket_protocol{      
      ip = IPPROTO_IP, ///< ip Internet Protocol v4      
      icmp = IPPROTO_ICMP,///< icmp Internet Control Message Protocol      
      tcp = IPPROTO_TCP,///< tcp Transmission Control Protocol     
      udp = IPPROTO_UDP, ///< udp User Datagram Protocol
      ipv6 = IPPROTO_IPV6, ///< ipv6 Internet Protocol v6
    };
    NOTE("add more socket protocols as needed defined in netinet/in.h")


    TODO("Refactor out socket::serializer into something more universal")
    ///Serializes data on a socket
    template <typename> class serializer;

#if (XTD_OS_WINDOWS & XTD_OS)
    /** Initializes WinSock
    Winsock requires a call to WSAStartup before any other calls to the winsock library.
    winsock_initializer is a global static that ensures winsock is initialized and cleaned up properly
    */
    class winsock_initializer sealed{
    public:
      //singleton getter method
      static winsock_initializer &get(){
        static winsock_initializer oInit;
        return oInit;
      }
      winsock_initializer(const winsock_initializer&) = delete;
      winsock_initializer& operator=(const winsock_initializer&) = delete;
    private:
      winsock_initializer(){
        WSADATA oData;
        crt_exception::throw_if(WSAStartup(0x202, &oData), [](int err){ return 0 != err; });
        if (oData.wVersion != 0x202) throw crt_exception(here(), "Invalid winsock version");
      }

      ~winsock_initializer(){ WSACleanup(); }
    };
#endif


#if (!DOXY_INVOKED)
    template<typename _address_t, socket_type, socket_protocol, template<class> class ... _policy_ts> class socket_base;


    template<typename _address_t, socket_type _socket_t, socket_protocol _socket_protocol, template<class> class _head_t, template<class> class ..._tail_ts>
    class socket_base<_address_t, _socket_t, _socket_protocol, _head_t, _tail_ts...> : public _head_t<socket_base<_address_t, _socket_t, _socket_protocol, _tail_ts...> >{
    public:
      template<typename ... _arg_ts> explicit socket_base(_arg_ts &&...oArgs) : _head_t<socket_base<_address_t, _socket_t, _socket_protocol, _tail_ts...> >(std::forward<_arg_ts>(oArgs)...){}
    };
#endif

    /** Base class of the various socket specializations
    Hierarchy generation TMP pattern to create all the socket object types with the various behavioral policies.
     */
    template<typename _address_t, socket_type _socket_t, socket_protocol _socket_protocol>
    class socket_base<_address_t, _socket_t, _socket_protocol>{
    public:

      /// typedefs
      /// @{
      using unique_ptr = std::unique_ptr<socket_base>;
      using shared_ptr = std::shared_ptr<socket_base>;
      using address_type = _address_t;
      /// @}

      /// constants
      /// @{
      static constexpr socket_type type = _socket_t;
      static constexpr socket_protocol protocol = _socket_protocol;
      /// @}

      /// dtor
      ~socket_base(){ close(); }

      /// constructors
      /// @{
      socket_base() : _socket(0){
#if (XTD_OS_WINDOWS & XTD_OS)
        winsock_initializer::get();
#endif
        _socket = xtd::crt_exception::throw_if(::socket(address_type::address_family, (int)type, (int)protocol), [](SOCKET s){ return static_cast<SOCKET>(-1) == s; });
      }

      explicit socket_base(socket_base&& src) : _socket(src._socket){
        src._socket = 0;
      }


      explicit socket_base(SOCKET newval) : _socket(newval){}

      socket_base(const socket_base&) = delete;
      /// @}

      /// copy-assignment not permitted
      socket_base& operator=(const socket_base&) = delete;

      /**
       * move assignment
       * @param src source socket
       * @return *this
       */
      socket_base& operator=(socket_base&& src){
        std::swap(_socket, src._socket);
        return *this;
      }

      /**
       * cast to native socket
       * @return native socket value
       */
      operator SOCKET() const{ return _socket; }


      /**
       * writes data to the connected socket
       * @param data the data to write
       */
      template <typename _ty> void write(const _ty& data){
        serializer<_ty>::write(*this, data);
      }

      /**
       * reads data from the connected socket
       * @param data returned data
       */
      template <typename _ty> void read(_ty& data){
        serializer<_ty>::read(*this, data);
      }

      /**
       * reads data from the connected socket
       * @return the data of type _Ty from the socket
       */
      template <typename _ty> _ty read(){
        _ty data;
        serializer<_ty>::read(*this, data);
        return data;
      }

      /**
       * Closes the open socket
       */
      void close(){
        if (!_socket){
          return;
        }
        ::closesocket(_socket);
        _socket = -1;
      }

      /** sets the blocking mode of the socket
       * @param blocking true to set to blocking mode
       */
      void set_blocking(bool blocking){
        u_long val = (blocking ? 0 : 1);
        xtd::crt_exception::throw_if(ioctlsocket(_socket, FIONBIO, &val), [](int i){ return i < 0; });
      }

      /** gets the number of bytes waiting in the read buffer
       */
      u_long bytes_available(){
        u_long iRet=0;
        xtd::crt_exception::throw_if(ioctlsocket(_socket, FIONREAD, &iRet), [](int i){ return i < 0; });
        return iRet;
      }

      /**
       * test if the socket is valid
       */
      bool is_valid() const{ return -1 != _socket; }

    protected:
      /// OS/CRT inner SOCKET that is being managed by this wrapper
      SOCKET _socket;

    };


    ///Polling behavior policy
    template <typename _super_t>
    class polling_socket : public _super_t{
    public:
      /// callback event fires when data is ready to read data
      callback<void()> read_event;
      /// callback event fires when socket is ready to write data
      callback<void()> write_event;
      /// callback event fires when socket becomes disconnected
      callback<void()> disconnect_event;
      /// callback event fires when an error occurs
      callback<void()> error_event;
      /// begins polling the socket for events for a period of Timeout
      void poll(int Timeout){
        struct pollfd oPoll;
        oPoll.events = POLLIN | POLLOUT;
        oPoll.fd = _super_t::_socket;
        oPoll.revents = 0;
        if (0 == xtd::crt_exception::throw_if(::poll(&oPoll, 1, Timeout), [](int i){ return i < 0; })){
          return;
        }
        if (oPoll.revents & POLLERR){
          error_event();
        }
        if (oPoll.revents & POLLHUP){
          disconnect_event();
        }
        if (oPoll.revents & POLLIN){
          read_event();
        }
        if (oPoll.revents & POLLOUT){
          write_event();
        }
      }
      ///ctor
      template<typename ... _arg_ts>
      explicit polling_socket(_arg_ts &&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

#if (XTD_OS_WINDOWS & XTD_OS)
      int poll(POLLFD *ufds, unsigned int nfds, int timeout){ return ::WSAPoll(ufds, nfds, timeout); }
#endif
    };


    ///Server side binding behavior
    template <typename _super_t>
    class bindable_socket : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit bindable_socket(_arg_ts &&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      /// binds the socket to an address and port
      void bind(const typename _super_t::address_type& addr){
        exception::throw_if(::bind(_super_t::_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(typename _super_t::address_type)), [](int i){ return i < 0; });
      }
    };


    ///Client side connecting behavior
    template <typename _super_t>
    class connectable_socket : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit connectable_socket(_arg_ts &&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      /// initiates connection to a socket
      void connect(const typename _super_t::address_type& addr){
        exception::throw_if(::connect(_super_t::_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(typename _super_t::address_type)), [](int i){ return i < 0; });
      }
    };


    ///Server side listening behavior
    template <typename _super_t>
    class listening_socket : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit listening_socket(_arg_ts &&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      /// begins listening on the socket
      void listen(int Backlog = SOMAXCONN){
        exception::throw_if(::listen(_super_t::_socket, Backlog), [](int i){ return i < 0; });
      }

      /// accepts an incoming connection request
      template <typename _return_t>
      _return_t accept(){
        return _return_t(exception::throw_if(::accept(_super_t::_socket, nullptr, nullptr), [](SOCKET s){ return (s <= 0); }));
      }
    };

    /// Socket properties
    template <typename _super_t>
    class socket_options : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit socket_options(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      /// gets the SO_KEEPALIVE property
      bool keep_alive() const{ return (_::socket_option<int, SOL_SOCKET, SO_KEEPALIVE>::get(_super_t::_socket) ? true : false); }
      /// sets the SO_KEEPALIVE property
      void keep_alive(bool newval){ _::socket_option<int, SOL_SOCKET, SO_KEEPALIVE>::set(_super_t::_socket, newval); }
      TODO("Add more SOL_SOCKET options");
    };

    /// IP based socket properties
    template <typename _super_t>
    class ip_options : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit ip_options(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}
#if (XTD_OS_WINDOWS & XTD_OS)
      /// gets the IP_DONTFRAGMENT property
      bool dont_fragment() const{ return (_::socket_option<int, IPPROTO_IP, IP_DONTFRAGMENT>::get(_super_t::_socket) ? true : false); }
      /// sets the IP_DONTFRAGMENT property
      void dont_fragment(bool newval){ _::socket_option<int, IPPROTO_IP, IP_DONTFRAGMENT>::set(_super_t::_socket, newval); }
#endif
      TODO("Add more IPPROTO_IP options");
    };

    /// TCP based socket properties
    template <typename _super_t>
    class tcp_options : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit tcp_options(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

      /// gets the TCP_NODELAY property
      bool no_delay() const{ return (_::socket_option<int, IPPROTO_TCP, TCP_NODELAY>::get(_super_t::_socket) ? true : false); }
      /// sets the TCP_NODELAY property
      void no_delay(bool newval){ _::socket_option<int, IPPROTO_TCP, TCP_NODELAY>::set(_super_t::_socket, newval); }
      TODO("Add more IPPROTO_TCP options");
    };

    /// UDP socket properties
    template <typename _super_t>
    class udp_options : public _super_t{
    public:

      /// ctor
      template<typename ... _arg_ts>
      explicit udp_options(_arg_ts&&...oArgs) : _super_t(std::forward<_arg_ts>(oArgs)...){}

#if (XTD_OS_WINDOWS & XTD_OS)
      /// gets the UDP_NOCHECKSUM property
      bool no_checksum() const{ return (_::socket_option<int, IPPROTO_UDP, UDP_NOCHECKSUM>::get(_super_t::_socket) ? true : false); }
      /// sets the UDP_NOCHECKSUM property
      void no_checksum(bool newval){ _::socket_option<int, IPPROTO_UDP, UDP_NOCHECKSUM>::set(_super_t::_socket, newval); }
#endif
      TODO("Add more IPPROTO_UDP options");
    };


    ///Async IO select behavior
    template <typename _super_t>
    class selectable_socket : public _super_t{
    public:
      /// ctor
      template<typename ... _arg_ts>
      explicit selectable_socket(_arg_ts&&...oArgs)
        : _super_t(std::forward<_arg_ts>(oArgs)...), onRead(), onWrite(), onError() {}

      /// callback event fired when data is ready to be read
      xtd::callback<void()> onRead;
      /// callback event fired when socket is ready to write
      xtd::callback<void()> onWrite;
      /// callback event fired when a socket error occurs
      xtd::callback<void()> onError;

      /// begin the select to wait for an event or timeout
      bool select(int WaitMS){
        timeval tv;
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdErr;
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdErr);
        FD_SET((SOCKET)*this, &fdRead);
        FD_SET((SOCKET)*this, &fdWrite);
        FD_SET((SOCKET)*this, &fdErr);
        tv.tv_sec = WaitMS / 1000;
        WaitMS %= 1000;
        tv.tv_usec = WaitMS / 1000;
#if (XTD_COMPILER_MINGW == XTD_COMPILER)
        auto iRet = xtd::crt_exception::throw_if(::select(1 + (SOCKET)*this, &fdRead, &fdWrite, &fdErr, reinterpret_cast<PTIMEVAL>(&tv)), [](int i){return i < 0; });
#else
        auto iRet = xtd::crt_exception::throw_if(::select(static_cast<int>(1 + (SOCKET)*this), &fdRead, &fdWrite, &fdErr, &tv), [](int i){return i < 0; });
#endif
        if (0 == iRet){
          return false;
        }
        if (FD_ISSET((SOCKET)*this, &fdErr)){
          onError();
        }
        if (FD_ISSET((SOCKET)*this, &fdRead)){
          onRead();
        }
        if (FD_ISSET((SOCKET)*this, &fdWrite)){
          onWrite();
        }
        return true;
      }
    };

#if (!DOXY_INVOKED)
    TODO("Get rid of these")
    template <typename _ty>
    class serializer{
    public:

      template <typename _socket_t>
      static void write(_socket_t& oSocket, const _ty& src){
        static_assert(std::is_pod<_ty>::value, "no acceptable specialization for type");
        exception::throw_if(::send(oSocket, reinterpret_cast<const char*>(&src), sizeof(_ty), 0), [](int i){ return i <= 0; });
      }

      template <typename _socket_t>
      static void read(_socket_t& oSocket, _ty& src){
        static_assert(std::is_pod<_ty>::value, "no acceptable specialization for type");
        exception::throw_if(::recv(oSocket, reinterpret_cast<char*>(&src), sizeof(_ty), 0), [](int i){ return i <= 0; });
      }
    };


    template <typename _ty>
    class NON_POD_Vector_Serializer{
    public:

      template <typename _socket_t>
      static void write(_socket_t& oSocket, const std::vector<_ty>& src){
        serializer<typename std::vector<_ty>::size_type>::write(oSocket, src.size());
        for (const auto & oItem : src){
          serializer<_ty>::write(oSocket, oItem);
        }
      }

      template <typename _socket_t>
      static void read(_socket_t& oSocket, std::vector<_ty>& src){
        typename std::vector<_ty>::size_type count;
        serializer<typename std::vector<_ty>::size_type>::read(oSocket, count);
        for (; count; --count){
          _ty newval;
          serializer<_ty>::read(oSocket, newval);
          src.push_back(newval);
        }
      }
    };


    template <typename _ty>
    class POD_Vector_Serializer{
    public:

      template <typename _socket_t >
      static void write(_socket_t& oSocket, const std::vector<_ty>& src){
        serializer<typename std::vector<_ty>::size_type>::write(oSocket, src.size());
        send(oSocket, reinterpret_cast<const char*>(&src[0]), (int)(sizeof(_ty) * src.size()), 0);
      }

      template <typename _socket_t>
      static void read(_socket_t& oSocket, std::vector<_ty>& src){
        typename std::vector<_ty>::size_type count;
        serializer<typename std::vector<_ty>::size_type>::read(oSocket, count);
        src.resize(count);
        recv(oSocket, reinterpret_cast<char*>(&src[0]), (int)(sizeof(_ty) * count), 0);
      }

    };


    template <typename _ty>
    class serializer<std::vector<_ty>> : public std::conditional<std::is_pod<_ty>::value, POD_Vector_Serializer<_ty>, NON_POD_Vector_Serializer<_ty>>::type{};
#endif
    /// General purpose IPV4 client and server socket type
    using ipv4_tcp_stream = socket_base<ipv4address, socket_type::stream, socket_protocol::tcp, socket_options, ip_options, tcp_options, connectable_socket, bindable_socket, listening_socket, selectable_socket>;
    /// General purpose UDP socket type
    using ipv4_udp_socket = socket_base<ipv4address, socket_type::datagram, socket_protocol::udp, socket_options, ip_options, udp_options>;
    ///@}

  }

}
