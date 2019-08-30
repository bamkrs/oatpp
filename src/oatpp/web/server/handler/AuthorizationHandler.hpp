/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <bam@icognize.de>
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

#ifndef oatpp_web_server_handler_AuthorizationHandler_hpp
#define oatpp_web_server_handler_AuthorizationHandler_hpp

#include <oatpp/web/protocol/http/incoming/Request.hpp>
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/data/mapping/type/Type.hpp"


namespace oatpp { namespace web { namespace server { namespace handler {

/**
 * The AuthorizationObject superclass, all AuthorizationObjects have to extend this class.
 */
class AuthorizationObject : public oatpp::base::Countable {
protected:
  AuthorizationObject() = default;
};

/**
 * Default AuthorizationObject - Convenience object to enable Basic-Authorization without the need to implement anything
 */
class DefaultBasicAuthorizationObject : public AuthorizationObject {
public:
  oatpp::String userId;
  oatpp::String password;
};

/**
 * Abstract Authorization Handler.
 */
class AuthorizationHandler {
public:
  /**
   * Convenience typedef for &l:AuthorizationObject;.
   */
  typedef oatpp::web::server::handler::AuthorizationObject AuthorizationObject;
public:

  /**
   * Default virtual destructor.
   */
  virtual ~AuthorizationHandler() = default;

  /**
   * Implement this method! Return nullptr if authorization should be denied.
   * @param header - `Authorization` header. &id:oatpp::String;.
   * @return - `std::shared_ptr` to &id:oatpp::web::server::handler::AuthorizationObject;.
   */
  virtual std::shared_ptr<AuthorizationObject> handleAuthorization(const oatpp::String& authorizationHeader) = 0;

  /**
   * Implement this method! Should generate e.g. 'Basic realm="API"' for basic authentication
   * @return
   */
  virtual oatpp::String generateAuthorizationRequest(const oatpp::String& realm, const oatpp::String& param) = 0;

  /**
   * Implement this method! Should generate e.g. 'Basic' for basic authentication
   * @return
   */
  virtual oatpp::String generateAuthorizationInfo(const oatpp::String& realm, const oatpp::String& param) = 0;

};

/**
 * AuthorizationHandler for Authorization Type `Basic`. <br>
 * See [RFC 7617](https://tools.ietf.org/html/rfc7617). <br>
 * Extend this class to implement Basic Authorization.
 */
class BasicAuthorizationHandler : public AuthorizationHandler {
public:

  /**
   * Implementation of &l:AuthorizationHandler::handleAuthorization ();
   * @param header - &id:oatpp::String;.
   * @return - std::shared_ptr to &id:oatpp::web::server::handler::AuthorizationObject;.
   */
  std::shared_ptr<AuthorizationObject> handleAuthorization(const oatpp::String& header) override;

  /**
   * Implement this method! Do the actual authorization here. When not implemented returns &l:DefaultAuthorizationObject
   * @param userId - user id. &id:oatpp::String;.
   * @param password - password. &id:oatpp::String;.
   * @return - `std::shared_ptr` to &l:AuthorizationObject;.
   */
  virtual std::shared_ptr<AuthorizationObject> authorize(const oatpp::String& userId, const oatpp::String& password);

  /**
   * Implement this method! Should generate e.g. 'Basic realm="API"' for basic authentication
   * @return
   */
  virtual oatpp::String generateAuthorizationRequest(const oatpp::String& realm, const oatpp::String& param);

  /**
   * Implement this method! Should generate e.g. 'Basic' for basic authentication
   * @return
   */
  virtual oatpp::String generateAuthorizationInfo(const oatpp::String& realm, const oatpp::String& param);

};

}}}}

#endif /* oatpp_web_server_handler_ErrorHandler_hpp */
