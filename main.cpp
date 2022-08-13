// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "LibuvCurlCpp.h"
#include <iostream>
#include <uv.h>

using namespace std;
int main() {

  LibuvCurlCpp::request_options options;
  std::unordered_map<std::string, std::string> headers;
  headers["Accept"] = "*";
  options["method"] = "GET";
  options["url"] = "https://ya.ru/";
  options["headers"] = headers;

  LibuvCurlCpp::LibuvCurlCpp::request(options, [](string body, string headers, int curl_code, int http_code) {
    std::cout << body << std::endl;
    std::cout << headers << std::endl;
    std::cout << curl_code << std::endl;
  });
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
