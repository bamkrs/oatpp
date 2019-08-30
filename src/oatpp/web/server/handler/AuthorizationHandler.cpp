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

#include "AuthorizationHandler.hpp"

#include "oatpp/encoding/Base64.hpp"
#include "oatpp/core/parser/Caret.hpp"

namespace oatpp { namespace web { namespace server { namespace handler {

std::shared_ptr<handler::AuthorizationObject> BasicAuthorizationHandler::handleAuthorization(const oatpp::String &header) {

  if(!header->startsWith("Basic ")) {
    return nullptr;
  }

  oatpp::String auth = oatpp::encoding::Base64::decode(header->c_str() + 6, header->getSize() - 6);
  parser::Caret caret(auth);

  if(caret.findChar(':')) {
    oatpp::String userId((const char*)&caret.getData()[0], caret.getPosition(), true /* copy as own data */);
    oatpp::String password((const char*)&caret.getData()[caret.getPosition() + 1], caret.getDataSize() - caret.getPosition() - 1, true /* copy as own data */);
    return authorize(userId, password);
  }

  return nullptr;

}

std::shared_ptr<AuthorizationObject> BasicAuthorizationHandler::authorize(const oatpp::String &userId,
                                                                          const oatpp::String &password) {
  auto authorizationObject = std::make_shared<DefaultBasicAuthorizationObject>();
  authorizationObject->userId = userId;
  authorizationObject->password = password;

  return authorizationObject;
}

oatpp::String BasicAuthorizationHandler::generateAuthorizationRequest(const oatpp::String &realm, const oatpp::String &param) {
  (void) param;

  if (realm) {
    return "Basic realm=\"" + realm + "\"";
  }

  return "Basic realm=\"API\"";
}

oatpp::String BasicAuthorizationHandler::generateAuthorizationInfo(const oatpp::String &realm, const oatpp::String &param) {
  (void)realm; (void)param;

  return "Basic";
}

}}}}