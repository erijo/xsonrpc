// This file is part of xsonrpc, an XML/JSON RPC library.
// Copyright (C) 2015 Erik Johansson <erik@ejohansson.se
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

#include "xmlreader.h"

#include "fault.h"
#include "request.h"
#include "response.h"
#include "util.h"
#include "value.h"
#include "xml.h"

namespace xsonrpc {

using namespace xml;

XmlReader::XmlReader(const char* data, size_t size)
{
  auto error = myDocument.Parse(data, size);
  if (error == tinyxml2::XML_CAN_NOT_CONVERT_TEXT) {
    throw InvalidCharacterFault();
  }
  else if (error != tinyxml2::XML_NO_ERROR) {
    throw NotWellFormedFault();
  }
}

Request XmlReader::GetRequest()
{
  auto root = myDocument.RootElement();

  if (!root || !util::IsTag(*root, METHOD_CALL_TAG)) {
    throw InvalidXmlRpcFault("missing method call element");
  }

  auto name = root->FirstChildElement(METHOD_NAME_TAG);
  if (!name || util::HasEmptyText(*name)) {
    throw InvalidXmlRpcFault("missing method name");
  }

  auto params = root->FirstChildElement(PARAMS_TAG);
  if (!params) {
    return Request(name->GetText(), {});
  }

  Request::Parameters parameters;
  for (auto param = params->FirstChildElement(PARAM_TAG);
       param; param = param->NextSiblingElement(PARAM_TAG)) {
    parameters.emplace_back(GetValue(param->FirstChildElement()));
  }
  return Request(name->GetText(), std::move(parameters));
}

Response XmlReader::GetResponse()
{
  auto root = myDocument.RootElement();

  if (!root || !util::IsTag(*root, METHOD_RESPONSE_TAG)) {
    throw InvalidXmlRpcFault("missing method response element");
  }

  bool isFault;
  auto value = root->FirstChildElement(PARAMS_TAG);
  if (value) {
    isFault = false;
    value = value->FirstChildElement(PARAM_TAG);
  }
  else {
    isFault = true;
    value = root->FirstChildElement(FAULT_TAG);
  }

  if (!value) {
    throw InvalidXmlRpcFault("missing param or fault element");
  }

  auto result = GetValue(value->FirstChildElement());
  if (!isFault) {
    return Response(std::move(result));
  }

  if (!result.IsStruct()) {
    throw InvalidXmlRpcFault("fault is not a struct");
  }
  auto& s = result.AsStruct();
  auto code = s.find(FAULT_CODE_NAME);
  if (code == s.end() || !code->second.IsInteger32()) {
    throw InvalidXmlRpcFault("missing or invalid fault code");
  }
  auto string = s.find(FAULT_STRING_NAME);
  if (string == s.end() || !string->second.IsString()) {
    throw InvalidXmlRpcFault("missing or invalid fault string");
  }
  return Response(code->second.AsInteger32(), string->second.AsString());
}

Value XmlReader::GetValue()
{
  return GetValue(myDocument.RootElement());
}

Value XmlReader::GetValue(tinyxml2::XMLElement* element)
{
  if (!element || !util::IsTag(*element, VALUE_TAG)) {
    throw InvalidXmlRpcFault("missing value element");
  }

  auto value = element->FirstChildElement();
  if (!value) {
    throw InvalidXmlRpcFault("empty value element");
  }

  if (util::IsTag(*value, ARRAY_TAG)) {
    auto data = value->FirstChildElement(DATA_TAG);
    if (!data) {
      throw InvalidXmlRpcFault("missing data element in array");
    }
    Value::Array array;
    for (auto child = data->FirstChildElement();
         child; child = child->NextSiblingElement()) {
      array.emplace_back(GetValue(child));
    }
    return Value(std::move(array));
  }
  else if (util::IsTag(*value, BASE_64_TAG)) {
    auto text = value->GetText();
    if (!text) {
      throw InvalidXmlRpcFault("value is not base64");
    }
    return Value(util::Base64Decode(text, strlen(text)));
  }
  else if (util::IsTag(*value, BOOLEAN_TAG)) {
    bool data;
    if (value->QueryBoolText(&data) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a boolean");
    }
    return Value(data);
  }
  else if (util::IsTag(*value, DATE_TIME_TAG)) {
    Value::DateTime dateTime;
    if (!util::ParseIso8601DateTime(value->GetText(), dateTime)) {
      throw InvalidXmlRpcFault("value is not a date/time");
    }
    return Value(dateTime);
  }
  else if (util::IsTag(*value, DOUBLE_TAG)) {
    double data;
    if (value->QueryDoubleText(&data) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a double");
    }
    return Value(data);
  }
  else if (util::IsTag(*value, INTEGER_32_TAG)
           || util::IsTag(*value, INTEGER_INT_TAG)) {
    int32_t data;
    if (value->QueryIntText(&data) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a 32-bit integer");
    }
    return Value(data);
  }
  else if (util::IsTag(*value, INTEGER_64_TAG)) {
    if (!value->GetText()) {
      throw InvalidXmlRpcFault("value is not a 64-bit integer");
    }
    int64_t data = std::stoll(value->GetText());
    return Value(data);
  }
  else if (util::IsTag(*value, NIL_TAG)) {
    return Value();
  }
  else if (util::IsTag(*value, STRING_TAG)) {
    auto text = value->GetText();
    return Value(std::string(text ? text : ""));
  }
  else if (util::IsTag(*value, STRUCT_TAG)) {
    Value::Struct data;
    for (auto member = value->FirstChildElement(MEMBER_TAG);
         member; member = member->NextSiblingElement(MEMBER_TAG)) {
      auto name = member->FirstChildElement(NAME_TAG);
      if (!name || util::HasEmptyText(*name)) {
        throw InvalidXmlRpcFault("missing name element in struct");
      }
      data.emplace(name->GetText(),
                   GetValue(member->LastChildElement(VALUE_TAG)));
    }
    return Value(std::move(data));
  }
  else {
    throw InvalidXmlRpcFault("invalid type");
  }
}

} // namespace xsonrpc
