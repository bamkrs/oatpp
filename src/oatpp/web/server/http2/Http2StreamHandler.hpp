/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Benedikt-Alexander Mokroß <github@bamkrs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_web_server_http2_Http2StreamHandler_hpp
#define oatpp_web_server_http2_Http2StreamHandler_hpp

#include <utility>

#include "oatpp/core/base/Countable.hpp"
#include "oatpp/web/protocol/http2/Http2.hpp"
#include "oatpp/web/protocol/http2/hpack/Hpack.hpp"

#include "oatpp/web/protocol/http/utils/CommunicationUtils.hpp"


#include "oatpp/web/server/http2/Http2ProcessingComponents.hpp"

namespace oatpp { namespace web { namespace server { namespace http2 {

class Http2StreamHandler : public oatpp::base::Countable {
 public:

  typedef protocol::http::utils::CommunicationUtils::ConnectionState ConnectionState;

  enum H2StreamState {
    INIT,
    HEADERS,
    PAYLOAD,
    PROCESSING,
    RESPONSE,
    GOAWAY
  };

 private:
  H2StreamState m_state;
  v_uint32 m_streamId;
  std::shared_ptr<protocol::http2::hpack::Hpack> m_hpack;
  std::shared_ptr<http2::processing::Components> m_components;

 public:
  Http2StreamHandler(v_uint32 id, std::shared_ptr<protocol::http2::hpack::Hpack> hpack, std::shared_ptr<http2::processing::Components> components)
    : m_state(INIT)
    , m_streamId(id)
    , m_hpack(std::move(hpack))
    , m_components(std::move(components)) {}

  ConnectionState handleData(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleHeaders(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handlePriority(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleResetStream(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleSettings(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handlePushPromise(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleGoAway(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleWindowUpdate(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);
  ConnectionState handleContinuation(v_uint8 flags, const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream, v_io_size streamPayloadLength);

  H2StreamState getState() {
    return m_state;
  }
};

}}}}

#endif //oatpp_web_server_http2_Http2StreamHandler_hpp