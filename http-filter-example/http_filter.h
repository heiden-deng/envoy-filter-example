#pragma once

#include <string>
#include <map>

#include "envoy/server/filter_config.h"
#include "envoy/thread/thread.h"
#include "http-filter-example/http_filter.pb.h"

namespace Envoy {
namespace Http {


typedef std::map<std::string,std::string> stringMap;
class HttpSampleDecoderFilterConfig {
public:
  HttpSampleDecoderFilterConfig(const sample::Decoder& proto_config);

  const std::string& filename() const { return filename_; }

  static ThreadId currentThreadId();
  static stringMap create_map(){
    stringMap map_tmp;
    return map_tmp;
  }

  stringMap & get_custom_headers(){return custom_headers_;}
  

  std::string trim(std::string str){
    std::string::size_type pos = 0;
    while(str.npos != (pos = str.find(" "))){
      str = str.replace(pos, pos+1, "");
    }
    return str;
  }

private:
  const std::string filename_;
  static stringMap custom_headers_;

};

typedef std::shared_ptr<HttpSampleDecoderFilterConfig> HttpSampleDecoderFilterConfigSharedPtr;


class HttpSampleDecoderFilter : public StreamDecoderFilter {
public:
  HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr);
  ~HttpSampleDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;
  HttpSampleDecoderFilterConfigSharedPtr getHttpSampleDecoderFilterConfigSharedPtr(){return config_;}

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  FilterTrailersStatus decodeTrailers(RequestTrailerMap&) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;

private:
  void BuildRequestCustomHeader(RequestHeaderMap&, HttpSampleDecoderFilterConfigSharedPtr);
  void BuildCustomHeader(HeaderMap&, HttpSampleDecoderFilterConfigSharedPtr);
  const LowerCaseString headerKey(std::string) const;

  const HttpSampleDecoderFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* decoder_callbacks_;

};

} // namespace Http
} // namespace Envoy
