#include "../LibuvCurlCpp.h"
#include <gtest/gtest.h>
#include <uv.h>

TEST(GetMethodTest, BasicAssertions)
{

    LibuvCurlCpp::request_options options;
    std::unordered_map<std::string, std::string> headers;
    headers["Host"] = "localhost";

    options["method"] = "GET";
    options["url"] = "http://localhost:8001/";
    options["headers"] = headers;
    auto n = new int(123);
    EXPECT_EQ(*n, 123);

    LibuvCurlCpp::LibuvCurlCpp<int>::request(
        options, [](uv_async_t* handle) {
            auto* po = reinterpret_cast<LibuvCurlCpp::response<int*>*>(handle->data);
            EXPECT_EQ(*po->user_payload, 123);
            EXPECT_EQ(po->http_code, 200);
            EXPECT_EQ(po->body, "HTTP/1.1 200 OK\r\nDate: \r\nServer: \r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 11\r\n\r\nHello world");

            uv_close(reinterpret_cast<uv_handle_t*>(handle), [](uv_handle_t* t) { delete t; });
        },
        n);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

TEST(UploadFileTest, BasicAssertions)
{

    LibuvCurlCpp::request_options options;
    std::unordered_map<std::string, std::string> headers;
    headers["Host"] = "localhost";
    headers["Content-Type"] = "image/jpeg";

    options["method"] = "POST";
    options["file_upload"] = "../tests/upload_file_test.txt";

    options["url"] = "http://localhost:8002/";
    options["headers"] = headers;
    auto n = new int(123);
    EXPECT_EQ(*n, 123);

    LibuvCurlCpp::LibuvCurlCpp<int>::request(
        options, [](uv_async_t* handle) {
            auto* po = reinterpret_cast<LibuvCurlCpp::response<int*>*>(handle->data);
            EXPECT_EQ(*po->user_payload, 123);
            EXPECT_EQ(po->http_code, 200);
            EXPECT_EQ(po->body, "HTTP/1.1 100 Continue\r\n\r\nHTTP/1.1 200 OK\r\nDate: \r\nServer: \r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 10\r\n\r\n0xa7277606");

            uv_close(reinterpret_cast<uv_handle_t*>(handle), [](uv_handle_t* t) { delete t; });
        },
        n);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}