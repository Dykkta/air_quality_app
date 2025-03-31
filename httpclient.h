//httpclient.h
#pragma once
#include <string>

class HttpClient {
public:
    static std::string getRequest(const std::string& url);
};
