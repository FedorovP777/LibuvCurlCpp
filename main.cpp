// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "LibuvCurlCpp.h"
#include <iostream>
#include <uv.h>
using namespace std;
int main() {
  std::cout << "Hello, World!" << std::endl;
  std::unordered_map<std::string, std::string> options;
  options["method"] = "POST";

  LibuvCurlCpp::request("https://gist.github.com/FedorovP777/6638fc36daf3ce7435eb13378b12da83", []() {
    std::cout << "DONE!!!!!!!!" << std::endl;
  });
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
