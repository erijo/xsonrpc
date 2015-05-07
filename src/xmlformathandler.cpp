// This file is part of xsonrpc, an XML/JSON RPC library.
// Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "xmlformathandler.h"

#include "dispatcher.h"
#include "xml.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <cassert>

namespace {

const char TEXT_XML[] = "text/xml";
const char SYSTEM_MULTICALL[] = "system.multicall";
const char SYSTEM_LISTMETHODS[] = "system.listMethods";
const char SYSTEM_METHODSIGNATURE[] = "system.methodSignature";
const char SYSTEM_METHODHELP[] = "system.methodHelp";

} // namespace

namespace xsonrpc {

XmlFormatHandler::XmlFormatHandler()
  : myDispather(nullptr)
{
}

XmlFormatHandler::XmlFormatHandler(
  Dispatcher& dispatcher, std::string requestPath)
  : myDispather(&dispatcher),
    myRequestPath(std::move(requestPath))
{
  myDispather->AddMethod(
    SYSTEM_MULTICALL, &XmlFormatHandler::SystemMulticall, *this)
    .SetHelpText("Call multiple methods at once")
    .AddSignature(Value::Type::ARRAY, Value::Type::ARRAY);
}

void XmlFormatHandler::EnableIntrospection()
{
  assert(myDispather);

  myDispather->AddMethod(
    SYSTEM_LISTMETHODS, &XmlFormatHandler::SystemListMethods, *this)
    .SetHelpText("Returns a list of the methods the server has")
    .AddSignature(Value::Type::ARRAY);

  myDispather->AddMethod(
    SYSTEM_METHODSIGNATURE, &XmlFormatHandler::SystemMethodSignature, *this)
    .SetHelpText("Returns a description of the argument format a particular"
                 " method expects")
    .AddSignature(Value::Type::ARRAY, Value::Type::STRING);

  myDispather->AddMethod(
    SYSTEM_METHODHELP, &XmlFormatHandler::SystemMethodHelp, *this)
    .SetHelpText("Returns a text description of a particular method")
    .AddSignature(Value::Type::STRING, Value::Type::STRING);
}

bool XmlFormatHandler::CanHandleRequest(
  const std::string& path, const std::string& contentType)
{
  // Verify that the correct c-tor was used
  assert(myDispather);
  return path == myRequestPath && contentType == TEXT_XML;
}

std::string XmlFormatHandler::GetContentType()
{
  return TEXT_XML;
}

std::unique_ptr<Reader> XmlFormatHandler::CreateReader(std::string data)
{
  return std::unique_ptr<Reader>(new XmlReader(data.data(), data.size()));
}

std::unique_ptr<Writer> XmlFormatHandler::CreateWriter()
{
  return std::unique_ptr<Writer>(new XmlWriter());
}

Value XmlFormatHandler::SystemMulticall(
  const Request::Parameters& parameters) const
{
  assert(myDispather);

  Value::Array result;
  for (auto& call : parameters.at(0).AsArray()) {
    if (call[xml::METHOD_NAME_TAG].AsString() == SYSTEM_MULTICALL) {
      throw InternalErrorFault("Recursive system.multicall forbidden");
    }
    try {
      auto& array = call[xml::PARAMS_TAG].AsArray();
      Request::Parameters callParams(array.begin(), array.end());
      auto retval = myDispather->Invoke(
        call[xml::METHOD_NAME_TAG].AsString(), callParams);

      retval.ThrowIfFault();
      result.push_back(Value::Array{std::move(retval.GetResult())});
    }
    catch (const Fault& ex) {
      Value::Struct fault;
      fault[xml::FAULT_CODE_NAME] = ex.GetCode();
      fault[xml::FAULT_STRING_NAME] = ex.GetString();
      result.push_back(std::move(fault));
    }
  }
  return result;
}

Value XmlFormatHandler::SystemListMethods() const
{
  assert(myDispather);
  return myDispather->GetMethodNames();
}

Value XmlFormatHandler::SystemMethodSignature(
  const std::string& methodName) const
{
  assert(myDispather);

  try {
    auto& method = myDispather->GetMethod(methodName);
    if (!method.IsHidden()) {
      auto& signatures = method.GetSignatures();
      if (signatures.empty()) {
        return "undef";
      }

      Value::Array result;
      result.reserve(signatures.size());

      for (auto& signature : signatures) {
        Value::Array types;
        types.reserve(signature.size());

        for (auto type : signature) {
          switch (type) {
            case Value::Type::ARRAY:
              types.emplace_back(xml::ARRAY_TAG);
              break;
            case Value::Type::BINARY:
              types.emplace_back(xml::BASE_64_TAG);
              break;
            case Value::Type::BOOLEAN:
              types.emplace_back(xml::BOOLEAN_TAG);
              break;
            case Value::Type::DATE_TIME:
              types.emplace_back(xml::DATE_TIME_TAG);
              break;
            case Value::Type::DOUBLE:
              types.emplace_back(xml::DOUBLE_TAG);
              break;
            case Value::Type::INTEGER_32:
              types.emplace_back(xml::INTEGER_32_TAG);
              break;
            case Value::Type::INTEGER_64:
              types.emplace_back(xml::INTEGER_64_TAG);
              break;
            case Value::Type::NIL:
              // Only useful for return value
              if (types.empty()) {
                types.emplace_back(xml::NIL_TAG);
              }
              break;
            case Value::Type::STRING:
              types.emplace_back(xml::STRING_TAG);
              break;
            case Value::Type::STRUCT:
              types.emplace_back(xml::STRUCT_TAG);
              break;
          }
        }
        result.emplace_back(std::move(types));
      }
      return result;
    }
  }
  catch (...) {
    // Ignore
  }

  throw Fault("No method " + methodName);
}

std::string XmlFormatHandler::SystemMethodHelp(
  const std::string& methodName) const
{
  assert(myDispather);

  try {
    auto& method = myDispather->GetMethod(methodName);
    if (!method.IsHidden()) {
      return method.GetHelpText();
    }
  }
  catch (...) {
    // Ignore
  }

  throw Fault("No method " + methodName);
}

} // namespace xsonrpc
