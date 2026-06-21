#include "../include/client/socket/TCP/socket_tcp.hpp"
#include "../include/client/clientapp/clientTool.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
 
struct stagdeer::client::clientTool::client_parser_url M_parser_result_;
void doMakeHttpv1tmp(stagdeer::client::clientToolPtr tool_ptr) {
    tool_ptr->asyncCreateHttpv1Tmp([](const std::string& httpTmp){
            printf("TMP:\n%s\n" , httpTmp.c_str());
        }, M_parser_result_.addrs_host,
         M_parser_result_.addrs_path, 
         "SB", stagdeer::httpMethod::GET,
          {{"Content-type" , "application/json"}}
        );
}

void doParserUrl(stagdeer::client::clientToolPtr tool_ptr , const std::string& url) {
    tool_ptr->asyncParserUri([tool_ptr]
        (struct stagdeer::client::clientTool::client_parser_url parser_result){
            M_parser_result_ = std::move(parser_result);
            doMakeHttpv1tmp(tool_ptr);
        },url);
}
void doWrite(struct stagdeer::client::socketTcp::client_addrs addrs ,
    std::shared_ptr<stagdeer::client::socketTcp> TcpPtr) {
        TcpPtr->async_write([](int err_code , std::string err_message, size_t writed_bytes , 
            struct stagdeer::client::socketTcp::client_addrs&& addr){
                if (err_code < 1) {
                    printf("WRITED FAILED: %s\n" , err_message.c_str());
                    return;
                }
             
            }, 
        std::move(addrs), "Hello word!_____");
    return;
}


void doConnect(struct stagdeer::client::socketTcp::client_addrs addrs , 
    std::shared_ptr<stagdeer::client::socketTcp> TcpPtr) {
        TcpPtr->async_try_connect_tcp([TcpPtr](int err_code , std::string err_message , 
            struct stagdeer::client::socketTcp::client_addrs&& addr){
                if (err_code < 1) {
                    printf("CONNECT FAIELD: %s\n" , err_message.c_str());
                    return;
                }
                printf("CONNECT SUCCESS: %s\n" , err_message.c_str());
                doWrite(std::move(addr), TcpPtr);
                return;
            }, std::move(addrs));
    return;
}

int main () {
    stagdeer::THREAD& threadInit = stagdeer::THREAD::getInstance();
    threadInit.createThreadManager(5);
   stagdeer::client::clientToolPtr toolPtr = stagdeer::client::clientTool::newClientTool();
                doParserUrl(toolPtr, "http://baidu.com/");
    std::shared_ptr<stagdeer::client::socketTcp> TCP =
    std::make_shared<stagdeer::client::socketTcp>("www.baidu.com" , 80 , "SB");
    struct stagdeer::client::socketTcp::client_addrs addrs = TCP->getMyaddr();
    TCP->async_resolver_domain([TCP]
        (int err_code , std::string error_msg , stagdeer::client::socketTcp::client_addrs&& addr){
            if (err_code < 1) {
                printf("RESOLVER FAILED: %s\n" , error_msg.c_str());
                return;
            }
        printf("RESOLVER SUCCESS\n");
        stagdeer::THREAD& newThread = stagdeer::THREAD::getInstance();
            newThread.getThreadManager()
            .asyncTaskvoid(std::move(doConnect), std::move(addr) ,TCP);
        printf("TRY CONNECT!\n");
        return;
    }, std::move(addrs));
    return 1;
}
