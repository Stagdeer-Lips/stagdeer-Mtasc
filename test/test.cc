#include "../include/client/socket/TCP/socket_tcp.hpp"
#include "../include/client/clientapp/clientTool.hpp"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

stagdeer::client::clientToolPtr tool_ptr = stagdeer::client::clientTool::newClientTool();

void doRead(struct stagdeer::client::socketTcp::client_addrs addrs , 
    stagdeer::client::socketTcpPtrT TcpPtr) {
        TcpPtr->async_read_until([TcpPtr](int err_code , std::string err_message , size_t accepet_bytes ,
             std::shared_ptr<stagdeer::client::readBuffer>&& result_buffer_ , 
                struct stagdeer::client::socketTcp::client_addrs&& addrs)
            {
                if (err_code < 1) {
                    printf("READ FAILED! %s\n" , err_message.c_str());
                    return;
                }
                std::string data(result_buffer_->peekData());
                /**
                    Here you can parse HTTP/1.1, parse 'Content-Length',
                     and then tell 'async_read' how many bytes your body needs.
                */
                //READ FULL
                TcpPtr->async_read([](int err_code_ , std::string err_message_ , size_t accepet_bytes_ , 
                    std::shared_ptr<stagdeer::client::readBuffer>&& buffer ,
                        struct stagdeer::client::socketTcp::client_addrs&& _addrs){
                        if (err_code_ < 0) {
                            printf("READ FAILED: %s\n" , err_message_.c_str());
                            return;
                        }
                        /**
                        Here, you can parse a full HTTP response wrapped in your 'Response' 
                        class to become an HTTP client library.
                        */
                        printf("DATA:\n%s\n" , buffer->peekData());
                    return;
                }, std::move(result_buffer_) , std::move(addrs), 429);
             }, std::move(addrs), "\r\n\r\n");
    return;
}

void doWrite(struct stagdeer::client::socketTcp::client_addrs addrs ,
    stagdeer::client::socketTcpPtrT TcpPtr) {
        /**
          From now on, block template generation and URL parsing  
        */
        struct stagdeer::client::clientTool::client_parser_basic_url url_result = tool_ptr->syncParserBasicUri
        ("http://httpbin.org/json");
            std::string httpv1tmp = tool_ptr->syncCreateHttpv1template(url_result.addrs_host,
                url_result.addrs_path, "NULL", stagdeer::httpMethod::GET,
                {{"Content-type" , "application/json"}}) ;
        std::cout << httpv1tmp << std::endl; //Debug print template here
        /**
        RESULT:
            GET /json HTTP/1.1
            Host: httpbin.org
            Content-type: application/json
            Connection: close
        */
        TcpPtr->async_write([TcpPtr](int err_code , std::string err_message, size_t writed_bytes , 
            struct stagdeer::client::socketTcp::client_addrs&& addr) mutable{
                if (err_code < 1) {
                    printf("WRITED FAILED: %s\n" , err_message.c_str());
                    return;
                }
                
                printf("SUCCESS WRITE %zu BYTES\n" , writed_bytes);
                doRead(std::move(addr), TcpPtr);
                return;
            }, 
        std::move(addrs), httpv1tmp);
    return;
}

void doConnect(struct stagdeer::client::socketTcp::client_addrs addrs , 
    stagdeer::client::socketTcpPtrT TcpPtr) {
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
    stagdeer::client::socketTcpPtrT TCP = std::make_shared<stagdeer::client::socketTcp>("httpbin.org" , 80 , "NULL");
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
}
