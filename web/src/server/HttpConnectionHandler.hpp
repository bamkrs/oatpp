/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
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

#ifndef oatpp_web_server_HttpConnectionHandler_hpp
#define oatpp_web_server_HttpConnectionHandler_hpp

#include "./handler/ErrorHandler.hpp"

#include "./HttpRouter.hpp"

#include "../protocol/http/incoming/Request.hpp"
#include "../protocol/http/outgoing/Response.hpp"

#include "../../../../oatpp-lib/network/src/server/Server.hpp"
#include "../../../../oatpp-lib/network/src/Connection.hpp"

#include "../../../../oatpp-lib/core/src/concurrency/Thread.hpp"
#include "../../../../oatpp-lib/core/src/concurrency/Runnable.hpp"

#include "../../../../oatpp-lib/core/src/data/stream/StreamBufferedProxy.hpp"
#include "../../../../oatpp-lib/core/src/data/buffer/IOBuffer.hpp"

namespace oatpp { namespace web { namespace server {
  
class HttpConnectionHandler : public base::Controllable, public network::server::ConnectionHandler {
private:
  class Task : public base::Controllable, public concurrency::Runnable{
  private:
    HttpRouter* m_router;
    std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
    std::shared_ptr<handler::ErrorHandler> m_errorHandler;
    
  public:
    Task(HttpRouter* router,
         const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
         const std::shared_ptr<handler::ErrorHandler>& errorHandler)
      : m_router(router)
      , m_connection(connection)
      , m_errorHandler(errorHandler)
    {}
  public:
    
    static std::shared_ptr<Task> createShared(HttpRouter* router,
                                        const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
                                        const std::shared_ptr<handler::ErrorHandler>& errorHandler){
      return std::shared_ptr<Task>(new Task(router, connection, errorHandler));
    }
    
    void run() override;
    
  };
  
private:
  std::shared_ptr<HttpRouter> m_router;
  std::shared_ptr<handler::ErrorHandler> m_errorHandler;
  
public:
  HttpConnectionHandler(const std::shared_ptr<HttpRouter>& router)
    : m_router(router)
    , m_errorHandler(handler::DefaultErrorHandler::createShared())
  {}
public:
  
  static std::shared_ptr<HttpConnectionHandler> createShared(const std::shared_ptr<HttpRouter>& router){
    return std::shared_ptr<HttpConnectionHandler>(new HttpConnectionHandler(router));
  }
  
  void setErrorHandler(const std::shared_ptr<handler::ErrorHandler>& errorHandler){
    m_errorHandler = errorHandler;
    if(!m_errorHandler) {
      m_errorHandler = handler::DefaultErrorHandler::createShared();
    }
  }
  
  void handleConnection(const std::shared_ptr<oatpp::data::stream::IOStream>& connection) override;
  
};
  
}}}

#endif /* oatpp_web_server_HttpConnectionHandler_hpp */