#ifndef STAGDEER_CLIENT_TCP_SOCKET
#define STAGDEER_CLIENT_TCP_SOCKET

#include "../socket_header.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include "../../util/type_util.hpp"
#include "../../../thread/thread.h"
#include "../../buffer/buffer.hpp"
#include "../ip/Ipv4addrs.h"
#include "../ip/Ipv6addrs.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>


namespace stagdeer {
    namespace client {
        class socketTcp : public std::enable_shared_from_this<socketTcp> {
            public:

            socketTcp() = default;

            socketTcp(const char* M_hostname__ , uint16_t M_port__ , const char* M_message__) {
                M_client_config_addrs.M_addrs_host = M_hostname__;
                M_client_config_addrs.M_addrs_port = M_port__;
                M_client_config_addrs.M_clientMessage = M_message__;
                return;
            }

            ~socketTcp() {
                free(M_resolver_Ipv4);
                free(M_resolver_Ipv6);
                return;
            }
            socketTcp(stagdeer::client::socketTcp&& other_tcp) noexcept: 
                M_client_config_addrs(std::forward<struct client_addrs>
                    (other_tcp.M_client_config_addrs)) , 
                    M_resolver_Ipv4(other_tcp.M_resolver_Ipv4),
                    M_resolver_Ipv6(other_tcp.M_resolver_Ipv6),
                    M_connects(std::forward<std::unordered_map<M_SOCKET_TP, struct connectInfo>>(other_tcp.M_connects)){};
                socketTcp(const socketTcp&) = delete;
                socketTcp& operator=(const socketTcp&) = delete;
                socketTcp& operator=(socketTcp&& other_tcp_operator) 
                    noexcept {
                        if (this != &other_tcp_operator) {
                            M_connects = std::forward<std::unordered_map<M_SOCKET_TP, struct connectInfo>>(other_tcp_operator.M_connects);
                            M_client_config_addrs = std::forward<struct client_addrs>(other_tcp_operator.M_client_config_addrs);
                            M_resolver_Ipv4 = other_tcp_operator.M_resolver_Ipv4;
                            M_resolver_Ipv6 = other_tcp_operator.M_resolver_Ipv6;
                        }
                        return *this;
                    }

                struct client_addrs {
                    const char* M_addrs_host;
                    uint16_t M_addrs_port;
                    struct addrinfo* M_resovler_addrs;
                    const char* M_clientMessage;
                    int M_clientMessageLength;
                    M_SOCKET_TP M_socketfd;
                    uint16_t M_timeout;
                    bool M_is_enable_ipV6 = false;
                    bool M_this_addr_invalid = false;
                    int M_connect_rety_count = 0;
                    int M_max_rety_count = 0;
                    size_t M_client_write_bytes = 0;
                    bool M_is_retry_success = false;

                    client_addrs() = default;
                    client_addrs(client_addrs&& other_addrs) 
                        noexcept: M_addrs_host(std::forward<const char*>(other_addrs.M_addrs_host)),
                            M_client_write_bytes(std::move(other_addrs.M_client_write_bytes)),
                            M_resovler_addrs(other_addrs.M_resovler_addrs),
                            M_addrs_port(std::forward<uint16_t>(other_addrs.M_addrs_port)),
                            M_clientMessage(std::forward<const char*>(other_addrs.M_clientMessage)),
                            M_clientMessageLength(std::forward<size_t>(other_addrs.M_clientMessageLength)),
                            M_is_enable_ipV6(std::forward<bool>(other_addrs.M_is_enable_ipV6)),
                            M_timeout(std::forward<uint16_t>(other_addrs.M_timeout)),
                            M_this_addr_invalid(std::forward<bool>(other_addrs.M_this_addr_invalid)),
                            M_is_retry_success(std::forward<bool>(other_addrs.M_is_retry_success)),
                            M_socketfd(std::forward<M_SOCKET_TP>(other_addrs.M_socketfd)),
                            M_max_rety_count(std::forward<int>(other_addrs.M_max_rety_count)) {};
                    client_addrs& operator=(client_addrs&& other_addrs) {
                        if (this != &other_addrs) {
                            M_max_rety_count = std::forward<int>(other_addrs.M_max_rety_count);
                            M_socketfd = std::forward<M_SOCKET_TP>(other_addrs.M_socketfd);
                            M_addrs_host = std::forward<const char*>(other_addrs.M_addrs_host);
                            M_addrs_port = std::forward<uint16_t>(other_addrs.M_addrs_port);
                            M_resovler_addrs = other_addrs.M_resovler_addrs;
                            M_client_write_bytes = other_addrs.M_client_write_bytes;
                            M_clientMessage = std::forward<const char*>(other_addrs.M_clientMessage);
                            M_clientMessageLength = std::forward<size_t>(other_addrs.M_clientMessageLength);
                            M_is_enable_ipV6 = std::forward<bool>(other_addrs.M_is_enable_ipV6);
                            M_timeout = std::forward<uint16_t>(other_addrs.M_timeout);
                            M_is_retry_success = std::forward<bool>(other_addrs.M_is_retry_success);
                            M_this_addr_invalid = std::forward<bool>(other_addrs.M_this_addr_invalid);
                        }
                        return *this;
                    };
                    client_addrs(const client_addrs&) = default;
                    client_addrs& operator=(const client_addrs&) = default;
                };

                struct client_addrs getMyaddr() {
                    return M_client_config_addrs;
                }

                /**
                //STATUS ERROR CODE
                @param -1 | CREATE FALIED
                @param 1 | CREATE SUCCESS
                */
                template<typename Tp>
                typename stagdeer::util::lamdba_trais::constraint<
                    stagdeer::util::lamdba_trais::M_is_retTp
                        <typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp<
                            Tp, int , std::string , struct stagdeer
                                ::client::socketTcp::client_addrs&&>
                        ::__M_ret_lmdba, void>
                    ::__is_M_ret_Tp
                >::type
                async_resolver_domain(
                    Tp&& callback_token,
                    struct client_addrs&& M_addrs_ , 
                    uint16_t M_timeout = 2000 ,
                    bool M_reuse_addr = true,
                    bool M_enable_Ipv6 = false,
                    int M_max_ret_count = 10
                ) noexcept {
                    if (M_addrs_.M_addrs_port == 0 || M_addrs_.M_addrs_host == nullptr) {
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) ,
                                    -1 , "Paramter canot empty" , std::move(M_addrs_));
                        return -1;
                    }
                    //ADD CONFIG TO ADDRS STRUCT
                    M_addrs_.M_timeout = M_timeout;
                    M_addrs_.M_is_enable_ipV6 = M_enable_Ipv6;
                    M_addrs_.M_max_rety_count = M_max_ret_count;
                        //CREATE NEW SOSKER
                        //RESOLVER DOMAIN
                        if (M_enable_Ipv6) {
                            //RESOVLER IPV6
                          M_resolver_Ipv6 = new stagdeer::
                          ip::Ipv6addrs(M_addrs_.M_addrs_host , M_addrs_.M_addrs_port);
                            try {
                                struct addrinfo* resolver_result = M_resolver_Ipv6->getResolverResult();
                                if (!resolver_result) {
                                    #ifdef STAGDEER_GNU_LINUX
                                        std::string M_err_message = strerror(errno);
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) ,
                                                 -1 , M_err_message , std::move(M_addrs_));
                                        return -1;
                                    #else 
                                        //TODO: WINDOWS ERROR HANDLERS
                                    #endif
                                }
                                //CREATE SOCKET
                                M_addrs_.M_socketfd = socket(resolver_result->ai_family, 
                                    resolver_result->ai_socktype, resolver_result->ai_protocol);
                                //ADD TO THREAD TASK
                                if (M_addrs_.M_socketfd == -1) {
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) , -1 , 
                                            std::string("Invalid socket fd: " + std::to_string(M_addrs_.M_socketfd)) , 
                                            std::move(M_addrs_));
                                        return -1;
                                }
                                M_addrs_.M_resovler_addrs = resolver_result;
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token),
                                     1 , std::string("Create socket success") ,
                                      std::move(M_addrs_));
                                return 1;
                            } catch (std::exception& M_err) {
                                //RESOLVER FAILED
                                printf("Resolver hostname failed! failed: %s\n" , 
                                    std::string(M_err.what()).c_str());
                                return -1;
                            }
                            return -1;
                        }
                        //RESOLVER IPV4
                        M_resolver_Ipv4 = new stagdeer::
                            ip::Ipv4addrs(M_addrs_.M_addrs_host , M_addrs_.M_addrs_port);
                        try {
                            struct addrinfo* resolver_result = M_resolver_Ipv4->getResolverResult();
                            if (!resolver_result) {
                                #ifdef STAGDEER_GNU_LINUX
                                printf("Parser failed\n");
                                    std::string error_message = strerror(errno);
                                        M_threadManager.getThreadManager()
                                            .asyncTaskvoid(std::move(callback_token) ,
                                             -1 , error_message , std::move(M_addrs_));
                                    return -1;
                                #else 
                                    //TODO: WINDOWS ERROR HANDLERS
                                #endif
                            }
                            //CREATE SOCKET
                            M_addrs_.M_socketfd = socket(resolver_result->ai_family, 
                                resolver_result->ai_socktype, resolver_result->ai_protocol);
                            //ADD TO THREAD TASK
                            if (M_addrs_.M_socketfd == -1) {
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token) , 
                                    -1 ,std::string("Invalid socket fd: " + std::to_string(M_addrs_.M_socketfd)),
                                    std::move(M_addrs_));
                                return -1;
                            }
                            M_addrs_.M_resovler_addrs = resolver_result;
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) , 1 , 
                                std::string("Create success"), std::move(M_addrs_));
                            return 1;
                        } catch (std::exception& M_err) {
                            //RESOLVER FAILED
                            printf("Resolver hostname failed! failed: %s\n" ,
                                 std::string(M_err.what()).c_str());
                            return -1;
                        }
                        return -1;
                }


                /**
                //STATUS ERROR CODE
                @param -1 | CONNECT FALIED
                @param 1 | CONNECT SUCCESS
                @param 2 | RETRY CONNECT SUCCESS
                */
                template<typename Tp>
                inline typename stagdeer::util::lamdba_trais::constraint<
                    stagdeer::util::lamdba_trais::M_is_retTp<
                        typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp< 
                            Tp&&, int , std::string , struct stagdeer
                                ::client::socketTcp::client_addrs&&>::__M_ret_lmdba, void 
                    >::__is_M_ret_Tp
                >::type
                async_try_connect_tcp(
                    Tp&& callback_token,
                    struct client_addrs&& M_addrs_
                ) noexcept {
                        if (M_addrs_.M_addrs_host == nullptr || M_addrs_.M_resovler_addrs == nullptr) {
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token),
                                 -1 , std::string("Paramter error!") ,
                                  std::move(M_addrs_));
                            return -1;
                        }
                            //VERIFIYCATION FD
                            if (M_addrs_.M_socketfd == -1) {
                                //INVALID FD
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token) ,  -1 ,
                                     std::string("Invalid socket fd: " + std::to_string(M_addrs_.M_socketfd)) , 
                                     std::move(M_addrs_));
                                return -1;
                            }
                            //VERIFIYCATION INVALID IPADDRS
                            struct client_addrs M_addr = M_tryIpaddrs(std::move(M_addrs_));
                            if (M_addr.M_this_addr_invalid) {
                               // ALL ADDRS INVALID
                                M_addrs_ = std::move(M_addr);
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token) ,
                                    -1 , std::string("All resolver addrs invalid") , 
                                   std::move(M_addrs_));
                               return -1;
                            }
                            //CONNECTION RESOLVER RESULT ADDRS
                           M_addrs_ = std::move(M_addr);
                           socket_setFcntl(M_addrs_.M_socketfd);
                            int M_connectRet = connect(M_addrs_.M_socketfd,M_addrs_.M_resovler_addrs->ai_addr, 
                            M_addrs_.M_resovler_addrs->ai_addrlen);
                                if (M_connectRet < 0) {
                                    //ERROR HANDLER

                                    //SETTING TIMEOUT
                                    fd_set M_writefds;
                                    FD_ZERO(&M_writefds);
                                    FD_SET(M_addrs_.M_socketfd, &M_writefds);

                                    struct timeval M_tv;
                                    M_tv.tv_sec = M_addrs_.M_timeout;
                                    M_tv.tv_usec = 0;

                                    #ifdef STAGDEER_GNU_LINUX
                                    int M_select_ret = select(M_addrs_.M_socketfd + 1, NULL , &M_writefds, NULL, &M_tv);
                                    if (M_select_ret < 0) {
                                        int errno_copy = errno;
                                        const std::string M_erro_message = strerror(errno);
                                        if (errno_copy == ECONNREFUSED || errno_copy == ENETUNREACH || errno_copy == EHOSTUNREACH) {
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) , 
                                                -1 , std::string("Connect failed: " + M_erro_message),
                                                std::move(M_addrs_));
                                            return -1;
                                        }
                                        struct client_addrs M_retry_addr = 
                                            M_retryTcpConnect(std::move(M_addrs_) , errno_copy);
                                        M_addrs_ = std::move(M_retry_addr);
                                        if (M_addrs_.M_is_retry_success) {
                                            //RETRY SUCCESS
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) , 2 ,
                                                 std::string("Retry connect suceess") , std::move(M_addrs_));
                                            return 2;
                                        }
                                        //RETRY FAILED
                                    #else
                                        //TODO: WINDOWS ERROR HEANDLER
                                    #endif
                                    std::string M_err_message = strerror(errno);
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) , -1 , 
                                        std::string("Connect failed" + M_err_message) , std::move(M_addrs_));
                                    return -1;
                                } else if (M_select_ret == 0) {
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) , -1 , 
                                        std::string("Connect failed: connection timeout") , std::move(M_addrs_));
                                    return -1;
                                }
                            }
                            //CONNECT SUCCESS
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) , 1 ,
                                 std::string("Connect success") , std::move(M_addrs_));
                            return 1;
                }

                /**
                //STATUS ERROR CODE
                @param -1 | WRITE FALIED
                @param 1 | WRITE SUCCESS
                @param 2 | RETRY WRITE SUCCESS
                @param 3 | WRITE SUCCESS BUT NOT INCOMPLETE
                */
                template<typename Tp>
                typename stagdeer::util::lamdba_trais::constraint<
                    stagdeer::util::lamdba_trais::M_is_retTp<
                        typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp<
                            Tp, int , std::string , size_t , 
                                struct client_addrs&&>::__M_ret_lmdba , void 
                    >::__is_M_ret_Tp
                >::type
                async_write(
                    Tp&& callback_token, 
                    struct client_addrs&& M_addr__,
                    const char* M_message__
                )
                noexcept {
                    if (M_addr__.M_socketfd < 0 || M_addr__.M_this_addr_invalid 
                        || M_addr__.M_resovler_addrs == nullptr || M_message__ == nullptr) {
                            //PARAMPER INVLIAD
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(callback_token , -1 ,
                                    std::string("Paramter invliad!"), size_t(0) , 
                                    std::move(M_addr__)
                                );
                        return -1;
                    }
                    
                    //UPDATE CLIENT CONFIGURE
                    size_t M_message_lenght = strlen(M_message__);
                    M_addr__.M_clientMessage = M_message__;
                    M_addr__.M_clientMessageLength = M_message_lenght;

                    M_threadManager.getThreadManager()
                        .asyncTaskvoid([self = shared_from_this() ,
                            M_callback_token__ = std::function<void(int , std::string , size_t ,
                                 struct stagdeer::client::socketTcp::client_addrs&&)>
                                 (std::move(callback_token)) , M_addr__ = std::move(M_addr__)]() 
                            mutable -> void {
                            //WRITE MESSAGE TO SERVER
                            if (self == nullptr) {
                                self->M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(M_callback_token__),
                                     -1 , std::string("Tcp object invalid") , size_t(0) ,
                                      std::move(M_addr__)
                                    );
                                return;
                            }

                            //TRY WRITE MESSAGE
                            int M_written = 0;
                            int M_total_write = 0;
                            while (M_written < strlen(M_addr__.M_clientMessage)) {
                                int M_write_ret = write(M_addr__.M_socketfd, 
                                    M_addr__.M_clientMessage + M_written, 
                                    strlen(M_addr__.M_clientMessage) - M_written
                                );

                                if (M_write_ret < 0) {
                                    //WRITE FAILED
                                    struct client_addrs M_retry_write = 
                                        self -> M_retryTcpwrite(M_addr__, M_write_ret, 
                                            strlen(M_addr__.M_clientMessage), errno);
                                        if (M_retry_write.M_is_retry_success) {
                                            //RETRY SUCCESS
                                            if (M_retry_write.M_client_write_bytes == 0) {
                                                //FALSE RETRY SUCCESS
                                                self -> M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__),
                                                    -1 ,std::string("Write failed! ERR: " + 
                                                        std::string(strerror(errno))), 
                                                        size_t(M_total_write) , std::move(M_addr__)
                                                    );
                                                return;
                                            } else {
                                                //TRUE RETRY SUCCESS
                                                self -> M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__), 2 ,
                                                    "Retry write to server success" , M_retry_write.M_client_write_bytes ,
                                                        std::move(M_retry_write)
                                                    );
                                                return;
                                            }
                                        }
                                    //RETRY FAILED
                                    self -> M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(M_callback_token__),
                                        -1 ,std::string("Write failed! ERR: " + 
                                            std::string(strerror(errno))), 
                                            size_t(M_total_write) , std::move(M_addr__)
                                        );
                                    return;
                                }

                                //UPDATE MESSAGE
                                M_total_write += M_write_ret;
                                M_written += M_write_ret;
                                if (M_total_write == strlen(M_addr__.M_clientMessage)) {
                                    //QUIT WHILE LOOP WRITE
                                    break;
                                }
                                //CONTINUE
                            } 
                            //WRITE SUCCESS
                            if (M_total_write != strlen(M_addr__.M_clientMessage)) {
                                //WRITE INCOMPLETE
                                self -> M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(M_callback_token__), 
                                        3 , std::string("Success but writed bytes not incomplete") , 
                                        size_t(M_total_write), std::move(M_addr__)
                                    );
                                return;
                            }

                            self -> M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(M_callback_token__), 1 , 
                                    std::string("Write success") , size_t(M_total_write) ,
                                    std::move(M_addr__)
                                );
                            return;
                        });
                    return -1;
                }

            /**
            
            */
            template<typename Tp>
            typename stagdeer::util::lamdba_trais::constraint<
                stagdeer::util::lamdba_trais::M_is_retTp<
                    typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp
                     <Tp, int , std::string , size_t , stagdeer::client::readBuffer>
                        ::__M_ret_lmdba, void>::__is_M_ret_Tp
            >::type
            async_read(
                Tp&& callback_token, 
                client_addrs& M_addr
            ) noexcept {
                if (M_addr.M_socketfd < 0 || M_addr.M_resovler_addrs == nullptr) {
                    M_threadManager.getThreadManager()
                        .asyncTaskvoid(std::move(callback_token), -1 , 
                            "Invalid socket!" , size_t(0) , NULL);
                    return -1;
                }
                
                
            }

            private:
            struct client_addrs M_client_config_addrs;
            stagdeer::THREAD& M_threadManager = stagdeer::THREAD::getInstance();

            #ifdef STAGDEER_GNU_LINUX
                struct client_addrs M_retryTcpConnect(struct client_addrs M_addrs__ , int M_errno_copy) {
                    if (M_addrs__.M_max_rety_count <= 0) {
                        M_addrs__.M_max_rety_count = 10;
                    }
                    while (M_addrs__.M_connect_rety_count < M_addrs__.M_max_rety_count) {
                    M_errno_copy = errno;
                    if (M_errno_copy == 106) {
                        //Already connection
                        M_addrs__.M_is_retry_success = true;
                        return M_addrs__;
                    }
                        if (
                            M_errno_copy == EAGAIN || 
                            M_errno_copy == EWOULDBLOCK || 
                            M_errno_copy == EINTR ||
                            M_errno_copy == ETIMEDOUT ||
                            M_errno_copy && M_addrs__.M_max_rety_count 
                                > M_addrs__.M_connect_rety_count) {
                            //RETRY TO
                            M_addrs__.M_connect_rety_count += 1; //RETY COUNT +1
                            SLEEP(1000 * M_addrs__.M_connect_rety_count);
                            int M_retry_result = connect(M_addrs__.M_socketfd, 
                            M_addrs__.M_resovler_addrs->ai_addr, 
                                M_addrs__.M_resovler_addrs->ai_addrlen);
                                if (M_retry_result < 0) {
                                    //RETRY FAILED
                                    if (M_addrs__.M_max_rety_count == M_addrs__.M_connect_rety_count) {
                                        //RETRY FAILED
                                        //CHISNES: 无药可救
                                        break;
                                    } else {
                                        //CONTINUE
                                        //CHINESE: 能救但是悬
                                        continue;
                                    }
                                } else {
                                    //RETRY SUCCESS
                                    M_addrs__.M_is_retry_success = true;
                                    return M_addrs__;
                                }
                            continue;
                        }
                        //NOT RETTY BREAK WHILE
                        break;
                    }
                    return M_addrs__;
                }

                struct client_addrs M_retryTcpwrite(struct client_addrs& M_addr , 
                    size_t M_total_write , size_t M_strfull_len, int M_errno_copy) {
                        if (M_addr.M_socketfd < 0 || M_addr.M_this_addr_invalid 
                            || M_addr.M_resovler_addrs == nullptr) {
                                M_addr.M_is_retry_success = false;
                                return M_addr;
                            }
                        if (M_errno_copy == EAGAIN || 
                            M_errno_copy == EWOULDBLOCK || 
                            M_errno_copy == EINTR ||
                            M_errno_copy == ENOBUFS ||
                            M_errno_copy == ENOMEM ||
                            M_errno_copy == ETIMEDOUT
                        ) {
                            //WTITE WITH RETRY
                            //CLEAR CONNECT RETRY COUNT
                            M_addr.M_connect_rety_count = 0;
                            while (M_addr.M_connect_rety_count < M_addr.M_max_rety_count) {
                                //ADD WRITE RETRY COUNT
                                M_addr.M_connect_rety_count ++;
                                SLEEP(1000 * M_addr.M_connect_rety_count);
                                const char* M_retry_char = M_addr.M_clientMessage;

                                //RETRY WRITE WHILE LOOP
                                int M_recv = 0;
                                while (M_total_write < M_strfull_len) {
                                    M_recv = write(M_addr.M_socketfd, 
                                        M_addr.M_clientMessage + M_total_write,
                                         M_strfull_len - M_total_write);
                                    if (M_recv < 0) {
                                        if (M_addr.M_max_rety_count >= M_addr.M_connect_rety_count) {
                                            //RETRY FAILED
                                            M_addr.M_is_retry_success = false;
                                            return M_addr;
                                        }
                                        //CONTINUE RETRY
                                        continue;
                                    }

                                    if (M_recv + M_total_write == M_strfull_len) {
                                        //RETRY SUCCESS
                                        M_addr.M_is_retry_success = true;
                                        M_addr.M_client_write_bytes = M_total_write;
                                        return M_addr;
                                    }

                                    //UPDATE MESSAGE
                                    M_total_write += M_recv;
                                }
                            }
                            //RETRY END
                            if (M_total_write >= M_strfull_len) {
                                //DATA COMPLETE
                                M_addr.M_is_retry_success = true;
                                M_addr.M_client_write_bytes = M_total_write;
                                return M_addr;
                            }

                            //DATA NOT INCOMPLETE
                            if (M_total_write < M_strfull_len || M_total_write <= 0) {
                                //FAILED
                                M_addr.M_is_retry_success = false;
                                return M_addr;
                            }
                        }
                    //CANOT RETRY ERROR
                    M_addr.M_is_retry_success = false;
                    return M_addr;
                }
            #else
                //TODO: WINDOWS RETRY
            #endif

            struct client_addrs M_tryIpaddrs(struct client_addrs M_addrs__) {
                if (M_addrs__.M_resovler_addrs == nullptr) {
                    throw std::runtime_error("'M_addrs__' is null pointer!");
                }
                //FOR RESOLVER RESULT
                for (struct addrinfo* M_resolver_result = M_addrs__.M_resovler_addrs; 
                    M_resolver_result != nullptr; M_resolver_result = 
                    M_resolver_result->ai_next) {
                        if (M_addrs__.M_is_enable_ipV6) {
                            if (M_resolver_result->ai_family != AF_INET6) {
                                continue;
                            }
                        } else {
                            if (M_resolver_result->ai_family != AF_INET) {
                                continue;
                            };
                        }
                        M_SOCKET_TP M_test_fd = socket(M_resolver_result->ai_family, 
                            M_resolver_result->ai_socktype, M_resolver_result->ai_protocol);
                        if (M_test_fd == -1) {
                            throw std::runtime_error("Test socket invalid!");
                        }
                        //TEST SOCKET CONNECT
                        if (connect(M_test_fd,M_resolver_result->ai_addr, M_resolver_result->ai_addrlen) == 0) {
                            //CONNECTION SUCCESS
                            int M_reuse = 1;
                            setsockopt(M_test_fd, SOL_SOCKET, 
                                SO_REUSEADDR, &M_reuse, sizeof(M_reuse));
                            M_addrs__.M_socketfd = std::move(M_test_fd);
                            return M_addrs__;
                        }
                        CLOSE_FD(M_test_fd);
                        continue;
                    }
                    //ALL ADDRS INVALID
                    printf("INVALID\n");
                    M_addrs__.M_this_addr_invalid = true;
                return M_addrs__;
            }
            
            struct connectInfo {
                bool M_is_keep_alive = false;
                M_SOCKET_TP M_server_fd;
                connectInfo() = default;
                connectInfo(connectInfo&& M_other_Connect) 
                noexcept: M_is_keep_alive(std::forward<bool>(M_other_Connect.M_is_keep_alive)),
                    M_server_fd(std::forward<M_SOCKET_TP>(M_other_Connect.M_server_fd))
                    {};
                    connectInfo& operator=(connectInfo&& M_other_Connect_Operator) 
                    noexcept {
                        if (this != &M_other_Connect_Operator) {
                            M_is_keep_alive = std::forward<bool>(M_other_Connect_Operator.M_is_keep_alive);
                            M_server_fd = std::forward<M_SOCKET_TP>(M_other_Connect_Operator.M_server_fd);
                        }
                        return *this;
                    }
                connectInfo(const connectInfo&) = delete;
                connectInfo& operator=(const connectInfo&) = delete;
            };

            stagdeer::ip::Ipv4addrs* M_resolver_Ipv4 = nullptr;
            stagdeer::ip::Ipv6addrs* M_resolver_Ipv6 = nullptr;
            std::unordered_map<M_SOCKET_TP, connectInfo> M_connects;
        };
    }
}

#endif