#include "../include/mtasc.hpp"
#include <cstdio>
#include <string>

int main () {
    stagdeer::THREAD& thread_ = stagdeer::THREAD::getInstance();
        thread_.createThreadManager(5);
    std::string ParserUrl = "https://test.com/v1/v2?a=33&b=55&al=2#session";
    stagdeer::client::clientToolPtr client_tool = 
        stagdeer::client::clientTool::newClientTool();
        client_tool->asyncParserQueryUri([](struct 
            stagdeer::client::clientTool::client_parser_query_url parser_result){
                printf("HOST: %s\n" , parser_result.addrs_host.c_str());
                printf("BASIC PATH: %s\n" , parser_result.addrs_basic_path.c_str());
                printf("QUERY PATH: %s\n" , parser_result.addrs_query_path.c_str());
                for (stagdeer::client::urlQueryMapT::const_iterator query_map_it = 
                    parser_result.query_paramter_map.begin(); query_map_it 
                        != parser_result.query_paramter_map.end(); ++query_map_it) {
                    printf("QUERY_KEY: %s | QUERY_VALUE: %s\n" , 
                        query_map_it->first.c_str() , query_map_it->second.c_str());
                    return;
                }
        }, ParserUrl);
    return 0;
}