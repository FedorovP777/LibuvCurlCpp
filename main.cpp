#include <iostream>
#include <memory>

extern "C" {
#include <libavutil/parseutils.h>
}

#include "LibuvCurlCpp.h"

void done(uv_async_t *handle) {
  auto result = reinterpret_cast<LibuvCurlCpp::response<int> *>(handle->data);
  std::cout << result->http_code << std::endl;
  std::cout << result->body << std::endl;

  uv_close(reinterpret_cast<uv_handle_t *>(handle), [](uv_handle_t *t) { delete t; });
}

int main() {
  LibuvCurlCpp::request_options options;
  std::unordered_map<std::string, std::string> headers;
  headers["Host"] = "ya.ru";

  options["method"] = "GET";
  options["url"] = "http://ya.ru/";
  options["headers"] = headers;
  auto n = new int(123);

  LibuvCurlCpp::LibuvCurlCpp<int>::request(options, done, n);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  return 0;
}
