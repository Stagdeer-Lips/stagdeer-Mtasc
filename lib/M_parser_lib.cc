#include "../include/client/clientapp/clientTool.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

bool stagdeer::client::clientTool::verifyTLS(uint16_t M_port) {
    if (M_port == 80) {
        return false;
    }
    return true;
}

struct stagdeer::client::clientTool::client_parser_url stagdeer::client::clientTool::syncParserUri(const std::string& M_url__) {
    struct stagdeer::client::clientTool::client_parser_url M_url_buffer;
    std::string M_url = M_url__;
    size_t M_find_sign = M_url.find("http");
    if (M_find_sign == std::string::npos) {
        throw std::runtime_error("Invalid URL");
    }
    size_t M_KEEP_SIGN_SIZE = 0;
    size_t find_TLS_sign = M_url.find("https");
        if (find_TLS_sign != std::string::npos) {
        //USE TLS
            M_KEEP_SIGN_SIZE = 5;
        } else {
        //NO TLS
            M_KEEP_SIGN_SIZE = 4;
            M_url_buffer.this_url_enable_tls = false;
            M_url_buffer.addrs_port = 80;
        }
    size_t M_find_host_start = M_find_sign + M_KEEP_SIGN_SIZE + 3;
    if (M_find_host_start == std::string::npos) {
        throw std::runtime_error("Invlid URL");
    }
    //EXTERA HOST
    size_t M_host_end = 0;
    size_t M_find_port = M_url.find(":" , M_find_host_start);
    if (M_find_port != std::string::npos) {
        size_t M_find_port_start = M_find_port;
        M_host_end = M_url.find("/" , M_find_port_start);
        size_t M_port_len = M_find_port_start - M_host_end;
        M_host_end = M_host_end + M_port_len;
    } else {
        M_host_end = M_url.find("/" , M_find_host_start);
    }
    size_t M_find_path_start = M_url.find("/" , M_host_end);
    if (M_find_path_start == std::string::npos) {
        M_url += "/";
        throw std::runtime_error("Invalid URL");
    }
    std::string M_HOST = M_url.substr(M_find_host_start , M_host_end - M_find_host_start);
    M_url_buffer.addrs_host = std::move(M_HOST);
    //FIND PORT
    size_t M_find_port_first = M_url.find(":" , M_find_host_start);
    if (M_find_port_first == std::string::npos) {
        //NO PORT | EXTERA PATH
        M_url_buffer.addrs_path = std::move(M_url.substr(M_find_path_start));
        return M_url_buffer;
    } else {
        //USE PORT 
        // M_find_port_first = PORT_START 
        // https://xxx.xxx:8080/path
        size_t M_PORT_END = M_url.find("/" , M_find_port_first);
        std::string M_PORT_STR = M_url.substr((M_find_port_first + 1) , (M_PORT_END - M_find_port) - 1);
        try {  
            //EXTRA PATH
            M_url_buffer.addrs_port = std::stoi(M_PORT_STR);
            M_url_buffer.this_url_enable_tls = verifyTLS(M_url_buffer.addrs_port);
            std::string M_PATH = M_url.substr(M_find_path_start);
            M_url_buffer.addrs_path = std::move(M_PATH);
        } catch (std::runtime_error& M_err) {
            throw std::runtime_error("Invlid URL: " + 
                std::string(M_err.what()
            ));
        }
        return M_url_buffer;
    }   
}