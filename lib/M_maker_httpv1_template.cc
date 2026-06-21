#include "../include/client/clientapp/clientTool.hpp"
#include <string>
#include <unordered_map>
#include <utility>

std::string stagdeer::client::clientTool::syncCreateHttpv1template(
    const std::string& addrs_host,
    const std::string& addrs_path,
    const std::string& addrs_body,
    httpMethod method,
    std::unordered_map<std::string, std::string> headers_map
) {
    //MAKER HTTP/1.1 TEMPLATES
    const std::string method_str = stagdeer::methodTool::methodToStr(method);
    std::string M_httpv1_template;
    M_httpv1_template += method_str + " " + addrs_path + " " + "HTTP/1.1\r\n";
    M_httpv1_template += "Host: " + addrs_host + "\r\n";
    
    for (std::unordered_map<std::string, std::string>
        ::const_iterator __M_iterator = headers_map.begin();
        __M_iterator != headers_map.end(); ++ __M_iterator) {
            if (!__M_iterator->first.empty() && !__M_iterator->second.empty()) {
                M_httpv1_template += __M_iterator->first + ": " + __M_iterator->second + "\r\n";
            } else {
                //NO HEADERS
                M_httpv1_template += "Connection: close\r\n";
                M_httpv1_template += "\r\n\r\n";
                return M_httpv1_template;
            }
        }
    
    if (method == httpMethod::GET) {
        M_httpv1_template += "Connection: close\r\n";
        M_httpv1_template += "\r\n\r\n";
        return M_httpv1_template;
    }

    M_httpv1_template += "Content-Length: " + 
        std::to_string(addrs_body.length()) + "\r\n";
    M_httpv1_template += "Connection: close\r\n";
    M_httpv1_template += "\r\n\r\n";
    M_httpv1_template += std::move(addrs_body);
    return M_httpv1_template;
}