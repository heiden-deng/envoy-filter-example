#include <string>
#include <fstream>
#include<errno.h>
#include<limits.h>
#include<cstdint>
#include<string>
#include<fstream>
#include<iostream>

#ifdef __linux__
#include <sys/syscall.h>
#elif defined(__APPLE__)
#include <pthread.h>
#endif

#include "http_filter.h"



#include "envoy/common/exception.h"
#include "envoy/server/filter_config.h"
#include "common/common/logger.h"

namespace Envoy {
namespace Http {

int64_t getCurrentThreadId() {
#ifdef __linux__
  return static_cast<int64_t>(syscall(SYS_gettid));
#elif defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(nullptr, &tid);
  return tid;
#else
#error "Enable and test pthread id retrieval code for you arch in pthread/thread_impl.cc"
#endif
}

ThreadId HttpSampleDecoderFilterConfig::currentThreadId(){
   return ThreadId(getCurrentThreadId()); 
}

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const sample::Decoder& proto_config)
    : filename_(proto_config.filename()) {
        if (custom_headers_.size() == 0) {
          printf( "\nread custom configuration from ini file \n");
          std::ios::sync_with_stdio(false);

          std::ifstream file(filename_);
          if (!file) {
            printf("open file error");
            exit(1);
          }
          std::string config_line;
          std::string config_key;
          std::string config_value;
          std::string::size_type equal_div_pos = 0;
          while(std::getline(file, config_line)){
            if(config_line.npos != (equal_div_pos = config_line.find("="))){
                config_key = config_line.substr(0, equal_div_pos);
                config_value = config_line.substr(equal_div_pos+1, config_line.size() - 1);
                config_key = trim(config_key);
                config_value = trim(config_value);
                if (config_key.length() > 0){
                  custom_headers_.insert(std::pair<std::string,std::string>(config_key,config_value));
                }
            }
          }
          custom_headers_.insert(std::pair<std::string,std::string>("CEB_TECH_DEV_PLACEHOLDER","20200416"));
        }

    }

stringMap HttpSampleDecoderFilterConfig::custom_headers_ = HttpSampleDecoderFilterConfig::create_map();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config)
    : config_(config) {
      printf("\n\nHttpSampleDecoderFilter construction called\n\n")
    }

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {
  printf("\n\nHttpSampleDecoderFilter deconstruction called\n\n")
}

void HttpSampleDecoderFilter::onDestroy() {}


void HttpSampleDecoderFilter::BuildRequestCustomHeader(RequestHeaderMap& header, HttpSampleDecoderFilterConfigSharedPtr config){
  printf("HttpSampleDecoderFilter::BuildRequestCustomHeader called \n");
  stringMap::iterator iter = config->get_custom_headers().begin();
  while(iter != config->get_custom_headers().end()){
    header.addCopy(headerKey(iter->first),iter->second);
    iter++;
  }
  printf( "HttpSampleDecoderFilter::BuildRequestCustomHeader called end\n");
}
void HttpSampleDecoderFilter::BuildCustomHeader(HeaderMap& header, HttpSampleDecoderFilterConfigSharedPtr config){
  printf("HttpSampleDecoderFilter::BuildCustomHeader called \n");
  stringMap::iterator iter = config->get_custom_headers().begin();
  while(iter != config->get_custom_headers().end()){
    header.addCopy(headerKey(iter->first),iter->second);
    iter++;
  }
  printf("HttpSampleDecoderFilter::BuildCustomHeader called end\n");
}
const LowerCaseString HttpSampleDecoderFilter::headerKey(std::string key) const {
  return LowerCaseString(key);
}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  printf("HttpSampleDecoderFilter::decodeHeaders called \n");

  printf("\n\nCurrent Thread id= [ %s ]\n\n",HttpSampleDecoderFilterConfig::currentThreadId().debugString().c_str());

  // 添加一个请求头
  BuildRequestCustomHeader(headers, getHttpSampleDecoderFilterConfigSharedPtr());
  // 设置响应体
  auto body_text = fmt::format("{}:{}", "heiden", "haha");
  
  auto stop_flag_iter = getHttpSampleDecoderFilterConfigSharedPtr()->get_custom_headers().find("x-stop-continue-call");
  if(stop_flag_iter != getHttpSampleDecoderFilterConfigSharedPtr()->get_custom_headers().end() 
      && 0 == strcmp(stop_flag_iter->second.c_str(),"true")){
      
      printf("HttpSampleDecoderFilter::decodeHeaders called, stop continue call \n");
      // 添加一个响应头
      auto modify_headers = [this](HeaderMap& headers) -> void {
          //headers.addCopy(headerKey(), headerValue());
          BuildCustomHeader(headers, getHttpSampleDecoderFilterConfigSharedPtr());
      };
      decoder_callbacks_->sendLocalReply(Http::Code::OK, body_text, modify_headers, absl::nullopt, "force return");
      // 本地响应已经发送，必须停止迭代，否则执行到Router会出现断言失败
      return FilterHeadersStatus::StopIteration;
  }
  return FilterHeadersStatus::Continue;
}


FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&) {
  return FilterTrailersStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
