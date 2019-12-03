//
//  HTTP.hpp
//  HTTPServerCpp
//
//  Created by Sanjay Yepuri on 11/1/19.
//  Copyright © 2019 Sanjay Yepuri. All rights reserved.
//

#ifndef HTTP_hpp
#define HTTP_hpp

#include <stdio.h>
#include <functional>
#include <regex>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <streambuf>
#include <unordered_map>
#include <memory>


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>


#define BUFFER_SIZE 1024


namespace http {
std::string readFile(std::string file_path);
std::string getHTTPDate();


enum HTTPMethod {
    GET, POST, PUT, _unknown
};

class Request {
    HTTPMethod method;
    std::string path;
    std::string http_version;

    std::string data;
    
    int _sock_fd;

    std::unordered_map<std::string, std::string> headers; // at the moment we are just storing the headers as strings
    
    void parse_buffer(char *buffer);
    
public:
    Request(std::shared_ptr<char> buffer, int sock_fd);
    std::string get_path();
    std::string get_data();
    boost::optional<std::string> get_header(std::string header);
    HTTPMethod get_method();
};


class Response {
    const std::string _version = "1.1";
    int _sock_fd;
    
    static std::unordered_map<int, std::string> ResponseCodes;

public:
    Response(int sock_fd);
    
    void send(int status);
    void send(int status, std::string data);
};


using Handler = std::function<void(Request&, Response&)>;
        
class Server {
    
private:
    int _port;
    int _srvfd;
    struct sockaddr_in _address;
    
//    std::vector<std::pair<std::regex, Handler>> getHandlers;
//    std::vector<std::pair<std::regex, Handler>> postHandlers;
    
    // TODO for the time begin we will only have one post and get handler
    std::pair<std::string, Handler> getHandler;
    std::pair<std::string, Handler> postHandler;
    
    void errHandler(Request& req, Response& res);
    
public:
    Server(int port);
    
    // register post and get handlers
    void post(std::string path, Handler handler);
    void get (std::string path, Handler handler);
    
    void listen();
};

class SocketReader {
    int _sock_fd;
    
    int cursor;
    int len;
    
    char buffer[BUFFER_SIZE];
    
    int fill_buffer();
    
public:
    SocketReader(int sock_fd);
    
    int getline(std::stringstream &strm);
};


}

#endif /* HTTP_hpp */
