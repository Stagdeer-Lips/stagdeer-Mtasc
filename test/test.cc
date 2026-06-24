#include "../include/client/socket/TCP/socket_tcp.hpp"
#include "../include/client/clientapp/clientTool.hpp"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <system_error>
#include <utility>

stagdeer::client::clientToolPtr tool_ptr = stagdeer::client::clientTool::newClientTool();

void doRead(struct stagdeer::client::socketTcp::client_context client_ctx , 
    stagdeer::client::socketTcpPtrT TcpPtr) {
        TcpPtr->async_read_until([TcpPtr](const std::error_code& ec , size_t accepet_bytes ,
             std::shared_ptr<stagdeer::client::readBuffer>&& result_buffer_ , 
                struct stagdeer::client::socketTcp::client_context&& ctx)
            {
                if (ec) {
                    printf("READ FAILED! %s\n" , ec.message().c_str());
                    return;
                }
                /**
                    Here you can parse HTTP/1.1, parse 'Content-Length',
                     and then tell 'async_read' how many bytes your body needs.
                */
                //READ FULL
                TcpPtr->async_read([](const std::error_code& ec, size_t accepet_bytes_ , 
                    std::shared_ptr<stagdeer::client::readBuffer>&& buffer ,
                        struct stagdeer::client::socketTcp::client_context&& _ctx){
                        if (ec) {
                            printf("READ FAILED: %s\n" , ec.message().c_str());
                            return;
                        }
                        /**
                        Here, you can parse a full HTTP response wrapped in your 'Response' 
                        class to become an HTTP client library.
                        */
                        printf("DATA:\n%s\n" , buffer->peekData());
                    return;
                }, std::move(result_buffer_) , std::move(ctx), 29506);
             }, std::move(client_ctx), "\r\n\r\n");
    return;
}

void doWrite(struct stagdeer::client::socketTcp::client_context client_ctx ,
    stagdeer::client::socketTcpPtrT TcpPtr , const std::string& url) {
        /**
          From now on, block template generation and URL parsing  
        */
        struct stagdeer::client::clientTool::client_parser_basic_url url_result = tool_ptr->syncParserBasicUri
        (url);
            std::string httpv1tmp = tool_ptr->syncCreateHttpv1template(url_result.addrs_host,
                url_result.addrs_path, "NULL", stagdeer::httpMethod::GET,
                {{"Content-type" , "text/html"}}) ;
        std::cout << httpv1tmp << std::endl; //Debug print template here
        /**
        RESULT:
            GET /json HTTP/1.1
            Host: httpbin.org
            Content-type: application/json
            Connection: close
        */
        TcpPtr->async_write([TcpPtr](const std::error_code& ec, size_t writed_bytes , 
            struct stagdeer::client::socketTcp::client_context&& ctx) mutable{
                if (ec) {
                    printf("WRITED FAILED: %s\n" , ec.message().c_str());
                    return;
                }
                
                printf("SUCCESS WRITE %zu BYTES\n" , writed_bytes);
                doRead(std::move(ctx), TcpPtr);
                return;
            }, 
        std::move(client_ctx), httpv1tmp);
    return;
}

void doConnect(struct stagdeer::client::socketTcp::client_context client_ctx , 
    stagdeer::client::socketTcpPtrT TcpPtr , const std::string& url) {
        TcpPtr->async_try_connect_tcp([TcpPtr , url](const std::error_code& ec , 
            struct stagdeer::client::socketTcp::client_context&& ctx){
                if (ec) {
                    printf("CONNECT FAIELD: %s\n" , ec.message().c_str());
                    return;
                }
                printf("CONNECT SUCCESS\n");
                doWrite(std::move(ctx), TcpPtr , url);
                return;
            }, std::move(client_ctx));
    return;
}

int main (int arg , char* argv[]) {
    if (arg < 3) {
        printf("ERROR: Paramter invalid!");
        return -1;
    }
    while (true) {
        std::string url = argv[1];
        std::string host  = argv[2];
        stagdeer::THREAD& threadInit = stagdeer::THREAD::getInstance();
        threadInit.createThreadManager(5);
        stagdeer::client::socketTcpPtrT TCP = std::make_shared<stagdeer::client::socketTcp>(host.c_str() , 80 , "NULL");
        struct stagdeer::client::socketTcp::client_context client_ctx = TCP->getClientContext();
        TCP->async_resolver_domain([TCP , url]
            (const std::error_code& ec , stagdeer::client::socketTcp::client_context&& ctx){
                if (ec) {
                    printf("RESOLVER FAILED: %s\n" , ec.message().c_str());
                    return;
                }
            printf("RESOLVER SUCCESS\n");
            stagdeer::THREAD& newThread = stagdeer::THREAD::getInstance();
                newThread.getThreadManager()
                .asyncTaskvoid(std::move(doConnect), std::move(ctx) ,TCP , url);
            printf("TRY CONNECT!\n");
            return;
        }, std::move(client_ctx));
    }
}
