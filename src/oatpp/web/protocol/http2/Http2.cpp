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

#include <arpa/inet.h>

#include "oatpp/core/data/stream/BufferStream.hpp"

#include "Http2.hpp"
#include "hpack/Hpack.hpp"


namespace oatpp { namespace web { namespace protocol { namespace http2 {

const char* error::stringRepresentation(ErrorCode code) {
#define ENUM2STR(x) case x: return #x
  switch (code) {
    ENUM2STR(UNKNOWN_ERROR);
    ENUM2STR(PROTOCOL_ERROR);
    ENUM2STR(INTERNAL_ERROR);
    ENUM2STR(FLOW_CONTROL_ERROR);
    ENUM2STR(SETTINGS_TIMEOUT);
    ENUM2STR(STREAM_CLOSED);
    ENUM2STR(FRAME_SIZE_ERROR);
    ENUM2STR(REFUSED_STREAM);
    ENUM2STR(CANCEL);
    ENUM2STR(COMPRESSION_ERROR);
    ENUM2STR(CONNECT_ERROR);
    ENUM2STR(ENHANCE_YOUR_CALM);
    ENUM2STR(INADEQUATE_SECURITY);
    ENUM2STR(HTTP_1_1_REQUIRED);
  }
#undef ENUM2STR
  return nullptr;
}

const char* Frame::Header::TAG = "oatpp::web::protocol::http2::Frame::Header";

const char *Frame::Header::frameTypeStringRepresentation(Frame::Header::FrameType t) {
#define ENUM2STR(x) case x: return #x
  switch (t) {
    ENUM2STR(DATA);
    ENUM2STR(HEADERS);
    ENUM2STR(PRIORITY);
    ENUM2STR(RST_STREAM);
    ENUM2STR(SETTINGS);
    ENUM2STR(PUSH_PROMISE);
    ENUM2STR(PING);
    ENUM2STR(GOAWAY);
    ENUM2STR(WINDOW_UPDATE);
    ENUM2STR(CONTINUATION);
  }
#undef ENUM2STR
  return nullptr;
}

Frame::Header::Header(v_uint32 length, v_uint8 flags, FrameType type, v_uint32 streamId)
  : m_length(length)
  , m_flags(flags)
  , m_type(type)
  , m_streamId(streamId) {
}

std::shared_ptr<Frame::Header> Frame::Header::createShared(const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream) {
  v_uint8 data[9] = {0};
  data::share::MemoryLabel label(nullptr, data, 9);
  data::buffer::InlineReadData inlineData((void*)data, 9);
  if (stream->readExactSizeDataSimple(inlineData) != 9) {
    OATPP_LOGE(TAG, "Error: Could not read http2 frame header from stream.");
    throw std::runtime_error("[oatpp::web::protocol::http2::Frame::Header] Error: Could not read http2 frame header from stream.");
  }
  return createShared(label);
}

std::shared_ptr<Frame::Header> Frame::Header::createShared(const data::share::MemoryLabel &label) {
  p_uint8 dataptr = (p_uint8) label.getData();
  v_uint32 payloadLength = ((*dataptr) << 16) | (*(dataptr + 1) << 8) | (*(dataptr + 2));
  dataptr += 3;
  auto type = (FrameType) *dataptr++;
  v_uint8 flags = *dataptr++;
  v_uint32 streamIdent = ((*(dataptr) & 0x7f) << 24) | (*(dataptr + 1) << 16) | (*(dataptr + 2) << 8) | (*(dataptr + 3));
  return std::make_shared<Header>(payloadLength, flags, type, streamIdent);
}

v_io_size Frame::Header::writeToStream(data::stream::OutputStream *stream) {
  stream->writeCharSimple((m_length >> 16) & 0xff);
  stream->writeCharSimple((m_length >> 8) & 0xff);
  stream->writeCharSimple(m_length & 0xff);

  stream->writeSimple(&m_type, 1);
  stream->writeSimple(&m_flags, 1);

  v_uint32 streamIdent = htonl(m_streamId);
  stream->writeSimple(&streamIdent, 4);
  return HeaderSize;
}

oatpp::String Frame::Header::toString() {
  data::stream::BufferOutputStream bos(HeaderSize);
  writeToStream(&bos);
  return bos.toString();
}

v_uint32 Frame::Header::getLength() const {
  return m_length;
}

v_uint8 Frame::Header::getFlags() const {
  return m_flags;
}

Frame::Header::FrameType Frame::Header::getType() const {
  return m_type;
}

v_uint32 Frame::Header::getStreamId() const {
  return m_streamId;
}


oatpp::async::CoroutineStarterForResult<const std::shared_ptr<Frame::Header>&>
Frame::Header::readFrameHeaderAsync(const std::shared_ptr<oatpp::data::stream::IOStream>& connection)
{

  class ReaderCoroutine : public oatpp::async::CoroutineWithResult<ReaderCoroutine, const std::shared_ptr<Frame::Header>&> {
   private:
    v_uint8 m_buffer[9];
    v_uint32 m_pos;
    data::share::MemoryLabel m_label;
    std::shared_ptr<oatpp::data::stream::IOStream> m_connection;

   public:

    ReaderCoroutine(const std::shared_ptr<oatpp::data::stream::IOStream>& connection)
        : m_connection(connection)
        , m_buffer{}
        , m_label(nullptr, m_buffer, 9)
    {}

    Action act() override {

      async::Action action;
      auto bufferData = (p_char8) m_buffer;
      auto res = m_connection->read(bufferData, 9 - m_pos, action);

      if(!action.isNone()) {
        return action;
      }

      if(m_pos == 9) {
        return _return(Frame::Header::createShared(m_label));
      } else {
        if (res > 0) {
          m_pos += res;
          return repeat();
        } else if (res == IOError::RETRY_READ || res == IOError::RETRY_WRITE) {
          return repeat();
        }

      }

      return error<Error>("[oatpp::web::protocol::http2::Frame::Header::readFrameHeaderAsync()]: Error. Error reading connection stream.");

    }

  };

  return ReaderCoroutine::startForResult(connection);

}

}}}}