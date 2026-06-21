## Stagdeer::Mtasc - ClientFremowrk / C++ 20

### 

<div style="display: flex; flex-direction: row; gap: 5px; justify-content: center; margin: 5px;">
    <span>
         Asynchronous client framework, users build client libraries relying on lightweight POSIX-Socket,
          OpenSSL, and C++ Thread. Since it’s aimed at 'client' 
          development, Epoll/Iocp are not used and supports IPv4/IPv6.
          <hr>
          <span>
           Welcome brothers to suggest RP, so the code becomes more stable !
          </span>
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
        <th>DATA/TIME</th>        
        <th>EVENT/RP</th>        
        <th>CONTRIBUTOR</th>
        <tr>
            <td>2026/6/22|12:26[CHINA]</td>
            <td>First time submitting to the repository</td>
            <td><a href="https://github.com/chromes-air">chromes-air</a></td>
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
        </div>
    </table>
</div>

##
## Next target

<span>
    Hide explicit initialization, use class instead of THREAD, support OpenSSL, organize header files , improve 'CMakeLists.txt'
</span>

##

###

### It's not recommended to use it right now, but you can check out the code and submit a PR to make it more stable .

## EXAMPLE CODE (Complicated)

```cpp

#include "stagdeer/Mtasc/client/socket/TCP/socket_tcp.hpp"
#include "stagdeer/Mtasc/include/client/clientapp/clientTool.hpp"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

// Global variable to store parsed URL result
// TODO: Replace with class member in production code
struct stagdeer::client::clientTool::client_parser_url M_parser_result_;

// Step 2: Build HTTP/1.1 template from parsed URL
void doMakeHttpv1tmp(stagdeer::client::clientToolPtr tool_ptr) {
    tool_ptr->asyncCreateHttpv1Tmp([](const std::string& httpTmp){
            // Callback: Print the generated HTTP template
            printf("TMP:\n%s\n" , httpTmp.c_str());
        }, M_parser_result_.addrs_host,      // Host from parsed URL
         M_parser_result_.addrs_path,        // Path from parsed URL
         "SB",                               // Body (placeholder)
         stagdeer::httpMethod::GET,          // HTTP method
         {{"Content-type" , "application/json"}}  // Headers
        );
}

// Step 1: Parse the URL string
void doParserUrl(stagdeer::client::clientToolPtr tool_ptr , const std::string& url) {
    tool_ptr->asyncParserUri([tool_ptr]
        (struct stagdeer::client::clientTool::client_parser_url parser_result){
            // Callback: Store parsed result and proceed to build HTTP template
            M_parser_result_ = std::move(parser_result);
            doMakeHttpv1tmp(tool_ptr);
        }, url);
}

// Step 4: Write data to the connected socket
void doWrite(struct stagdeer::client::socketTcp::client_addrs addrs ,
    std::shared_ptr<stagdeer::client::socketTcp> TcpPtr) {
        TcpPtr->async_write([](int err_code , std::string err_message, 
            size_t writed_bytes , 
            struct stagdeer::client::socketTcp::client_addrs&& addr){
                // Callback: Check write result
                if (err_code < 1) {
                    printf("WRITED FAILED: %s\n" , err_message.c_str());
                    return;
                }
                printf("SUCCESS WRITED %zu BYTES\n", writed_bytes);
            }, 
        std::move(addrs), 
        "Hello word!_____");  // Data to send (placeholder)
    return;
}

// Step 3: Establish TCP connection to the resolved address
void doConnect(struct stagdeer::client::socketTcp::client_addrs addrs , 
    std::shared_ptr<stagdeer::client::socketTcp> TcpPtr) {
        TcpPtr->async_try_connect_tcp([TcpPtr](int err_code , 
            std::string err_message , 
            struct stagdeer::client::socketTcp::client_addrs&& addr){
                // Callback: Check connection result
                if (err_code < 1) {
                    printf("CONNECT FAIELD: %s\n" , err_message.c_str());
                    return;
                }
                printf("CONNECT SUCCESS: %s\n" , err_message.c_str());
                // Proceed to write data after successful connection
                doWrite(std::move(addr), TcpPtr);
                return;
            }, std::move(addrs));
    return;
}

// Main entry point
int main () {
    // Initialize the global thread pool with 5 worker threads
    stagdeer::THREAD& threadInit = stagdeer::THREAD::getInstance();
    threadInit.createThreadManager(5);
    
    // Create the client tool (URL parser + HTTP template builder)
    stagdeer::client::clientToolPtr toolPtr = 
        stagdeer::client::clientTool::newClientTool();
    
    // Step 1: Parse the URL
    doParserUrl(toolPtr, "http://baidu.com/");
    
    // Create the TCP socket client
    std::shared_ptr<stagdeer::client::socketTcp> TCP =
        std::make_shared<stagdeer::client::socketTcp>("www.baidu.com" , 80 , "SB");
    struct stagdeer::client::socketTcp::client_addrs addrs = TCP->getMyaddr();
    
    // Step 2: Resolve domain name asynchronously
    TCP->async_resolver_domain([TCP]
        (int err_code , std::string error_msg , 
         stagdeer::client::socketTcp::client_addrs&& addr){
            if (err_code < 1) {
                printf("RESOLVER FAILED: %s\n" , error_msg.c_str());
                return;
            }
            printf("RESOLVER SUCCESS\n");
            
            // Step 3: Connect to the resolved address
            stagdeer::THREAD& newThread = stagdeer::THREAD::getInstance();
            newThread.getThreadManager()
                .asyncTaskvoid(std::move(doConnect), std::move(addr), TCP);
            printf("TRY CONNECT!\n");
            return;
        }, std::move(addrs));
    
    return 1;
}

```


###