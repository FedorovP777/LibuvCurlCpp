// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#ifndef CLANG_TIDY_LIBUVCURLCPP_H_
#define CLANG_TIDY_LIBUVCURLCPP_H_
#define LOG(msg) std::cout << __FILE__ << ":" << __LINE__ << " " << msg << endl;

#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <uv.h>
#include <variant>

using namespace std;
namespace LibuvCurlCpp

{
using request_options = std::unordered_map<std::string, std::variant<std::string, std::unordered_map<std::string, std::string>>>;
int counter_leak_handlesocketdata = 0;
int counter_leak_curlcontext = 0;
int counter_leak_timerrequest = 0;

template<typename T>
struct response {
  uv_async_t async_cb;
  long http_code;
  T user_payload;
  string body;
};

template<typename T>
class LibuvCurlCpp {
 public:
  /**
     * Make struct for response data.
     * @param payload
     * @param cb
     * @return
     */
  auto static make_response(T *payload, const uv_async_cb cb) {
    auto *response = new ::LibuvCurlCpp::response<T *>;
    uv_async_init(uv_default_loop(), &response->async_cb, cb);
    response->async_cb.data = (void *) response;
    response->user_payload = payload;
    return response;
  }
  struct HandleSocketData {
    CURLM *curl_handle;
    uv_async_t *done_cb;
    HandleSocketData() {
      counter_leak_handlesocketdata++;
      LOG("dtor HandleSocketData " << counter_leak_handlesocketdata)
    }

    ~HandleSocketData() {
      counter_leak_handlesocketdata--;
      LOG("dtor HandleSocketData " << counter_leak_handlesocketdata)
    }
  };

  struct CurlContext {
    uv_poll_t poll_handle;
    CURLM *curl_handle;
    curl_socket_t sockfd;
    uv_async_t *done_cb;
    HandleSocketData *handle_socket_data;

    CurlContext() {
      counter_leak_curlcontext++;
      LOG("dtor CurlContext " << counter_leak_curlcontext)
    }

    ~CurlContext() {
      counter_leak_curlcontext--;
      LOG("dtor CurlContext " << counter_leak_curlcontext)
    }
  };

  struct TimerRequest {
    uv_timer_t uv_timer;
    CURLM *curl_handle;
    int curl_socket_action_running;
    uv_async_t *done_cb;
    curl_slist *request_headers = NULL;

    TimerRequest() {
      counter_leak_timerrequest++;
      LOG("ctor TimerRequest " << counter_leak_timerrequest)
    }

    ~TimerRequest() {
      counter_leak_timerrequest--;
      if (request_headers != NULL) {
        curl_slist_free_all(request_headers);
        request_headers = NULL;
      }
      LOG("dtor TimerRequest " << counter_leak_timerrequest)
    }
  };

  static CurlContext *createCurlContext(curl_socket_t &sockfd, HandleSocketData *hsd) {
    auto *context = new CurlContext;
    context->sockfd = sockfd;
    context->curl_handle = hsd->curl_handle;
    context->done_cb = hsd->done_cb;
    context->handle_socket_data = hsd;
    context->poll_handle.data = context;

    return context;
  }

  static void curlCloseCb(uv_handle_t *handle) { delete reinterpret_cast<CurlContext *>(handle->data); }

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    (reinterpret_cast<std::string *>(userp))->append(reinterpret_cast<char *>(contents), size * nmemb);
    return size * nmemb;
  }

  static void make_request(CURLM *curl_handle, request_options &options, curl_slist *&request_headers) {
    CURL *handle;
    auto *read_buffer = new std::string;
    handle = curl_easy_init();

    if (options.find("method") != options.end() && get<string>(options["method"]) != "GET") {
      curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, get<string>(options["method"]).c_str());
      curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, get<string>(options["body"]).c_str());
      curl_easy_setopt(handle, CURLOPT_POST, 1);
    }

    if (options.find("file_upload") != options.end()) {
      string filename = get<string>(options["file_upload"]);

      FILE *fd = fopen(filename.c_str(), "rb");

      if (!fd)
        throw string("File not found.");

      struct stat file_info;

      if (fstat(fileno(fd), &file_info) != 0)
        throw string("Error file read.");

      curl_easy_setopt(handle, CURLOPT_POST, 1);
      curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(handle, CURLOPT_READDATA, fd);//ToDo: replace on async read
      curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t) file_info.st_size);
      curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    }

    if (options.find("headers") != options.end() && !get<std::unordered_map<std::string, std::string>>(options["headers"]).empty()) {
      curl_slist *s = NULL;
      for (auto &i : get<std::unordered_map<std::string, std::string>>(options["headers"])) {
        stringstream ss;
        ss << i.first << ": " << i.second;
        s = curl_slist_append(s, ss.str().c_str());
      }

      request_headers = s;
      curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request_headers);
    }

    curl_easy_setopt(handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 1);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, read_buffer);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, read_buffer);
    curl_easy_setopt(handle, CURLOPT_URL, get<string>(options["url"]).data());
    curl_multi_add_handle(curl_handle, handle);
  }

  static void checkMultiInfo(CURLM *curl_handle, uv_async_t *done_cb, HandleSocketData *hsd) {
    CURLMsg *message;
    int pending;
    CURL *easy_handle;
    auto http_code = new long;
    string *responseBody;
    string headers;
    string body;
    auto *result_date = reinterpret_cast<response<T> *>(done_cb->data);

    while ((message = curl_multi_info_read(curl_handle, &pending))) {
      switch (message->msg) {
        case CURLMSG_DONE:
          /* Do not use message data after calling curl_multi_remove_handle() and
                           curl_easy_cleanup(). As per curl_multi_info_read() docs:
                           "WARNING: The data the returned pointer points to will not survive
                           calling curl_multi_cleanup, curl_multi_remove_handle or
                           curl_easy_cleanup." */
          easy_handle = message->easy_handle;
          char *done_url;
          curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
          curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &responseBody);
          curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, http_code);

          if (message->data.result != CURLE_OK)
            fprintf(stderr, " %s\n", curl_easy_strerror(message->data.result));

          curl_multi_remove_handle(curl_handle, easy_handle);
          curl_easy_cleanup(easy_handle);
          curl_multi_cleanup(curl_handle);

          //                    found = responseBody->find("\r\n\r\n");
          //                    headers = responseBody->substr(0, found);
          //
          //                    if (found != std::string::npos)
          //                    {
          //                        body = responseBody->substr(found + 4);
          //                    }

          //                    delete responseBody;
          result_date->body = *responseBody;
          result_date->http_code = *http_code;
          LOG("send1")
          uv_async_send(done_cb);
          LOG("send2")
          break;

        default:
          cout << "ERROR!!!" << endl;
          fprintf(stderr, "CURLMSG default\n");
          break;
      }
    }
  }

  static void curlEventPerform(uv_poll_t *req, int /*status*/, int events) {
    LOG("curlEventPerform")
    int running_handles;
    int flags = 0;

    if (events & UV_READABLE)
      flags |= CURL_CSELECT_IN;
    if (events & UV_WRITABLE)
      flags |= CURL_CSELECT_OUT;

    auto *context = reinterpret_cast<CurlContext *>(req->data);
    curl_multi_socket_action(context->curl_handle, context->sockfd, flags, &running_handles);
    checkMultiInfo(context->curl_handle, context->done_cb, context->handle_socket_data);
  }

  static void onTimeout(uv_timer_t *req) {
    LOG("onTimeout")
    auto *timer_req = reinterpret_cast<TimerRequest *>(req->data);
    curl_multi_socket_action(timer_req->curl_handle, CURL_SOCKET_TIMEOUT, 0, &timer_req->curl_socket_action_running);
    checkMultiInfo(timer_req->curl_handle, timer_req->done_cb, nullptr);
    uv_timer_stop(&timer_req->uv_timer);
  }

  /**
     * Callback to receive timeout values
     * @param multi
     * @param timeout_ms A timeout_ms value of -1 passed to this callback means you should delete the timer.
     * All other values are valid expire times in number of milliseconds.
     * @param userp Pointer to user payload
     * @return The timer callback should return 0 on success, and -1 on error. If this callback returns error,
     * all transfers currently in progress in this multi handle will be aborted and fail.
    */
  static int startTimeout(CURLM *multi, long timeout_ms, void *userp) {
    auto *timer_req = reinterpret_cast<TimerRequest *>(userp);
    cout << "timeout " << timeout_ms << endl;
    if (timeout_ms == -1) {
      LOG("DELETE TIMER")
      uv_timer_stop(&timer_req->uv_timer);
      uv_close((uv_handle_t *) &timer_req->uv_timer, [](uv_handle_t *handle) {
        auto *timer_req = reinterpret_cast<TimerRequest *>(handle->data);
        delete timer_req;
        timer_req = nullptr;
      });
    } else {
      LOG("RESUME TIMER")
      if (timeout_ms == 0) {
        timeout_ms = 10;
      }
      uv_timer_start(&timer_req->uv_timer, onTimeout, timeout_ms, 0);
    }
    return 0;
  }

  /**
     *
     * @param easy identifies the specific transfer for which this update is related
     * @param curl_socket_type is the specific socket this function invocation concerns.
     * If the what argument is not CURL_POLL_REMOVE then it holds information about what activity on this socket the application is
     * supposed to monitor. Subsequent calls to this callback might update the what bits for a socket that is already monitored.
     * @param action
     * @param userp
     * @param socketp
     * @return
     */
  static int handleSocket(CURL *easy, curl_socket_t curl_socket_type, int action, void *userp, void *socketp) {
    LOG("Handle Socket")
    auto *handle_socket_data = reinterpret_cast<HandleSocketData *>(userp);
    auto *curl_context = reinterpret_cast<CurlContext *>(socketp);
    int events = 0;

    switch (action) {
      case CURL_POLL_IN:
      case CURL_POLL_OUT:
      case CURL_POLL_INOUT:

        if (!socketp) {
          curl_context = createCurlContext(curl_socket_type, handle_socket_data);
          uv_poll_init_socket(uv_default_loop(), &curl_context->poll_handle, curl_socket_type);
        }
        curl_multi_assign(handle_socket_data->curl_handle, curl_socket_type, reinterpret_cast<void *>(curl_context));

        if (action != CURL_POLL_IN)
          events |= UV_WRITABLE;
        if (action != CURL_POLL_OUT)
          events |= UV_READABLE;

        uv_poll_start(&curl_context->poll_handle, events, curlEventPerform);
        break;
      case CURL_POLL_REMOVE:
        LOG("CURL_POLL_REMOVE" << socketp)
        if (socketp) {
          LOG("socketp exist")
          uv_poll_stop(&curl_context->poll_handle);
          uv_close(reinterpret_cast<uv_handle_t *>(&curl_context->poll_handle), [](uv_handle_t *handle) {
            auto context = reinterpret_cast<CurlContext *>(handle->data);
            delete context;
          });
        }
        curl_multi_assign(handle_socket_data->curl_handle, curl_socket_type, NULL);
        break;
      default:
        LOG("CURL exception")
        abort();
    }

    return 0;
  }

  static int request(request_options &options, uv_async_cb done_cb, T *user_payload) {
    auto response = LibuvCurlCpp::LibuvCurlCpp::make_response(user_payload, done_cb);

    if (curl_global_init(CURL_GLOBAL_ALL)) {
      fprintf(stderr, "Could not init curl\n");
      return 1;
    }

    auto *timer_req = new TimerRequest;
    auto *handle_socket_data = new HandleSocketData;
    CURLM *curl_handle = curl_multi_init();
    timer_req->curl_handle = curl_handle;
    timer_req->done_cb = &response->async_cb;
    timer_req->uv_timer.data = reinterpret_cast<void *>(timer_req);
    uv_timer_init(uv_default_loop(), &timer_req->uv_timer);
    handle_socket_data->done_cb = &response->async_cb;
    handle_socket_data->curl_handle = curl_handle;
    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, handleSocket);
    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETDATA, handle_socket_data);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, startTimeout);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERDATA, timer_req);
    make_request(curl_handle, options, timer_req->request_headers);
    return 0;
  }
};
}// namespace LibuvCurlCpp

#endif//LIBUVCURLCPP