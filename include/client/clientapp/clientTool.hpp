#ifndef STAGDEER_CLIENT_APPLICATION
#define STAGDEER_CLIENT_APPLICATION

#include <cstdint>
#include "../../thread/thread.h"
#include <functional>
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
            struct client_parser_url {
                std::string addrs_host;
                std::string addrs_path;
                uint16_t addrs_port;
                bool this_url_enable_tls;     
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
                    Tp , struct client_parser_url
                    >::__M_ret_lmdba, void>
                 ::__is_M_ret_Tp
            >::type
            asyncParserUri (
                Tp&& callback_token,
                const std::string& addrs_url
            ) 
            noexcept {
                M_threadManager.getThreadManager()
                    .asyncTaskvoid([self = shared_from_this() , addrs_url ,
                         M_callback__ = std::function<void(struct client_parser_url)>(
                            std::move(callback_token))](){
                        struct client_parser_url M_urlBuffer = self -> syncParserUri(addrs_url);
                        self->M_threadManager.getThreadManager()
                            .asyncTaskvoid(std::move(M_callback__), M_urlBuffer);
                        return;
                    });
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
            ) noexcept {
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

            bool verifyTLS(uint16_t M_port);
            struct client_parser_url syncParserUri(const std::string& M_url__);
            std::string syncCreateHttpv1template (
                const std::string& addrs_host,
                const std::string& addrs_path,
                const std::string& addrs_body,
                httpMethod method,
                std::unordered_map<std::string, std::string> headers_map
            );
            private:
            struct client_parser_url M_parser_url;
            stagdeer::THREAD& M_threadManager = stagdeer::THREAD::getInstance();
        };
    using clientToolPtr = std::shared_ptr<stagdeer::client::clientTool>;
    using clientHttpv1TmpHeadersT = std::unordered_map<std::string, std::string>;
    }
};

#endif