#ifndef STAGDEER_CLIENT_IPV4
#define STAGDEER_CLIENT_IPV4

#include "../socket_header.h"
#include <stdexcept>
#include <utility>

namespace stagdeer {
    namespace ip {
        class Ipv4addrs {
            public:

            ~ Ipv4addrs() {
                freeaddrinfo(M_ipv4addrinfo__);
                return;
            }

            Ipv4addrs() = default;
            Ipv4addrs(stagdeer::ip::Ipv4addrs&& other_Ipv4addrs)
                noexcept: 
                    M_ipv4addrinfo__(other_Ipv4addrs.M_ipv4addrinfo__){};
                    Ipv4addrs& operator=(stagdeer::ip::Ipv4addrs&& other_Ipv4addrs_operator)
                    noexcept {
                        if (this != &other_Ipv4addrs_operator) {
                            M_ipv4addrinfo__ = other_Ipv4addrs_operator.M_ipv4addrinfo__;
                        }
                        return *this;
                    }
            Ipv4addrs(const stagdeer::ip::Ipv4addrs&) = delete;
            Ipv4addrs& operator=(stagdeer::ip::Ipv4addrs&) = delete;
            Ipv4addrs(const char* M_hostname__ , uint16_t M_port__) {
                struct addrinfo M_hints{}, *M_result = nullptr;
                M_hints.ai_family = AF_INET;
                M_hints.ai_socktype = SOCK_STREAM;
                M_hints.ai_protocol = IPPROTO_TCP;
                char M_addrs_port_cstr_buf[16];
                snprintf(M_addrs_port_cstr_buf , sizeof(M_addrs_port_cstr_buf) , "%u" , M_port__);
                int M_return = getaddrinfo(M_hostname__,  M_addrs_port_cstr_buf , &M_hints, &M_result);
                if (M_return != 0) {
                    const char* M_err_str = gai_strerror(M_return);
                    if (M_err_str == nullptr) {
                        throw std::runtime_error("Resolver hostname failed!: unknown error");
                    } throw std::runtime_error("Resolver hostname failed: " + 
                        std::string(M_err_str));
                }
                M_ipv4addrinfo__ = M_result;
            }

            struct addrinfo* getResolverResult() {
                if (!M_ipv4addrinfo__) {
                    throw std::runtime_error("M_ipv4addrinfo__ is NullPtr!");
                }
                return M_ipv4addrinfo__;
            }

            private:
            struct addrinfo* M_ipv4addrinfo__ = nullptr;
        };
    }
}

#endif