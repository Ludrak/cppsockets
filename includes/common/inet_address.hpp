#pragma once

#include <string>
#include <exception>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#define MAX_HOST_RESOLVE_RETRIES 3
#define MAX_HOSTNAME_LEN         255

class InetAddress
{
    public:
        // constructs from IPv4 addr (i.e. on connection accepted)
        InetAddress(const struct sockaddr_in& addr);

        // constructs from IPv6 addr (i.e. on connection accepted)
        InetAddress(const struct sockaddr_in6& addr);

        // Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
        InetAddress(const std::string& hostname, const int port, const sa_family_t family = AF_INET);

        inline std::string  getHostname() const;

        inline std::string  getIpAddress() const;
        inline int          getPort() const;

        inline sa_family_t  getAddressFamily() const;

        inline sockaddr_in  getAddress4() const;
        inline sockaddr_in6 getAddress6() const;
    

    private:
        void        _get_addr_by_hostname(sockaddr *const addr, const socklen_t addr_len, std::string& ip_address, const std::string &hostname, const sa_family_t host_family);
        std::string _get_hostname_of_addr(const struct sockaddr* addr, const socklen_t addr_len);
        std::string _get_ip_of_addr4(const struct in_addr& addr) throw (std::logic_error);
        std::string _get_ip_of_addr6(const struct in6_addr& addr) throw (std::logic_error);

    protected:
        // hostname, ip, and port, from sockaddr structs
        std::string _hostname;
        std::string _ip_address;
        in_port_t   _port;

        // actual addr structs, _address_family indicates the current one
        struct sockaddr_in  _address_4;
        struct sockaddr_in6 _address_6;
        sa_family_t         _address_family;
};


inline std::string		InetAddress::getHostname() const
{
	return (this->_hostname + "(" + this->_ip_address + ")");
}

inline std::string		InetAddress::getIpAddress() const
{
	return (this->_ip_address);
}

inline int				InetAddress::getPort() const
{
	return (this->_port);
}

inline sa_family_t		InetAddress::getAddressFamily() const
{
	return (this->_address_family);
}

inline sockaddr_in		InetAddress::getAddress4() const
{
	return (this->_address_4);
}

inline sockaddr_in6	InetAddress::getAddress6() const
{
	return (this->_address_6);
}


