C++ implementation this example https://curl.se/libcurl/c/multi-uv.html.

HTTP clint based on event loop ([libuv](https://github.com/libuv/libuv)) and curl with poll type sockets

How to use: copy LibuvCurlCpp.h in you project.
Example usage:   

```
LibuvCurlCpp::request_options options;
std::unordered_map<std::string, std::string> headers;   
headers["Accept"] = "*";   
headers["Host"] = "example.com";   
options["method"] = "GET";   
options["url"] = "http://localhost:3000/";   
options["headers"] = headers;  


LibuvCurlCpp::LibuvCurlCpp::request(options, []() {   
std::cout << "DONE!!!!!!!!" << std::endl;   
});   
return uv_run(uv_default_loop(), UV_RUN_DEFAULT);  
```   

Project in development.