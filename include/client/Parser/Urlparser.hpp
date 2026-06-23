#ifndef STAGDEER_CLIENT_URLPARSER
#define STAGDEER_CLIENT_URLPARSER

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace stagdeer {
    namespace client {
        class urlHttpParser {
            public:

            struct basicUrlParserResult {
                std::string addrs_host;
                std::string addrs_path;
                std::string http_sign;
                uint16_t addrs_port;
                bool is_enable_tls = false;
            };

            struct queryUrlParserResult {
                std::string addrs_host;
                std::string basic_path;
                std::string http_sign;
                std::string query_path;
                uint16_t addrs_port;
                bool is_enable_tls = false;
                std::map<std::string, std::string> query_paramter_map;
            };

            urlHttpParser() = default;

            urlHttpParser(const urlHttpParser&) = default;
            urlHttpParser(urlHttpParser&& M_other_Parser) 
            noexcept:
                M_parser_char(std::move(M_other_Parser.M_parser_char)) {};
                
                urlHttpParser& operator=(urlHttpParser&& M_other_Parser_Operator)
                    noexcept {
                        if (this != &M_other_Parser_Operator) {
                            M_parser_char = std::move(M_other_Parser_Operator.M_parser_char);
                        }
                        return *this;
                    }

            urlHttpParser(const std::string parser_url) {
                M_parser_char = parser_url;
            }


            //BASIC HTTP URL PARSER STATE
            //EXAMPLE:
            /**
                https://www.test.org/path/
            */

            struct basicUrlParserResult parserBasicUrl() const;

            //QUERY HTTP URL PARSER STATE
            //EXAMPLE:
            /**
                https://www.test.org/user/name?n='Jack'/
            */
            
            struct queryUrlParserResult parserQueryUrl() const;
            
            // TODO: std::string parserIpv6Url();

            private:

            std::map<std::string, std::string> M_parserQueryParamter(const std::string& M_fullUrl_path) 
                const noexcept {
                    /**
                        https://xxx.com/api/user?name='jack'&age=20
                    */
                    std::map<std::string, std::string> M_paramterMap;

                    if (M_fullUrl_path.empty()) return M_paramterMap;

                    size_t M_parser_ops = 0;
                    while (M_parser_ops < M_fullUrl_path.size()) {
                        if (M_fullUrl_path[M_parser_ops] == '?') {
                            M_parser_ops ++;
                        }
                        size_t M_eq_ops = M_fullUrl_path.find("=" , M_parser_ops);
                        size_t M_amp_ops = M_fullUrl_path.find("&" , M_parser_ops);
                        if (M_amp_ops == std::string::npos) {
                            M_amp_ops = M_fullUrl_path.size();
                        }
                        std::string M_query_key = M_fullUrl_path.substr(M_parser_ops , M_eq_ops - M_parser_ops);

                        if (M_eq_ops != std::string::npos && M_eq_ops < M_amp_ops) {
                            std::string M_query_value = M_fullUrl_path.substr(M_eq_ops + 1 , M_amp_ops - M_eq_ops - 1);
                            M_paramterMap[std::move(M_query_key)] = std::move(M_query_value);
                        } else {
                            M_paramterMap[M_query_key] = std::string("[EMPYTY-PARAMTER]");
                        }
                        M_parser_ops = M_amp_ops + 1;
                    }
                    return M_paramterMap;
                }

            enum class basicParserState {
                M_PARSER_SIGN,
                M_PARSER_HOST,
                M_PARSER_PORT,
                M_PARSER_PATH,
                M_PARSER_END
            };

            enum class queryParserState {
                M_PARSER_SIGN,
                M_PARSER_HOST,
                M_PARSER_PORT,
                M_PARSER_QUERY,
                M_PARSER_END
            };

            std::string M_parser_char;
        };
    }
}

#endif