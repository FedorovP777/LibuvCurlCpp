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

  LibuvCurlCpp::LibuvCurlCpp<int>::request(
      options, [](uv_async_t *handle) {
        EXPECT_EQ("123123", "123123");
      },
      n);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}