//
//  main.cpp
//  HTTPServerCPP
//
//  Created by Sanjay Yepuri on 10/5/19.
//  Copyright © 2019 Sanjay Yepuri. All rights reserved.
//

#include <iostream>
#include <string>
#include <sstream>
#include <streambuf>
#include <boost/optional.hpp>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#include "HTTP.hpp"

#define PORT 8080

namespace fs = boost::filesystem;
boost::optional<std::string> readFile(std::string file_path) {
    fs::path p {file_path};
    fs::ifstream ifs {p};
    
    if (!ifs.is_open()) {
        return { };
    }

    std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    
    return data;
}

boost::optional<std::vector<std::string>> readHostFile(std::string file_path) {
    fs::path p {file_path};
    fs::ifstream ifs {p};
    
    if (!ifs.is_open()) {
        return {};
    }
    
    std::vector<std::string> hosts; 
    
    std::string host;
    while (std::getline(ifs, host)) {
        hosts.push_back(host);
    }
    
    return hosts;
}

std::unordered_map<std::string, std::string> create_host_dirs(std::string basedir, std::vector<std::string> dirs) {
    std::unordered_map<std::string, std::string> hostdirs;
    for (std::string dirname : dirs) {
        std::string p {basedir + dirname};
        
        hostdirs[dirname] = p;
        if (!boost::filesystem::create_directory(p)) {
            std::cout << "Directory " << p << " already exists.\n";
        } else {
            std::cout << "Created directory " << p << "\n";
        }
    }
    
    hostdirs["localhost"] = basedir;
    
    return hostdirs;
}

int main(int argc, const char * argv[]) {
    std::string basedir {"/Users/sanjayyepuri/Documents/Source/HTTPServerCpp/"};
    
    auto hosts = readHostFile(basedir +"hosts.txt");
    
    std::unordered_map<std::string, std::string> host_directories;
    if (hosts) {
        host_directories = create_host_dirs(basedir, *hosts);
    }
    
    http::Server srv { 8080 };
    
    srv.get("*", [&](http::Request& req, http::Response& res) {
        std::string path;
        if (boost::optional<std::string> p = req.get_header("Host")){
            path = host_directories[*p];
        } else {
            path = basedir;
        }
        
        std::cout << "PATH: " << path << "\n";
        
        if (auto data = readFile(path+req.get_path())) {
            res.send(200, *data);
        } else {
            res.send(404, "ERROR 404\n File not found.");
        }
    });

    srv.listen();
    return 0;
}
