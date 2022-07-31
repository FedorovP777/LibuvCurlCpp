#include "../LibuvCurlCpp.h"
#include <gtest/gtest.h>
#include <uv.h>
TEST(ConfigTest, BasicAssertions) {

  LibuvCurlCpp::request("https://gist.github.com/FedorovP777/6638fc36daf3ce7435eb13378b12da83", []() {
    std::cout << "DONE!!!!!!!!" << std::endl;
  });
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  EXPECT_EQ("123123", "rtsp://11.11.11.11:554/test");
}