#include "../LibuvCurlCpp.h"
#include <gtest/gtest.h>
#include <uv.h>

TEST(ConfigTest, BasicAssertions) {

  LibuvCurlCpp::request_options options;
  std::unordered_map<std::string, std::string> headers;
  headers["Host"] = "ya.ru";

  options["method"] = "GET";
  options["url"] = "http://127.0.0.1:8000/";
  options["headers"] = headers;
  auto n = new int(123);
  EXPECT_EQ(*n, 123);

  LibuvCurlCpp::LibuvCurlCpp<int>::request(
      options, [](uv_async_t *handle) {
        auto *po = reinterpret_cast<LibuvCurlCpp::response<int *> *>(handle->data);
        EXPECT_EQ(*po->user_payload, 123);

        uv_close(reinterpret_cast<uv_handle_t *>(handle), [](uv_handle_t *t) { delete t; });
      },
      n);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}