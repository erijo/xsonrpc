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

#include "xmlrpcsystemmethods.h"

#include "dispatcher.h"
#include "fault.h"
#include "xml.h"

namespace {

const char SYSTEM_MULTICALL[] = "system.multicall";
const char SYSTEM_LISTMETHODS[] = "system.listMethods";
const char SYSTEM_METHODSIGNATURE[] = "system.methodSignature";
const char SYSTEM_METHODHELP[] = "system.methodHelp";
const char SYSTEM_GETCAPABILITIES[] = "system.getCapabilities";

const char SIGNATURE_UNDEFINED[] = "undef";

const char SPEC_URL[] = "specUrl";
const char SPEC_VERSION[] = "specVersion";

const char CAPABILITY_XMLRPC[] = "xmlrpc";
const char CAPABILITY_XMLRPC_URL[] = "http://www.xmlrpc.com/spec";
const int32_t CAPABILITY_XMLRPC_VERSION = 1;

const char CAPABILITY_INTROSPECT[] = "introspect";
const char CAPABILITY_INTROSPECT_URL[] =
    "http://xmlrpc-c.sourceforge.net/xmlrpc-c/introspection.html";
const int32_t CAPABILITY_INTROSPECT_VERSION = 1;

const char CAPABILITY_FAULTS_INTEROP[] = "faults_interop";
const char CAPABILITY_FAULTS_INTEROP_URL[] =
    "http://xmlrpc-epi.sourceforge.net/specs/rfc.fault_codes.php";
const int32_t CAPABILITY_FAULTS_INTEROP_VERSION = 20010516;

} // namespace

namespace xsonrpc {

XmlRpcSystemMethods::XmlRpcSystemMethods(
  Dispatcher& dispatcher, bool introspection)
  : myDispatcher(dispatcher)
{
  AddCapability(CAPABILITY_XMLRPC,
                CAPABILITY_XMLRPC_URL,
                CAPABILITY_XMLRPC_VERSION);

  AddCapability(CAPABILITY_FAULTS_INTEROP,
                CAPABILITY_FAULTS_INTEROP_URL,
                CAPABILITY_FAULTS_INTEROP_VERSION);

  myDispatcher.AddMethod(
    SYSTEM_MULTICALL, &XmlRpcSystemMethods::SystemMulticall, *this)
    .SetHelpText("Call multiple methods at once")
    .AddSignature(Value::Type::ARRAY, Value::Type::ARRAY);

  myDispatcher.AddMethod(
    SYSTEM_GETCAPABILITIES, &XmlRpcSystemMethods::SystemGetCapabilities, *this)
    .SetHelpText("Get server capabilities")
    .AddSignature(Value::Type::STRUCT);

  if (introspection) {
    myDispatcher.AddMethod(
      SYSTEM_LISTMETHODS, &XmlRpcSystemMethods::SystemListMethods, *this)
      .SetHelpText("Returns a list of the methods the server has")
      .AddSignature(Value::Type::ARRAY);

    myDispatcher.AddMethod(
      SYSTEM_METHODSIGNATURE, &XmlRpcSystemMethods::SystemMethodSignature,
      *this)
      .SetHelpText("Returns a description of the argument format a particular"
                   " method expects")
      .AddSignature(Value::Type::ARRAY, Value::Type::STRING);

    myDispatcher.AddMethod(
      SYSTEM_METHODHELP, &XmlRpcSystemMethods::SystemMethodHelp, *this)
      .SetHelpText("Returns a text description of a particular method")
      .AddSignature(Value::Type::STRING, Value::Type::STRING);

    AddCapability(CAPABILITY_INTROSPECT,
                  CAPABILITY_INTROSPECT_URL,
                  CAPABILITY_INTROSPECT_VERSION);
  }
}

XmlRpcSystemMethods::~XmlRpcSystemMethods()
{
  if (myCapabilities.find(CAPABILITY_INTROSPECT) != myCapabilities.end()) {
    myDispatcher.RemoveMethod(SYSTEM_METHODHELP);
    myDispatcher.RemoveMethod(SYSTEM_METHODSIGNATURE);
    myDispatcher.RemoveMethod(SYSTEM_LISTMETHODS);
  }

  myDispatcher.RemoveMethod(SYSTEM_GETCAPABILITIES);
  myDispatcher.RemoveMethod(SYSTEM_MULTICALL);
}

void XmlRpcSystemMethods::AddCapability(
  std::string name, std::string url, int32_t version)
{
  auto result = myCapabilities.emplace(
    std::move(name), Capability{std::move(url), version});
  if (!result.second) {
    throw std::invalid_argument("capability already added");
  }
}

void XmlRpcSystemMethods::RemoveCapability(const std::string& name)
{
  myCapabilities.erase(name);
}

Value XmlRpcSystemMethods::SystemMulticall(
  const Request::Parameters& parameters) const
{
  const Value dummyId;
  Value::Array result;
  for (auto& call : parameters.at(0).AsArray()) {
    try {
      if (call[xml::METHOD_NAME_TAG].AsString() == SYSTEM_MULTICALL) {
        throw InternalErrorFault("Recursive system.multicall forbidden");
      }

      auto& array = call[xml::PARAMS_TAG].AsArray();
      Request::Parameters callParams(array.begin(), array.end());
      auto retval = myDispatcher.Invoke(
        call[xml::METHOD_NAME_TAG].AsString(), callParams, dummyId);

      retval.ThrowIfFault();
      Value::Array a;
      a.emplace_back(std::move(retval.GetResult()));
      result.push_back(std::move(a));
    }
    catch (const Fault& ex) {
      Value::Struct fault;
      fault[xml::FAULT_CODE_NAME] = ex.GetCode();
      fault[xml::FAULT_STRING_NAME] = ex.GetString();
      result.push_back(std::move(fault));
    }
    catch (const std::exception& ex) {
      Value::Struct fault;
      fault[xml::FAULT_CODE_NAME] = 0;
      fault[xml::FAULT_STRING_NAME] = ex.what();
      result.push_back(std::move(fault));
    }
    catch (...) {
      Value::Struct fault;
      fault[xml::FAULT_CODE_NAME] = 0;
      fault[xml::FAULT_STRING_NAME] = "Unknown error";
      result.push_back(std::move(fault));
    }
  }
  return std::move(result);
}

Value XmlRpcSystemMethods::SystemListMethods() const
{
  return myDispatcher.GetMethodNames();
}

Value XmlRpcSystemMethods::SystemMethodSignature(
  const std::string& methodName) const
{
  try {
    auto& method = myDispatcher.GetMethod(methodName);
    if (!method.IsHidden()) {
      auto& signatures = method.GetSignatures();
      if (signatures.empty()) {
        return SIGNATURE_UNDEFINED;
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
      return std::move(result);
    }
  }
  catch (...) {
    // Ignore
  }

  throw Fault("No method " + methodName);
}

std::string XmlRpcSystemMethods::SystemMethodHelp(
  const std::string& methodName) const
{
  try {
    auto& method = myDispatcher.GetMethod(methodName);
    if (!method.IsHidden()) {
      return method.GetHelpText();
    }
  }
  catch (...) {
    // Ignore
  }

  throw Fault("No method " + methodName);
}

Value XmlRpcSystemMethods::SystemGetCapabilities() const
{
  Value::Struct capabilities;
  for (auto& capability : myCapabilities) {
    Value::Struct value;
    value.emplace(SPEC_URL, capability.second.Url);
    value.emplace(SPEC_VERSION, capability.second.Version);
    capabilities.emplace(capability.first, std::move(value));
  }
  return std::move(capabilities);
}

} // namespace xsonrpc
