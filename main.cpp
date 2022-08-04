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
  //  headers["Host"] = "example.com";
  options["method"] = "GET";
  options["url"] = "https://ya123123.ru/";
  options["headers"] = headers;
  //  options["timeout"] = headers;

  LibuvCurlCpp::LibuvCurlCpp::request(options, [](string *body) {
    std::cout << *body << std::endl;
  });
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
