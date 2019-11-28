 //
//  HTTP.cpp
//  HTTPServerCpp
//
//  Created by Sanjay Yepuri on 11/1/19.
//  Copyright © 2019 Sanjay Yepuri. All rights reserved.
//

#include "HTTP.hpp"


namespace http {

std::string getHTTPDate() {
    char buf[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    
    std::string time(buf);
    return time;
}

/* Server Implementation */

Server::Server(int port): _port(port) {
    int opt = 1; // TODO: look what is use for again
    
    if ((_srvfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(_srvfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    
    if (bind(_srvfd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void Server::errHandler(Request &req, Response &res) {
    res.send(403);
}

void Server::get(std::string path, Handler handler) {
    this->getHandler = std::pair<std::string, Handler> (path, handler);
}

void Server::listen() {
    if (::listen(_srvfd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    socklen_t addr_len = sizeof(_address);
    int inc_sock;
    for (;;) {
        inc_sock = accept(_srvfd, (struct sockaddr *) &_address, (socklen_t *) &addr_len);
        
        char buffer[BUFFER_SIZE]; // TODO: this buffer may lead to a memory leak.
//        read(inc_sock, buffer, BUFFER_SIZE);

        Request requestHeader(buffer, inc_sock);
        Response response(inc_sock); // hand off the socket to the response
        
        if (requestHeader.get_method() == GET) {
            // TODO do some error handling
            this->getHandler.second(requestHeader, response);
        } else if (requestHeader.get_method() == POST) {
            this->getHandler.second(requestHeader, response);
        } else {
            errHandler(requestHeader, response);
        }
    }
}

/* Request Implementation */

void Request::parse_buffer(char *buffer) {
    
    // SocketReader s_reader (_sock_fd );
    // std::stringstream hstream;
    // int dl;
    // while ( (dl = s_reader.getline(hstream)) > 0) {
    //     std::cout << "BYTES READ " << dl << "\n";
    // }
    std::stringstream hstream (buffer);
    
        
    std::string method_str;
    std::getline(hstream, method_str);
    
    std::stringstream mstream (method_str);
    mstream >> method_str >> this->path >> this->http_version; // TODO do some validation here
    
    if (method_str == "GET") {
        this->method = GET;
    } else if (method_str == "GET") {
        this->method = POST;
    } else {
        // TODO ERR handle
        std::cout << "Found unsupported http method: " << method_str << "\n";
    }
    
    std::cout << this->method << " " << this->path << " " << this->http_version << "\n";
    
    
    for (std::string header; std::getline(hstream, header); ) {
        std::vector<std::string> h_substrs;
        
        boost::split(h_substrs, header, boost::is_any_of(":")); // TODO do a more robust parse of headers        
        
        if (h_substrs.size() < 2) break;
        boost::trim(h_substrs[0]);
        boost::trim(h_substrs[1]);
        
        std::cout << "key: " << h_substrs[0] << " value: " << h_substrs[1] << "\n";
        
        this->headers[h_substrs[0]] = h_substrs[1];
    }
}

Request::Request(char *buffer, int sock_fd): _sock_fd(sock_fd) {
    parse_buffer(buffer);
}

boost::optional<std::string> Request::get_header(std::string header) {
    if (headers.count(header) > 0) {
        return this->headers[header];
    }
    return { };
}

HTTPMethod Request::get_method() {
    return this->method;
}

std::string Request::get_path() {
    return this->path;
}

/* Response Implementation */

std::unordered_map<int, std::string> Response::ResponseCodes =
   {
       {200, "OK"},
       {400, "Bad Request"},
       {404, "Not Found"},
       {500, "Internal Server Error"}
   };

Response::Response(int sock_fd) : _sock_fd(sock_fd) { }

void Response::send(int status) {
    std::stringstream res;
    
    std::string header_end = "\r\n";
    res << "HTTP/" << _version << " " << status << " " << ResponseCodes[status] << header_end
        << "Date: " << getHTTPDate() << header_end
        << header_end;
    
    ::send(_sock_fd, res.str().c_str(), res.str().length(), 0);
    
    close(_sock_fd);
}

void Response::send(int status, std::string data) {
    std::stringstream res;
       
    std::string header_end = "\r\n";
    res << "HTTP/" << _version << " " << status << " " << ResponseCodes[status] << header_end
        << "Date: " << getHTTPDate() << header_end
        << "Content-Length: " << data.length() << header_end
        << header_end
        << data;
       
    ::send(_sock_fd, res.str().c_str(), res.str().length(), 0);
    
    close(_sock_fd);
}


/* SocketReader Implementation */

SocketReader::SocketReader(int sock_fd): _sock_fd(sock_fd), cursor(0), len(0) { }


int SocketReader::fill_buffer() {
    std::cout << "fill buffer\n";
    cursor = 0;
    len = (int) read(_sock_fd, this->buffer, BUFFER_SIZE);;
    std::cout << "filled buffer: " << len << std::endl;
    return len;
}

int SocketReader::getline(std::stringstream &strm) {
    bool done = false;
    
    int data_read = 0;
    
    std::cout << "read socket\n";
    while (!done) {
        for ( ;cursor < len; cursor++) {
            char c = buffer[cursor];
            
            std::cout << "read: " << c << "\n";
            
            strm << c;
            data_read++;
            
            
            if (c == '\n') {
                cursor++;
                return data_read;
            }
        }
        
        int dlen = fill_buffer();
        if (dlen == 0) {
            return data_read;
        }
    }
    
    return data_read;
    
}



}
