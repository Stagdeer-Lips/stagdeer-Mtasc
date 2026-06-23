#include "../../include/client/Parser/Urlparser.hpp"
#include <string>
#include <utility>

struct stagdeer::client::urlHttpParser::basicUrlParserResult
     stagdeer::client::urlHttpParser::parserBasicUrl() const {
        struct stagdeer::client::urlHttpParser::basicUrlParserResult M_basic_parser_result;
        basicParserState M_basic_parser_state = basicParserState::M_PARSER_SIGN;
        const char* M_parser_char_pos = M_parser_char.data();
        const char* M_parser_char_end = M_parser_char.data() + M_parser_char.size();
        const char* M_parser_char_start = M_parser_char_pos;
        while (M_parser_char_pos != M_parser_char_end) {
            char M_char = *M_parser_char_pos;
            switch (M_basic_parser_state) {
                case basicParserState::M_PARSER_SIGN: {
                    //FIND URL HTTP SIGN
                    if (M_char == ':' && *(M_parser_char_pos + 1) == '/') {
                        //EXTRA SIGN
                        M_basic_parser_result.http_sign = std::string(
                            M_parser_char_start , 
                            M_parser_char_pos - M_parser_char_start);
                        //CHANGE STATE
                        M_basic_parser_result.is_enable_tls = (M_basic_parser_result.http_sign == "https");
                        M_parser_char_pos += 3;
                        M_parser_char_start = M_parser_char_pos;
                        M_basic_parser_state = basicParserState::M_PARSER_HOST;
                        continue;
                    }
                    break;
                };
                
                case basicParserState::M_PARSER_HOST: {
                    //FIND URL HTTP HOST
                    if (M_char == ':' || M_char == '/') {
                        //EXTAR HOST
                        M_basic_parser_result.addrs_host = std::string(M_parser_char_start ,  
                            M_parser_char_pos - M_parser_char_start); 
                        if (M_char == ':') {
                            //USE PORT
                            M_basic_parser_state = basicParserState::M_PARSER_PORT;
                            M_parser_char_start = M_parser_char_pos + 1;
                            break;
                        } else {
                            //NO PORT
                            M_basic_parser_state = basicParserState::M_PARSER_PATH;
                            M_parser_char_start = M_parser_char_pos;
                            continue;
                        }
                    }
                    break;
                };

                case basicParserState::M_PARSER_PORT: {
                    if (M_char == '/') {
                        //EXTRA PORT
                        std::string M_port_str = std::string(M_parser_char_start , 
                            M_parser_char_pos - M_parser_char_start);
                        M_basic_parser_result.addrs_port = std::stoi(M_port_str);
                        M_basic_parser_state = basicParserState::M_PARSER_PATH;
                        M_parser_char_start = M_parser_char_pos;
                        continue;
                    }
                    break;
                };

                case basicParserState::M_PARSER_PATH: {
                    std::string M_path = std::string(M_parser_char_start ,
                            M_parser_char_end - M_parser_char_start);
                        M_basic_parser_result.addrs_path = std::move(M_path.empty() ? "/" : M_path);
                    M_basic_parser_state = basicParserState::M_PARSER_END;
                    continue;
                };

                case basicParserState::M_PARSER_END: {
                    return M_basic_parser_result;
                }
            };

            ++M_parser_char_pos;
        }
    return M_basic_parser_result;
}

struct stagdeer::client::urlHttpParser::queryUrlParserResult 
    stagdeer::client::urlHttpParser::parserQueryUrl() const {
        struct stagdeer::client::urlHttpParser::queryUrlParserResult M_query_parser_result;
        queryParserState M_query_parser_state = queryParserState::M_PARSER_SIGN;
        const char* M_parser_char_ops = M_parser_char.data();
        const char* M_parser_char_end = M_parser_char.data() + M_parser_char.size();
        const char* M_parser_char_start = M_parser_char_ops;

        while (M_parser_char_ops != M_parser_char_end) {
            char M_char = *M_parser_char_ops;
            switch (M_query_parser_state) {
                //FIND SIGN
                case queryParserState::M_PARSER_SIGN: {
                    if (M_char == ':' && *(M_parser_char_ops + 1) == '/') {
                        //EXTREA SIGN
                        M_query_parser_result.http_sign = std::move(
                            std::string(M_parser_char_start , M_parser_char_ops - M_parser_char_start)
                        );
                        //CHANGE STATUS
                        M_query_parser_state = queryParserState::M_PARSER_HOST;
                        M_query_parser_result.is_enable_tls = (M_query_parser_result.http_sign == "https");
                        M_parser_char_ops += 3;
                        M_parser_char_start = M_parser_char_ops;
                        continue;
                    }
                    break;
                };

                case queryParserState::M_PARSER_HOST: {
                    //FIND HOST
                    if (M_char == ':' || M_char == '/') {
                        //EXTRA HOST
                        M_query_parser_result.addrs_host = std::move(
                            std::string(M_parser_char_start , M_parser_char_ops - M_parser_char_start)
                        );
                        if (M_char == ':') {
                            //USE PORT
                            M_query_parser_state = queryParserState::M_PARSER_PORT;
                            M_parser_char_start = M_parser_char_ops + 1;
                            break;
                        } else {
                            //USE QUERY PATH
                            M_query_parser_state = queryParserState::M_PARSER_QUERY;
                            M_parser_char_start = M_parser_char_ops;
                            break;
                        }
                    }
                    break;
                }

                case queryParserState::M_PARSER_PORT: {
                    if (M_char == '/') {
                        //EXTRA PORT
                        std::string M_query_port_str = std::move(
                            std::string(M_parser_char_start , M_parser_char_ops - M_parser_char_start)
                        );
                        M_query_parser_result.addrs_port = std::stoi(std::move(M_query_port_str));
                        M_query_parser_state = queryParserState::M_PARSER_QUERY;
                        M_parser_char_start = M_parser_char_ops;
                        continue;
                    }
                    break;
                }

                case queryParserState::M_PARSER_QUERY: {
                    if (M_char == '?' || M_char == '#') {
                        const char* M_query_start = M_parser_char_ops;
                        const char* M_query_end = M_query_start;
                        while (M_query_end != M_parser_char_end && *M_query_end != '#') {
                            M_query_end ++;
                        }
                        //EXTRA QUERY
                        std::string M_query_str(M_query_start , M_query_end - M_query_start);
                        
                        M_query_parser_result.basic_path = std::move(
                            std::string(M_parser_char_start , 
                                M_parser_char_ops - M_parser_char_start
                            )
                        );

                        M_query_parser_result.query_path = std::move(
                            std::string(M_query_str)
                        );
                        M_query_parser_state = queryParserState::M_PARSER_END;
                        continue;
                    }
                    break;
                }

                case queryParserState::M_PARSER_END: {
                    M_query_parser_result.query_paramter_map = 
                        M_parserQueryParamter(M_query_parser_result.query_path);
                    return M_query_parser_result;
                }
            }
            ++ M_parser_char_ops;
        }
        return M_query_parser_result;
    }