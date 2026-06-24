## Stagdeer::Mtasc - ClientFremwork / C++ 20

### 

<div style="display: flex; flex-direction: row; gap: 5px; justify-content: center; margin: 5px;">
    <span>
         Asynchronous client framework, users build client libraries relying on lightweight POSIX-Socket,
          OpenSSL, and C++ Thread. Since it’s aimed at 'client' 
          development, Epoll/Iocp are not used and supports IPv4/IPv6.
           Welcome brothers to suggest RP, so the code becomes more stable !
          <hr>
    </span>
    <img style="width: 200px; border-radius:20px;" src="github_assets/stagdeer-matsc-logo.png"></img>
</div>

###

###
## RP/EVENT

###

###
<div style="display:flex; flex-direction: column;">

<div>
    <table>
        <th>|DATA/TIME|</th>        
        <th>|EVENT/RP|</th>        
        <th>|CONTRIBUTOR|</th>
        <tr>
            <td>2026/6/22|12:26[CHINA]</td>
            <td>First time submitting to the repository</td>
            <td><a href="https://github.com/chromes-air">chromes-air</a>  
        </tr>
        <tr> <td>2026/6/24|1:04[CHINA]</td>
            <td>Changed ParserUrl parsing to use a state machine instead of brute-force parsing, fixed pointer errors in IPv6 parsing encapsulation, and renamed the CMake project to 'stagdeer-matsc' with support for 'async_read_until/async_read'.</td>
            <td><a href="https://github.com/chromes-air">chromes-air</a>
            </td>
        </tr>
    </table>
</div>

<hr>

###
## UPDATE

<div>
    <table>
        <th>UPDATE/TIME</th>
        <th>UPDATE/ERROR/FUNCTION</th>
        <th>UPDATE/FILE/LINE</th>
        <th>CONTRIBUTORS</th>
        <div>
            <tr>
                <td>2026/6/22|12:26[CHINA]</td>
                <td>Support for LINUX TCP connection/write/template generation/simple HTTPS/HTTP URL parsing will be supported tomorrow, and SSL handling will be organized after reading.</td>
                <td>TCP/socket_tcp.hpp</td>
                <td><a href="https://github.com/chromes-air">chromes-air</a></td>
            </tr>
            <tr>
                <td>2026/6/24|1:04[CHINA]</td>
                <td>Rewrote URLPARSER to use a state machine for parsing, supporting both query and normal URLs. Updated 'async_read_until/async_read' to implement basic TCP operations. Planning to handle 'Chunked' later, focusing on the 'Exml' project this week to support XML parsing, which will be helpful later. After next week, the main focus will be on implementing SSL connection reading.</td>
                <td>TCP/socket_tcp.hpp/Ipv4Addrs/Ipv6Addrs/Urlparser</td>
                <td><a href="https://github.com/chromes-air">chromes-air</a></td>
            </tr>
        </div>
    </table>
</div>

##
## Next target

<span>
Started writing the 'Exml' project, after a week of development continued to implement SSL support, then finally started testing, and after that gradually added 'Download' and 'WebSocket' support to make Mtasc usable for development.
</span>

##

###

### It's not recommended to use it right now, but you can check out the code and submit a PR to make it more stable .

## EXAMPLE CODE (Complicated)

```cpp

#include "stagdeer/Mtasc/client/socket/TCP/socket_tcp.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>
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


```
###
