#ifndef STAGDEER_CLIENT_APPLICATION
#define STAGDEER_CLIENT_APPLICATION

#include <cstdint>
#include "../../thread/thread.h"
#include "../Parser/Urlparser.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <utility>
#include "../util/type_util.hpp"
#include "method.hpp"

typedef long TIMEOUT_T;

namespace stagdeer {
    namespace client {
        class clientTool : public std::enable_shared_from_this<clientTool> {
            public:
            struct client_parser_basic_url {
                std::string addrs_host;
                std::string addrs_path;
                uint16_t addrs_port;
                bool this_url_enable_tls;     
            };

            struct client_parser_query_url {
                std::string addrs_host;
                std::string addrs_query_path;
                uint16_t addrs_port;
                bool this_url_enable_tls;
                std::string addrs_basic_path;
                std::string full_path;
                std::map<std::string, std::string> query_paramter_map;            
            };

            clientTool() = default;
            ~clientTool() = default;
            clientTool(const stagdeer::client::clientTool&) = delete;
            clientTool(stagdeer::client::clientTool&& M_other_Tool) noexcept
                : M_threadManager(std::forward<stagdeer::THREAD&>(M_other_Tool.M_threadManager)),
                    M_parser_url(std::move(M_other_Tool.M_parser_url)){};
            clientTool& operator=(clientTool&& M_other_Tool_Operator) 
                noexcept{
                    if (this != &M_other_Tool_Operator) {
                        M_parser_url = std::move(M_other_Tool_Operator.M_parser_url);
                    }
                    return *this;
                };

            template<typename Tp>
            inline typename stagdeer::util::lamdba_trais::constraint<
                util::lamdba_trais::M_is_retTp <
                 typename util::lamdba_trais::M_get_lamdba_ret_Tp<
                    Tp , struct client_parser_basic_url
                    >::__M_ret_lmdba, void>
                 ::__is_M_ret_Tp
            >::type
            asyncParserBasicUri (
                Tp&& callback_token,
                const std::string& addrs_url
            ) noexcept(
                noexcept(callback_token(
                    std::declval<struct client_parser_basic_url>()
                ))
            ) {
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([self = shared_from_this() , addrs_url ,
                         M_callback__ = std::function<void(struct client_parser_basic_url)>(
                            std::move(callback_token))](){
                        struct client_parser_basic_url M_urlBuffer = 
                        self -> syncParserBasicUri(addrs_url);
                        self->M_threadManager.getThreadManager()
                            .asyncTaskvoid(std::move(M_callback__),
                             M_urlBuffer);
                        return;
                    });
                return 1;
            }

            template<typename Tp>
            inline typename stagdeer::util::lamdba_trais::constraint<
                util::lamdba_trais::M_is_retTp<
                    typename util::lamdba_trais::M_get_lamdba_ret_Tp<
                        Tp, struct client_parser_query_url>::__M_ret_lmdba, void
                >::__is_M_ret_Tp
            >::type
            asyncParserQueryUri(
                Tp&& callback_token ,
                const std::string& addrs_url
            ) noexcept(
                noexcept(callback_token(
                    std::declval<struct client_parser_query_url>()
                ))
            ) {
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([ M_callback_token__ = std::function
                    <void(struct client_parser_query_url)>(std::move(callback_token)) ,
                         self = shared_from_this() , addrs_url]
                        () mutable {
                           struct client_parser_query_url M_query_result_ = 
                            self->syncParserQueryUrl(addrs_url);
                             self->M_threadManager.getThreadManager()
                                .asyncTaskvoid(std::move(M_callback_token__), 
                                 M_query_result_
                            );
                        return;
                    }
                );
                return 1;
            }

            template<typename Tp>
            inline typename stagdeer::util::lamdba_trais::constraint<
                util::lamdba_trais::M_is_retTp
                    <typename util::lamdba_trais::M_get_lamdba_ret_Tp<
                        Tp , const std::string&
                    >::__M_ret_lmdba, void
                >::__is_M_ret_Tp
            >::type
            asyncCreateHttpv1Tmp(
                Tp&& callback_token,
                const std::string& addrs_host,
                const std::string& addrs_path,
                const std::string& addrs_body,
                httpMethod method,
                std::unordered_map<std::string, std::string> headers_map
            ) noexcept(
                noexcept(callback_token(
                    std::declval<const std::string&>()
                ))
            ) {
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([self = shared_from_this() , 
                        addrs_host,  addrs_path, addrs_body,
                        headers_map , method , M_callback_token__ = 
                        std::function<void(const std::string&)>
                        (std::move(callback_token))](){
                        std::string M_template = self->syncCreateHttpv1template(
                            addrs_host,
                            addrs_path,
                            addrs_body,
                            method,
                            headers_map
                        );
                        self->M_threadManager.getThreadManager()
                            .asyncTaskvoid(std::move(M_callback_token__),
                            M_template);
                        return;
                    });
                return 1;
            }

            static std::shared_ptr<stagdeer::client::clientTool> newClientTool() {
                return std::make_shared<stagdeer::client::clientTool>();
            }

            struct client_parser_basic_url syncParserBasicUri(const std::string& M_url__) {
                struct stagdeer::client::urlHttpParser::basicUrlParserResult M_parser_url_result;
                  stagdeer::client::urlHttpParser M_newpar = stagdeer::client::urlHttpParser(M_url__);
                    M_parser_url_result = M_newpar.parserBasicUrl();
                    struct stagdeer::client::clientTool::client_parser_basic_url M_client_parser_result;
                        M_client_parser_result.addrs_host = std::move(M_parser_url_result.addrs_host);
                        M_client_parser_result.addrs_path = std::move(M_parser_url_result.addrs_path);
                        M_client_parser_result.addrs_port = M_parser_url_result.addrs_port;
                        M_client_parser_result.this_url_enable_tls = M_parser_url_result.is_enable_tls;
                    return M_client_parser_result;
                }
            
            struct client_parser_query_url syncParserQueryUrl(const std::string& M_url__) {
                struct stagdeer::client::urlHttpParser::queryUrlParserResult M_parser_url_result;
                 stagdeer::client::urlHttpParser M_newpar = stagdeer::client::urlHttpParser(M_url__);
                  M_parser_url_result = M_newpar.parserQueryUrl();
                  struct stagdeer::client::clientTool::client_parser_query_url M_client_parser_result;
                    M_client_parser_result.addrs_query_path = std::move(M_parser_url_result.query_path);
                    M_client_parser_result.addrs_basic_path = std::move(M_parser_url_result.basic_path);
                    M_client_parser_result.query_paramter_map = std::move(M_parser_url_result.query_paramter_map);
                    M_client_parser_result.full_path = M_client_parser_result.addrs_basic_path + M_client_parser_result.addrs_query_path;
                    M_client_parser_result.addrs_host = std::move(M_parser_url_result.addrs_host);
                    M_client_parser_result.addrs_port = M_parser_url_result.addrs_port;
                    M_client_parser_result.this_url_enable_tls = M_parser_url_result.is_enable_tls;
                return M_client_parser_result;
            }
                
            std::string syncCreateHttpv1template (
                const std::string& addrs_host,
                const std::string& addrs_path,
                const std::string& addrs_body,
                httpMethod method,
                std::unordered_map<std::string, std::string> headers_map
            );
            private:
            struct client_parser_basic_url M_parser_url;
            stagdeer::THREAD& M_threadManager = stagdeer::THREAD::getInstance();
        };
    using clientToolPtr = std::shared_ptr<stagdeer::client::clientTool>;
    using urlQueryMapT = std::map<std::string, std::string>;
    using clientHttpv1TmpHeadersT = std::unordered_map<std::string, std::string>;
    }
};

#endif