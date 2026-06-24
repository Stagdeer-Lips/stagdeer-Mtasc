#ifndef STAGDEER_CLIENT_TCP_SOCKET
#define STAGDEER_CLIENT_TCP_SOCKET

#include "../socket_header.h"
#include <algorithm>
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
#include <system_error>
#include <unistd.h>
#include <unordered_map>
#include <utility>


namespace stagdeer {
    namespace client {
        class socketTcp : public std::enable_shared_from_this<socketTcp> {
            public:

            socketTcp() = default;

            socketTcp(const char* M_hostname__ , uint16_t M_port__ , const char* M_message__) {
                M_client_context.M_addrs_host = M_hostname__;
                M_client_context.M_addrs_port = M_port__; 
                M_client_context.M_clientMessage = M_message__;
                return;
            }

            ~socketTcp() {
                delete (M_resolver_Ipv4);
                delete (M_resolver_Ipv6);
                return;
            }
            socketTcp(stagdeer::client::socketTcp&& other_tcp) noexcept: 
                M_client_context(std::move(other_tcp.M_client_context)) , 
                    M_resolver_Ipv4(other_tcp.M_resolver_Ipv4),
                    M_resolver_Ipv6(other_tcp.M_resolver_Ipv6),
                    M_connects(std::forward<std::unordered_map<M_SOCKET_TP, struct connectInfo>>(other_tcp.M_connects)){};
                socketTcp(const socketTcp&) = delete;
                socketTcp& operator=(const socketTcp&) = delete;
                socketTcp& operator=(socketTcp&& other_tcp_operator) 
                    noexcept {
                        if (this != &other_tcp_operator) {
                            M_connects = std::forward<std::unordered_map<M_SOCKET_TP, struct connectInfo>>(other_tcp_operator.M_connects);
                            M_client_context = std::forward<struct client_context>
                            (other_tcp_operator.M_client_context);
                            M_resolver_Ipv4 = other_tcp_operator.M_resolver_Ipv4;
                            M_resolver_Ipv6 = other_tcp_operator.M_resolver_Ipv6;
                        }
                        return *this;
                    }

                struct client_context {
                    const char* M_addrs_host;
                    uint16_t M_addrs_port = 80;
                    struct addrinfo* M_resovler_addrs;
                    std::string M_clientMessage;
                    int M_clientMessageLength;
                    M_SOCKET_TP M_socketfd;
                    uint16_t M_timeout = 20;
                    bool M_is_enable_ipV6 = false;
                    bool M_this_addr_invalid = false;
                    int M_connect_rety_count = 0;
                    int M_max_rety_count = 0;
                    size_t M_client_retry_write_bytes = 0;
                    size_t M_client_write_bytes = 0;
                    bool M_is_retry_success = false;
                    std::shared_ptr<stagdeer::client::readBuffer> M_client_read_buffer;

                    client_context() = default;
                    client_context(client_context&& other_addrs) 
                        noexcept: M_addrs_host(std::forward<const char*>(other_addrs.M_addrs_host)),
                            M_client_retry_write_bytes(std::move(other_addrs.M_client_retry_write_bytes)),
                            M_client_write_bytes(std::move(other_addrs.M_client_write_bytes)),
                            M_resovler_addrs(other_addrs.M_resovler_addrs),
                            M_addrs_port(std::forward<uint16_t>(other_addrs.M_addrs_port)),
                            M_clientMessage(std::move(other_addrs.M_clientMessage)),
                            M_clientMessageLength(std::forward<size_t>(other_addrs.M_clientMessageLength)),
                            M_is_enable_ipV6(std::forward<bool>(other_addrs.M_is_enable_ipV6)),
                            M_timeout(std::forward<uint16_t>(other_addrs.M_timeout)),
                            M_this_addr_invalid(std::forward<bool>(other_addrs.M_this_addr_invalid)),
                            M_is_retry_success(std::forward<bool>(other_addrs.M_is_retry_success)),
                            M_socketfd(std::forward<M_SOCKET_TP>(other_addrs.M_socketfd)),
                            M_max_rety_count(std::forward<int>(other_addrs.M_max_rety_count)),
                            M_client_read_buffer(std::move(other_addrs.M_client_read_buffer)) 
                            {};
                    client_context& operator=(client_context&& other_addrs) {
                        if (this != &other_addrs) {
                            M_client_retry_write_bytes = std::move(other_addrs.M_client_retry_write_bytes);
                            M_max_rety_count = std::forward<int>(other_addrs.M_max_rety_count);
                            M_socketfd = std::forward<M_SOCKET_TP>(other_addrs.M_socketfd);
                            M_addrs_host = std::forward<const char*>(other_addrs.M_addrs_host);
                            M_addrs_port = std::forward<uint16_t>(other_addrs.M_addrs_port);
                            M_resovler_addrs = other_addrs.M_resovler_addrs;
                            M_client_write_bytes = other_addrs.M_client_write_bytes;
                            M_clientMessage = std::move(other_addrs.M_clientMessage);
                            M_clientMessageLength = std::forward<size_t>(other_addrs.M_clientMessageLength);
                            M_is_enable_ipV6 = std::forward<bool>(other_addrs.M_is_enable_ipV6);
                            M_timeout = std::forward<uint16_t>(other_addrs.M_timeout);
                            M_is_retry_success = std::forward<bool>(other_addrs.M_is_retry_success);
                            M_this_addr_invalid = std::forward<bool>(other_addrs.M_this_addr_invalid);
                            M_client_read_buffer = std::move(other_addrs.M_client_read_buffer);
                        }
                        return *this;
                    };
                    client_context(const client_context&) = default;
                    client_context& operator=(const client_context&) = default;
                };

                struct client_context getClientContext() {
                    return M_client_context;
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
                            Tp, const std::error_code&, struct stagdeer
                                ::client::socketTcp::client_context&&>
                        ::__M_ret_lmdba, void>
                    ::__is_M_ret_Tp
                >::type
                async_resolver_domain(
                    Tp&& callback_token,
                    struct client_context&& M_context_ , 
                    uint16_t M_timeout = 2000 ,
                    bool M_reuse_addr = true,
                    bool M_enable_Ipv6 = false,
                    int M_max_ret_count = 10
                ) noexcept {
                    if (M_context_.M_addrs_port == 0 || M_context_.M_addrs_host == nullptr) {
                        std::error_code new_ec = std::make_error_code(std::errc::invalid_argument);
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) ,
                                    new_ec , std::move(M_context_));
                        return -1;
                    }
                    //ADD CONFIG TO ADDRS STRUCT
                    M_context_.M_timeout = M_timeout;
                    M_context_.M_is_enable_ipV6 = M_enable_Ipv6;
                    M_context_.M_max_rety_count = M_max_ret_count;
                        //CREATE NEW SOSKER
                        //RESOLVER DOMAIN
                        if (M_enable_Ipv6) {
                            //RESOVLER IPV6
                          M_resolver_Ipv6 = new stagdeer::
                          ip::Ipv6addrs(M_context_.M_addrs_host , M_context_.M_addrs_port);
                            try {
                                struct addrinfo* resolver_result = M_resolver_Ipv6->getResolverResult();
                                if (!resolver_result) {
                                    #ifdef STAGDEER_GNU_LINUX
                                        std::error_code new_ec(errno , std::system_category());
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) ,
                                                 new_ec , std::move(M_context_));
                                        return -1;
                                    #else 
                                        //TODO: WINDOWS ERROR HANDLERS
                                    #endif
                                }
                                //CREATE SOCKET
                                M_context_.M_socketfd = socket(resolver_result->ai_family, 
                                    resolver_result->ai_socktype, resolver_result->ai_protocol);
                                //ADD TO THREAD TASK
                                if (M_context_.M_socketfd == -1) {
                                    std::error_code new_ec = std::make_error_code(std::errc::bad_file_descriptor);
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) ,new_ec,
                                            std::move(M_context_));
                                        return -1;
                                }
                                M_context_.M_resovler_addrs = resolver_result;
                                std::error_code new_ec;
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token), new_ec,
                                      std::move(M_context_));
                                return 1;
                            } catch (std::exception& M_err) {
                                //RESOLVER FAILED
                                std::error_code new_ec = std::make_error_code(std::errc::io_error);
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token), new_ec , 
                                        std::move(M_context_));
                                return -1;
                            }
                            return -1;
                        }
                        //RESOLVER IPV4
                        M_resolver_Ipv4 = new stagdeer::
                            ip::Ipv4addrs(M_context_.M_addrs_host , M_context_.M_addrs_port);
                        try {
                            struct addrinfo* resolver_result = M_resolver_Ipv4->getResolverResult();
                            if (!resolver_result) {
                                #ifdef STAGDEER_GNU_LINUX
                                std::error_code ec(errno , std::system_category());
                                    M_threadManager.getThreadManager()
                                            .asyncTaskvoid(std::move(callback_token) , 
                                            ec, std::move(M_context_));
                                    return -1;
                                #else 
                                    //TODO: WINDOWS ERROR HANDLERS
                                #endif
                            }
                            //CREATE SOCKET
                            M_context_.M_socketfd = socket(resolver_result->ai_family, 
                                resolver_result->ai_socktype, resolver_result->ai_protocol);
                            //ADD TO THREAD TASK
                            if (M_context_.M_socketfd == -1) {
                                std::error_code new_ec = std::make_error_code(std::errc::bad_file_descriptor);
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token) , new_ec,
                                    std::move(M_context_));
                                return -1;
                            }
                            M_context_.M_resovler_addrs = resolver_result;
                            std::error_code new_ec;
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) ,new_ec,
                                 std::move(M_context_));
                            return 1;
                        } catch (std::exception& M_err) {
                            //RESOLVER FAILED
                            std::error_code new_ec = std::make_error_code(std::errc::io_error);
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) ,new_ec,
                                 std::move(M_context_));
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
                            Tp&&, const std::error_code& , struct stagdeer
                                ::client::socketTcp::client_context&&>::__M_ret_lmdba, void 
                    >::__is_M_ret_Tp
                >::type
                async_try_connect_tcp(
                    Tp&& callback_token,
                    struct client_context&& M_context_
                ) noexcept {
                        if (M_context_.M_addrs_host == nullptr || M_context_.M_resovler_addrs == nullptr) {
                            std::error_code new_ec = std::make_error_code(std::errc::bad_file_descriptor);
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token), new_ec ,
                                  std::move(M_context_));
                            return -1;
                        }
                            //VERIFIYCATION FD
                            if (M_context_.M_socketfd == -1) {
                                //INVALID FD   
                            std::error_code new_ec = std::make_error_code(std::errc::bad_file_descriptor);
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token), new_ec ,
                                  std::move(M_context_));
                                return -1;
                            }
                            //VERIFIYCATION INVALID IPADDRS
                            struct client_context try_context_ = M_tryIpaddrs(M_context_);
                            if (try_context_.M_this_addr_invalid) {
                               // ALL ADDRS INVALID
                               std::error_code ec = std::make_error_code(std::errc::address_not_available);
                                M_context_ = std::move(try_context_);
                                M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(callback_token) ,
                                ec , std::move(M_context_));
                               return -1;
                            }
                            //CONNECTION RESOLVER RESULT ADDRS
                           M_context_ = std::move(M_context_);
                           socket_setFcntl(M_context_.M_socketfd);
                            int M_connectRet = connect(M_context_.M_socketfd,M_context_.M_resovler_addrs->ai_addr, 
                            M_context_.M_resovler_addrs->ai_addrlen);
                                if (M_connectRet < 0) {
                                    //ERROR HANDLER

                                    //SETTING TIMEOUT
                                    fd_set M_writefds;
                                    FD_ZERO(&M_writefds);
                                    FD_SET(M_context_.M_socketfd, &M_writefds);

                                    struct timeval M_tv;
                                    M_tv.tv_sec = M_context_.M_timeout;
                                    M_tv.tv_usec = 0;

                                    #ifdef STAGDEER_GNU_LINUX
                                    int M_select_ret = select(M_context_.M_socketfd + 1, NULL , &M_writefds, NULL, &M_tv);
                                    if (M_select_ret < 0) {
                                        int errno_copy = errno;
                                        std::error_code ec = std::make_error_code(std::errc::timed_out);
                                        if (errno_copy == ECONNREFUSED || errno_copy == ENETUNREACH || errno_copy == EHOSTUNREACH) {
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) , 
                                                ec,std::move(M_context_));
                                            return -1;
                                        }
                                        struct client_context M_retry_ctx = 
                                            M_retryTcpConnect(M_context_ , errno_copy);
                                        M_context_ = std::move(M_retry_ctx);
                                        if (M_context_.M_is_retry_success) {
                                            //RETRY SUCCESS
                                            std::error_code ec;
                                            M_threadManager.getThreadManager()
                                                .asyncTaskvoid(std::move(callback_token) ,
                                                ec ,
                                                 std::move(M_context_));
                                            return 2;
                                        }
                                        //RETRY FAILED
                                    #else
                                        //TODO: WINDOWS ERROR HEANDLER
                                    #endif
                                    std::error_code ec_(errno , std::system_category());
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) , ec_ , std::move(M_context_));
                                    return -1;
                                } else if (M_select_ret == 0) {
                                    std::error_code ec_ = std::make_error_code(std::errc::connection_reset);
                                    M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(callback_token) , ec_ ,
                                         std::move(M_context_));
                                    return -1;
                                }
                            }
                            //CONNECT SUCCESS
                            std::error_code ec;
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(callback_token) , ec
                                 , std::move(M_context_));
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
                            Tp, const std::error_code& , size_t , 
                                struct client_context&&>::__M_ret_lmdba , void 
                    >::__is_M_ret_Tp
                >::type
                async_write(
                    Tp&& callback_token, 
                    struct client_context&& M_context__,
                    const std::string& M_message__
                )
                noexcept {
                    if (M_context__.M_socketfd < 0 || M_context__.M_this_addr_invalid 
                        || M_context__.M_resovler_addrs == nullptr || M_message__.empty()) {
                            //PARAMPER INVLIAD
                            std::error_code ec = std::make_error_code(std::errc::bad_file_descriptor);
                            M_threadManager.getThreadManager()
                                .asyncTaskvoid(callback_token , ec , size_t(0) , 
                                    std::move(M_context__)
                                );
                        return -1;
                    }
                    
                    //UPDATE CLIENT CONFIGURE
                    size_t M_message_lenght = strlen(M_message__.c_str());
                    M_context__.M_clientMessage = M_message__;
                    M_context__.M_clientMessageLength = M_message_lenght;

                    M_threadManager.getThreadManager()
                        .asyncTaskvoid([self = shared_from_this() ,
                            M_callback_token__ = std::function<void(const std::error_code& , size_t ,
                                 struct stagdeer::client::socketTcp::client_context&&)>
                                 (std::move(callback_token)) ,
                                  M_context__ = std::move(M_context__)]() 
                            mutable -> void {

                            //TRY WRITE MESSAGE
                            int M_written = 0;
                            int M_total_write = 0;
                            while (M_written < strlen(M_context__.M_clientMessage.c_str())) {
                                int M_write_ret = write(M_context__.M_socketfd, 
                                    M_context__.M_clientMessage.c_str() + M_written, 
                                    strlen(M_context__.M_clientMessage.c_str()) - M_written
                                );

                                if (M_write_ret < 0) {
                                    //WRITE FAILED
                                    struct client_context M_retry_write_ctx = 
                                        self -> M_retryTcpwrite(M_context__, M_write_ret, 
                                            strlen(M_context__.M_clientMessage.c_str()), errno);
                                        if (M_retry_write_ctx.M_is_retry_success) {
                                            //RETRY SUCCESS
                                            if (M_retry_write_ctx.M_client_write_bytes == 0) {
                                                //FALSE RETRY SUCCESS
                                                std::error_code ec = std::make_error_code(std::errc::bad_address);
                                                self -> M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__), ec ,
                                                        size_t(M_total_write) , std::move(M_context__)
                                                    );
                                                return;
                                            } else {
                                                //TRUE RETRY SUCCESS
                                                std::error_code ec;
                                                self -> M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__), ec ,
                                                     M_retry_write_ctx.M_client_write_bytes + M_total_write ,
                                                        std::move(M_retry_write_ctx)
                                                    );
                                                return;
                                            }
                                        }
                                    //RETRY FAILED
                                    std::error_code ec = std::make_error_code(std::errc::io_error);
                                    self -> M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(M_callback_token__), ec,
                                            size_t(M_total_write + M_total_write) , std::move(M_context__)
                                        );
                                    return;
                                }

                                //UPDATE MESSAGE
                                M_total_write += M_write_ret;
                                M_written += M_write_ret;
                                if (M_total_write == strlen(M_context__.M_clientMessage.c_str())) {
                                    //QUIT WHILE LOOP WRITE
                                    break;
                                }
                                //CONTINUE
                            } 
                            //WRITE SUCCESS
                            if (M_total_write != strlen(M_context__.M_clientMessage.c_str())) {
                                //WRITE INCOMPLETE
                                std::error_code ec;
                                self -> M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(M_callback_token__), ec ,
                                        size_t(M_total_write), std::move(M_context__)
                                    );
                                return;
                            }

                            std::error_code ec;
                            self -> M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(M_callback_token__), ec
                                 , size_t(M_total_write) ,
                                    std::move(M_context__)
                                );
                            return;
                        });
                    return -1;
                }

            /**
            //STATUS CODE
            @param 0 CLOSED
            @param 1 READ SUCCESS
            @param 2 RETRY READ SUCCESS
            @param -1 READ FAILED
            */
            template<typename Tp>
            typename stagdeer::util::lamdba_trais::constraint<
                stagdeer::util::lamdba_trais::M_is_retTp<
                    typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp <
                        Tp, const std::error_code& , size_t , std::shared_ptr<
                            stagdeer::client::readBuffer
                        >&& , struct client_context&&
                    >::__M_ret_lmdba                
                , void>::__is_M_ret_Tp
            >::type
            async_read_until(
                Tp&& callback_token, 
                client_context&& M_context_,
                const std::string& M_delimiter
            ) noexcept {
                //CREATE BUFFER
                //ADD UNITL READ TASK TO THREAD
                std::shared_ptr<stagdeer::client::readBuffer> M_recv_buffer = 
                    std::make_shared<stagdeer::client::readBuffer>();
                    M_context_.M_client_read_buffer = std::move(M_recv_buffer);
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([self = shared_from_this() , 
                        M_callback_token__ = std::function<void(const std::error_code&,
                            size_t ,std::shared_ptr<stagdeer::client::readBuffer>&& ,
                             struct client_context&&)>(std::move(callback_token)),
                             M_context_ = std::move(M_context_), M_delimiter]()
                             mutable {
                                //TRY READ
                                enum M_READ_STATE {
                                    VERIFY,
                                    READING,
                                    FOUND,
                                    ERROR,
                                    CLOSE
                                };
                                //READ STATUS
                                  M_READ_STATE M_newState = VERIFY;
                                //CREARTE READ FUNCTION
                                while (M_newState != FOUND) {
                                    switch (M_newState) {
                                        case VERIFY: {                      
                                            //VERIFIYCATION TOALDATA
                                            bool M_found = false;
                                            const char* M_Tolaldata = M_context_.M_client_read_buffer->peekData();
                                            size_t M_readableBytes = M_context_.M_client_read_buffer->readableBytes();
                                            for (int M_read_pointer = 0; M_read_pointer + M_delimiter.size()
                                                <= M_readableBytes; ++ M_read_pointer) {
                                                if (memcmp(M_Tolaldata + M_read_pointer ,
                                                    M_delimiter.c_str(), M_delimiter.size()) == 0) {
                                                        M_newState = FOUND;
                                                        M_found = true;
                                                    break;
                                                }
                                            }

                                            if (M_found) {
                                                break;
                                            }

                                            M_newState = READING;
                                        };

                                        case READING: {
                                            //SETTING TIMEOUT
                                            fd_set M_readtimeout_set;
                                            FD_ZERO(&M_readtimeout_set);
                                            FD_SET(M_context_.M_socketfd, &M_readtimeout_set);

                                            struct timeval M_tv;
                                            M_tv.tv_sec = M_context_.M_timeout;
                                            M_tv.tv_usec = 0;

                                            //WIAT DATA
                                            int M_wiat_ret = select(M_context_.M_socketfd + 1 , &M_readtimeout_set, NULL , NULL , &M_tv);

                                            if (M_wiat_ret < 0) {
                                                M_newState = ERROR;
                                                break;
                                            }
                                            
                                            char M_cache_buffer[4096];
                                            int M_read_bytes = recv(M_context_.M_socketfd, M_cache_buffer, 
                                                sizeof(M_cache_buffer), 0);
                                                if (M_read_bytes < 0) {
                                                    //READ ERROR
                                                    M_newState = ERROR;
                                                    break;
                                                }
                                            
                                            //ADD TO BUFFER
                                            if (M_read_bytes > 0) {
                                                M_context_.M_client_read_buffer->appendTobuffer(M_cache_buffer, M_read_bytes);
                                                M_newState = VERIFY;
                                            }

                                            if (M_read_bytes == 0) {
                                                //CONNECT CLOSE
                                                M_newState = CLOSE;
                                                break;
                                            }
                                            
                                        };

                                        case ERROR: {
                                            //ERROR HANDLER
                                            //RETRY
                                            struct client_context M_retry_result = self->M_retryTcpRead_until(M_context_, M_delimiter ,errno);
                                            M_context_ = std::move(M_retry_result);
                                            if (M_retry_result.M_is_retry_success) {
                                                //RETRY SUCCESS
                                                std::error_code ec;
                                                self->M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__), ec , 
                                                    size_t(M_context_.M_client_read_buffer->readableBytes()), 
                                                    std::move(M_context_.M_client_read_buffer) , std::move(M_context_)
                                                );
                                                return;
                                            } else { 
                                                //RETRY FAILED
                                                std::error_code ec(errno , std::system_category());
                                                self->M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__), 
                                                    ec, size_t(0) , 
                                                        std::move(M_context_.M_client_read_buffer) , std::move(M_context_)
                                                    );
                                                return;
                                            }
                                        };

                                        case FOUND: {
                                            break;
                                        };

                                        case CLOSE: {
                                            if (M_context_.M_client_read_buffer->readableBytes() > 0) {
                                                //CLOSED BUT WITH DATA
                                                std::error_code ec;
                                                self->M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__) , ec , 
                                                    size_t(M_context_.M_client_read_buffer->readableBytes()) , 
                                                    std::move(M_context_.M_client_read_buffer) , 
                                                    std::move(M_context_)
                                                );
                                                return;
                                            } else {
                                                std::error_code ec;
                                                self->M_threadManager.getThreadManager()
                                                    .asyncTaskvoid(std::move(M_callback_token__),
                                                    ec , 0 ,
                                                      std::move(M_context_.M_client_read_buffer) ,
                                                       std::move(M_context_)
                                                );
                                                return;
                                            }
                                        };

                                    };
                                }
            
                                std::error_code ec;
                                self->M_threadManager.getThreadManager()
                                    .asyncTaskvoid(std::move(M_callback_token__), ec  , size_t(M_context_.M_client_read_buffer->readableBytes()) ,
                                        std::move(M_context_.M_client_read_buffer) , std::move(M_context_)
                                    );
                                return;
                            });
                return 1;
            }

            /**
            //STATUS CODE
            @param 0 CLOSED
            @param 1 READ SUCCESS
            @param 2 RETRY SUCCESS
            @param -1 READ FAILED
            @Param -2 READ TIMEOUT
            */

            template<typename Tp>
            typename stagdeer::util::lamdba_trais::constraint<
                stagdeer::util::lamdba_trais::M_is_retTp<
                    typename stagdeer::util::lamdba_trais::M_get_lamdba_ret_Tp<
                        Tp, const std::error_code& ,size_t , std::shared_ptr<
                            stagdeer::client::readBuffer
                        >&& , struct client_context&&
                    >::__M_ret_lmdba, void
                >::__is_M_ret_Tp
            >::type
            async_read (
                Tp&& callback_token,
                std::shared_ptr<stagdeer::client::readBuffer>&& M_readBuffer,
                client_context&& M_context_,
                size_t M_chache_size
            ) noexcept {
                M_context_.M_client_read_buffer = std::move(M_readBuffer);
                if (M_chache_size <= 0) {
                    M_chache_size = 4096;
                }
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([M_context_ = std::move(M_context_) , 
                        M_chache_size , M_callback_token__ = 
                        std::function<void(const std::error_code&, size_t,
                            std::shared_ptr<stagdeer::client::readBuffer>&& , 
                                struct client_context&&)>(std::move(callback_token)) , 
                                self = shared_from_this()
                        ]()
                        mutable{
                        //BASIC STATE
                        enum BASIC_READ_STATE {
                            READING,
                            CLOSE,
                            ERROR,
                            VERIFY,
                            DONE
                        };
                        BASIC_READ_STATE M_state = READING;
                        while (true) {
                            switch (M_state) {
                                case READING: {
                                //BASIC READ TASK
                                //SETTING TIMEOUT
                                    fd_set M_readtimouet_set;
                                    FD_ZERO(&M_readtimouet_set);
                                    FD_SET(M_context_.M_socketfd, &M_readtimouet_set);

                                    struct timeval M_tv;
                                    M_tv.tv_sec = M_context_.M_timeout;
                                    M_tv.tv_usec = 0;
                                    //WIAT DATA
                                    int M_wiat_ret = select(M_context_.M_socketfd + 1, 
                                        &M_readtimouet_set, nullptr, 
                                        nullptr, &M_tv);
                                    if (M_wiat_ret < 0) {
                                        //TIMOUT
                                        M_state = ERROR;
                                        break;
                                    }
                                    //TRY READ
                                    char M_cache_buffer[4096];
                                    int M_read_ret = recv(M_context_.M_socketfd, 
                                        M_cache_buffer, sizeof(M_cache_buffer), 0);
                                        if (M_read_ret < 0) {
                                            //ERROR
                                            M_state = ERROR;
                                            break;
                                        }
                                    if (M_read_ret > 0) {
                                        //APPEND TO BUFFER
                                        M_context_.M_client_read_buffer->appendTobuffer(M_cache_buffer,
                                                M_read_ret);
                                        M_state = VERIFY;
                                        break;
                                    }
                                    if (M_read_ret == 0) {
                                        M_state = CLOSE;
                                        break;
                                    }
                                };
                                
                                //READING END
                                
                                case CLOSE: {
                                    if (M_context_.M_client_read_buffer->readableBytes() > 0) {
                                        std::error_code ec;
                                        self->M_threadManager.getThreadManager()
                                            .asyncTaskvoid(std::move(M_callback_token__), ec ,
                                            size_t(M_context_.M_client_read_buffer->readableBytes()),
                                                std::move(M_context_.M_client_read_buffer) , 
                                            std::move(M_context_)
                                        );
                                        return; 
                                    } else {
                                        std::error_code ec;
                                        self->M_threadManager.getThreadManager()
                                            .asyncTaskvoid(std::move(M_callback_token__), ec,
                                                size_t(0) , std::move(M_context_.M_client_read_buffer)
                                                , std::move(M_context_)
                                            );
                                        return;
                                    }
                                }

                                //CLOSE END
                                
                                case VERIFY: {
                                    //VERIFY
                                    if (M_context_.M_client_read_buffer->readableBytes() >= M_chache_size) {
                                        //CHANGE STATUS
                                        M_state = DONE;
                                        break;;
                                    }
                                    //CONTINUE
                                    M_state = READING;
                                    break;
                                }

                                //VERIFY END

                                case ERROR: {
                                    if (errno == 106) {
                                        std::error_code ec;
                                        self->M_threadManager.getThreadManager()
                                            .asyncTaskvoid(
                                                std::move(M_callback_token__) , ec ,size_t(0),
                                                std::move(M_context_.M_client_read_buffer) , 
                                                std::move(M_context_)
                                        );
                                        return;
                                    }
                                    struct client_context M_retry_read = 
                                        self->M_retryTcpRead(M_context_, 
                                            errno , M_chache_size);
                                    if (M_retry_read.M_is_retry_success) {
                                        //RETRY SUCCESS
                                        std::error_code ec;
                                        M_context_ = std::move(M_retry_read);
                                        self->M_threadManager.getThreadManager()
                                            .asyncTaskvoid(std::move(M_callback_token__), 
                                                ec, size_t(M_context_.M_client_read_buffer->readableBytes()),
                                            std::move(M_context_.M_client_read_buffer),
                                            std::move(M_context_)
                                        );
                                        return;
                                    }
                                    //RETRY FAILED
                                    std::error_code ec(errno , std::system_category());
                                    self->M_threadManager.getThreadManager()
                                            .asyncTaskvoid(
                                                std::move(M_callback_token__), ec,
                                                 size_t(0) , std::move(M_context_.M_client_read_buffer) ,
                                            std::move(M_context_)
                                        );
                                    return;
                                };

                                //ERROR END

                                case DONE: {
                                    std::error_code ec;
                                    self->M_threadManager.getThreadManager()
                                        .asyncTaskvoid(std::move(M_callback_token__) ,
                                        ec, 
                                        size_t(M_context_.M_client_read_buffer->readableBytes()),
                                        std::move(M_context_.M_client_read_buffer),
                                        std::move(M_context_)
                                    );

                                    return;
                                };

                            }
                        }
                        //WHILE END

                    }
                );
                //TASK END
                return -1;  
            }

            private:
            struct client_context M_client_context;
            stagdeer::THREAD& M_threadManager = stagdeer::THREAD::getInstance();

            #ifdef STAGDEER_GNU_LINUX

                struct client_context M_retryTcpRead(struct client_context M_context__ , int M_errno_copy , 
                    size_t M_chache_size) {
                    if (M_errno_copy == 106) {
                        M_context__.M_is_retry_success = true;
                        return M_context__;
                    }
                    if (M_context__.M_max_rety_count <= 0) {
                        M_context__.M_max_rety_count = 10;
                    }

                    M_context__.M_connect_rety_count = 0;
                    while (M_context__.M_connect_rety_count < M_context__.M_max_rety_count) {
                        M_errno_copy = errno; //UPDATE ERRNO
                        if (M_errno_copy == EAGAIN || M_errno_copy == EINTR ) {
                            //RETRY READ
                            SLEEP(1000 * M_context__.M_connect_rety_count);
                            char M_retry_read_buffer[1096];
                            int M_read_ret = recv(M_context__.M_socketfd, M_retry_read_buffer, sizeof(M_retry_read_buffer), 0);
                                M_context__.M_connect_rety_count ++;
                                
                                if (M_read_ret < 0) {
                                    //READ FAILED
                                    if (M_context__.M_max_rety_count >= M_context__.M_connect_rety_count) {
                                        M_context__.M_is_retry_success = false;
                                        return M_context__;
                                    }
                                    
                                    //CONTINUE
                                    continue;
                                }

                                if (M_read_ret > 0) {
                                    //APPEDN TO BUFFER
                                    M_context__.M_client_read_buffer->appendTobuffer(M_retry_read_buffer, M_read_ret);
                                        //VERIFY BYTES
                                        if (M_read_ret < M_chache_size && M_context__.M_max_rety_count > 
                                            M_context__.M_connect_rety_count) {
                                            //CONTINUE READ
                                            continue;
                                        } else {
                                            if (M_read_ret >= M_chache_size || M_read_ret < M_chache_size) {
                                                //SUCCESS
                                                M_context__.M_is_retry_success = true;
                                                return M_context__;
                                            }
                                        }
                                }

                                if (M_read_ret == 0) {
                                    //CONNECTION CLOSED
                                    if (M_context__.M_client_read_buffer->readableBytes() >= 0) {
                                        M_context__.M_is_retry_success = true;
                                        return M_context__;
                                    }
                                    M_context__.M_is_retry_success = false;
                                    return M_context__;
                                }
                        }
                        //CANOT RETRY
                        M_context__.M_is_retry_success = false;
                        return M_context__;
                    }

                    M_context__.M_is_retry_success = false;
                    return M_context__;
                }


                struct client_context M_retryTcpRead_until(struct client_context M_context__ , 
                    const std::string& M_delimiter , int M_errno_copy) {
                    if (M_context__.M_socketfd < 0) {
                        throw std::runtime_error("Invalid socket!");
                    }
                    if (M_context__.M_max_rety_count <= 0) {
                        M_context__.M_max_rety_count = 10;
                    }
                    //CLEARN RETRY COUNT
                    M_context__.M_connect_rety_count = 0;
                    //UPDATE ERROR
                    SLEEP(1000 * M_context__.M_connect_rety_count);
                    while (M_context__.M_connect_rety_count < M_context__.M_max_rety_count) {
                    M_errno_copy = errno;
                    if (M_errno_copy == 106) {
                        //ALREADY CONNECT BUG
                        M_context__.M_is_retry_success = true;
                        return M_context__;
                    }
                        if (M_errno_copy == EAGAIN || M_errno_copy == EINTR) {
                            //RETRY
                            //ADD RETRY COUNT
                            M_context__.M_connect_rety_count ++;
                            size_t M_readableBytes = M_context__.M_client_read_buffer->readableBytes();
                            for (int M_index_ = 0; M_index_ + M_delimiter.size() <= M_readableBytes; 
                                ++ M_index_) {
                                    if (memcmp(M_context__.M_client_read_buffer->peekData() + M_index_,
                                         M_delimiter.c_str(), M_delimiter.size()) == 0) {
                                            //RETRY SUCCESS
                                            M_context__.M_is_retry_success = true;
                                        return M_context__;
                                    }
                            }

                            char M_cache_buffer[1086];
                            int M_recv_bytes = read(M_context__.M_socketfd, M_cache_buffer, sizeof(M_cache_buffer));
                                if (M_recv_bytes < 0) {
                                    if (M_context__.M_connect_rety_count >= M_context__.M_max_rety_count) {
                                        M_context__.M_is_retry_success = false;
                                        return M_context__;
                                    }

                                    //CONTINUE RETRY
                                    continue;
                                }

                                if (M_recv_bytes == 0) {
                                    //CONNECT CLOSE
                                    if (M_context__.M_client_read_buffer->readableBytes() != 0) {
                                        M_context__.M_is_retry_success = true;
                                        return M_context__;
                                    }
                                    M_context__.M_is_retry_success = false;
                                    return M_context__;
                                }
                            if (M_recv_bytes > 0) {
                                //GO TO VERIFY
                                //ADD DATA TO BUFFER
                                M_context__.M_client_read_buffer->appendTobuffer(M_cache_buffer, M_recv_bytes);
                                continue;
                            }
                        } else {
                            //CANOT RETRY
                            M_context__.M_is_retry_success = false;
                            return M_context__;
                        }
                    }
                    M_context__.M_is_retry_success = false;
                    return M_context__;
                }

                struct client_context M_retryTcpConnect(struct client_context M_context__ , int M_errno_copy) {
                    if (M_context__.M_max_rety_count <= 0) {
                        M_context__.M_max_rety_count = 10;
                    }
                    while (M_context__.M_connect_rety_count < M_context__.M_max_rety_count) {
                    M_errno_copy = errno;
                    if (M_errno_copy == 106) {
                        //ALREADY CONNECTION
                        M_context__.M_is_retry_success = true;
                        return M_context__;
                    }
                        if (
                            M_errno_copy == EAGAIN || 
                            M_errno_copy == EWOULDBLOCK || 
                            M_errno_copy == EINTR ||
                            M_errno_copy == ETIMEDOUT ||
                            M_errno_copy && M_context__.M_max_rety_count 
                                > M_context__.M_connect_rety_count) {
                            //RETRY TO
                            M_context__.M_connect_rety_count += 1; //RETY COUNT +1
                            SLEEP(1000 * M_context__.M_connect_rety_count);
                            int M_retry_result = connect(M_context__.M_socketfd, 
                            M_context__.M_resovler_addrs->ai_addr, 
                                M_context__.M_resovler_addrs->ai_addrlen);
                                if (M_retry_result < 0) {
                                    //RETRY FAILED
                                    if (M_context__.M_max_rety_count == M_context__.M_connect_rety_count) {
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
                                    M_context__.M_is_retry_success = true;
                                    return M_context__;
                                }
                            continue;
                        }
                        //NOT RETTY BREAK WHILE
                        break;
                    }
                    return M_context__;
                }

                struct client_context M_retryTcpwrite(struct client_context& M_context_ , 
                    size_t M_total_write , size_t M_strfull_len, int M_errno_copy) {
                        if (M_context_ .M_socketfd < 0 || M_context_ .M_this_addr_invalid 
                            || M_context_.M_resovler_addrs == nullptr) {
                                M_context_.M_is_retry_success = false;
                                return M_context_;
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
                            M_context_.M_connect_rety_count = 0;
                            while (M_context_.M_connect_rety_count < M_context_.M_max_rety_count) {
                                //ADD WRITE RETRY COUNT
                                M_context_.M_connect_rety_count ++;
                                SLEEP(1000 * M_context_.M_connect_rety_count);
                                const char* M_retry_char = M_context_.M_clientMessage.c_str();

                                //RETRY WRITE WHILE LOOP
                                int M_recv = 0;
                                while (M_total_write < M_strfull_len) {
                                    M_recv = write(M_context_.M_socketfd, 
                                        M_context_.M_clientMessage.c_str() + M_total_write,
                                         M_strfull_len - M_total_write);
                                    if (M_recv < 0) {
                                        if (M_context_.M_max_rety_count >= M_context_.M_connect_rety_count) {
                                            //RETRY FAILED
                                            M_context_.M_is_retry_success = false;
                                            return M_context_;
                                        }
                                        //CONTINUE RETRY
                                        continue;
                                    }

                                    if (M_recv + M_total_write == M_strfull_len) {
                                        //RETRY SUCCESS
                                        M_context_.M_is_retry_success = true;
                                        M_context_.M_client_retry_write_bytes = M_total_write;
                                        return M_context_;
                                    }

                                    //UPDATE MESSAGE
                                    M_total_write += M_recv;
                                    M_context_.M_client_retry_write_bytes = M_total_write;
                                }
                            }
                            //RETRY END
                            if (M_total_write <= M_strfull_len) {
                                //DATA COMPLETE
                                M_context_.M_is_retry_success = true;
                                M_context_.M_client_retry_write_bytes = M_total_write;
                                return M_context_;
                            }

                            //DATA NOT INCOMPLETE
                            if (M_total_write < M_strfull_len || M_total_write <= 0) {
                                //FAILED
                                M_context_.M_is_retry_success = false;
                                return M_context_;
                            }
                        }
                    //CANOT RETRY ERROR
                    M_context_.M_is_retry_success = false;
                    return M_context_;
                }
            #else
                //TODO: WINDOWS RETRY
            #endif

            size_t M_findChunkedsize(std::shared_ptr<stagdeer::client::readBuffer>& M_buffer__) {
                std::string M_toal_data = M_buffer__->peekData();
                int M_find_str_pointer = 0;
                size_t M_chunked_size_sizeT;
                while (M_find_str_pointer < M_toal_data.size()) {
                    /**
                        4\r\n
                        Wiki\r\n
                        5\r\n
                        pedia\r\n
                        E\r\n
                        in\r\n\r\nchunks.\r\n
                        0\r\n
                        \r\n
                    */
                    size_t M_find_first = M_toal_data.find("\r\n" , M_find_str_pointer);
                    if (M_find_first == std::string::npos) {
                        return -1;
                    }
                    std::string M_chunked_size_strT = M_toal_data.substr(M_find_str_pointer , M_find_first - M_find_str_pointer);
                    if (M_chunked_size_strT.empty()) {
                        return -1;
                    }
                    M_chunked_size_sizeT = std::stoul(M_chunked_size_strT , nullptr , 16);
                    M_buffer__->retrieveData(M_find_first + 2);
                }
                return M_chunked_size_sizeT;
            }

            struct client_context M_tryIpaddrs(struct client_context& M_context__) {
                if (M_context__.M_resovler_addrs == nullptr) {
                    throw std::runtime_error("'M_context__' is null pointer!");
                }
                //FOR RESOLVER RESULT
                for (struct addrinfo* M_resolver_result = M_context__.M_resovler_addrs; 
                    M_resolver_result != nullptr; M_resolver_result = 
                    M_resolver_result->ai_next) {
                        if (M_context__.M_is_enable_ipV6) {
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
                            M_context__.M_socketfd = std::move(M_test_fd);
                            return M_context__;
                        }
                        CLOSE_FD(M_test_fd);
                        continue;
                    }
                    //ALL ADDRS INVALID
                    M_context__.M_this_addr_invalid = true;
                return M_context__;
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

        using socketTcpPtrT = std::shared_ptr<stagdeer::client::socketTcp>;
    }
}

#endif