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

#include "util.h"
#include "fault.h"
#include "value.h"

#include <ctime>
#include <ostream>
#include <string>
#include <tinyxml2.h>
#include <typeinfo>

namespace {

const char VALUE_TAG[] = "value";

const char ARRAY_TAG[] = "array";
const char BASE_64_TAG[] = "base64";
const char BOOLEAN_TAG[] = "boolean";
const char DATE_TIME_TAG[] = "dateTime.iso8601";
const char DOUBLE_TAG[] = "double";
const char INTEGER_INT_TAG[] = "int";
const char INTEGER_32_TAG[] = "i4";
const char INTEGER_64_TAG[] = "i8";
const char NIL_TAG[] = "nil";
const char STRING_TAG[] = "string";
const char STRUCT_TAG[] = "struct";

const char DATA_TAG[] = "data";
const char MEMBER_TAG[] = "member";
const char NAME_TAG[] = "name";

const char DATE_TIME_FORMAT[] = "%Y%m%dT%T";

std::string FormatIso8601DateTime(const tm& dt)
{
  char str[128];
  return std::string(str, strftime(str, sizeof(str), DATE_TIME_FORMAT, &dt));
}

} // namespace

namespace xsonrpc {

Value::Value(const tinyxml2::XMLElement* element)
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
    myType = Type::ARRAY;
    Array array;
    for (auto element = data->FirstChildElement();
         element; element = element->NextSiblingElement()) {
      array.emplace_back(element);
    }
    as.myArray = new Array(std::move(array));
  }
  else if (util::IsTag(*value, BASE_64_TAG)) {
    myType = Type::BINARY;
    auto text = value->GetText();
    if (!text) {
      throw InvalidXmlRpcFault("value is not base64");
    }
    auto binary = util::Base64Decode(text, strlen(text));
    as.myBinary = new Binary(std::move(binary));
  }
  else if (util::IsTag(*value, BOOLEAN_TAG)) {
    myType = Type::BOOLEAN;
    if (value->QueryBoolText(&as.myBoolean) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a boolean");
    }
  }
  else if (util::IsTag(*value, DATE_TIME_TAG)) {
    myType = Type::DATE_TIME;
    if (!value->GetText()) {
      throw InvalidXmlRpcFault("value is not a date/time");
    }
    DateTime dateTime;
    memset(&dateTime, 0, sizeof(dateTime));
    auto* res = strptime(value->GetText(), DATE_TIME_FORMAT, &dateTime);
    if (!res || *res != '\0') {
      throw InvalidXmlRpcFault("value is not a valid date/time");
    }
    dateTime.tm_isdst = -1;
    as.myDateTime = new DateTime(dateTime);
  }
  else if (util::IsTag(*value, DOUBLE_TAG)) {
    myType = Type::DOUBLE;
    if (value->QueryDoubleText(&as.myDouble) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a double");
    }
  }
  else if (util::IsTag(*value, INTEGER_32_TAG)
           || util::IsTag(*value, INTEGER_INT_TAG)) {
    myType = Type::INTEGER_32;
    if (value->QueryIntText(&as.myInteger32) != tinyxml2::XML_SUCCESS) {
      throw InvalidXmlRpcFault("value is not a 32-bit integer");
    }
  }
  else if (util::IsTag(*value, INTEGER_64_TAG)) {
    if (!value->GetText()) {
      throw InvalidXmlRpcFault("value is not a 64-bit integer");
    }
    myType = Type::INTEGER_64;
    as.myInteger64 = std::stoll(value->GetText());
  }
  else if (util::IsTag(*value, NIL_TAG)) {
    myType = Type::NIL;
  }
  else if (util::IsTag(*value, STRING_TAG)) {
    myType = Type::STRING;
    as.myString = new String(value->GetText() ? value->GetText() : "");
  }
  else if (util::IsTag(*value, STRUCT_TAG)) {
    myType = Type::STRUCT;

    Struct data;
    for (auto member = value->FirstChildElement(MEMBER_TAG);
         member; member = member->NextSiblingElement(MEMBER_TAG)) {
      auto name = member->FirstChildElement(NAME_TAG);
      if (!name || util::HasEmptyText(*name)) {
        throw InvalidXmlRpcFault("missing name element in struct");
      }
      data.emplace(name->GetText(),
                   Value(member->LastChildElement(VALUE_TAG)));
    }
    as.myStruct = new Struct(std::move(data));
  }
  else {
    throw InvalidXmlRpcFault("invalid type");
  }
}

Value::Value(Array value)
  : myType(Type::ARRAY)
{
  as.myArray = new Array(std::move(value));
}

Value::Value(Binary value)
  : myType(Type::BINARY)
{
  as.myBinary = new Binary(std::move(value));
}

Value::Value(const DateTime& value)
  : myType(Type::DATE_TIME)
{
  as.myDateTime = new DateTime(value);
  as.myDateTime->tm_isdst = -1;
}

Value::Value(String value)
  : myType(Type::STRING)
{
  as.myString = new String(std::move(value));
}

Value::Value(Struct value)
  : myType(Type::STRUCT)
{
  as.myStruct = new Struct(std::move(value));
}

Value::~Value()
{
  Reset();
}

Value::Value(const Value& other)
  : myType(other.myType),
    as(other.as)
{
  switch (myType) {
    case Type::BOOLEAN:
    case Type::DOUBLE:
    case Type::INTEGER_32:
    case Type::INTEGER_64:
    case Type::NIL:
      break;

    case Type::ARRAY:
      as.myArray = new Array(other.AsArray());
      break;
    case Type::BINARY:
      as.myBinary = new Binary(other.AsBinary());
      break;
    case Type::DATE_TIME:
      as.myDateTime = new DateTime(other.AsDateTime());
      break;
    case Type::STRING:
      as.myString = new String(other.AsString());
      break;
    case Type::STRUCT:
      as.myStruct = new Struct(other.AsStruct());
      break;
  }
}

Value::Value(Value&& other) noexcept
  : myType(other.myType),
    as(other.as)
{
  other.myType = Type::NIL;
}

Value& Value::operator=(Value&& other) noexcept
{
  if (this != &other) {
    Reset();

    myType = other.myType;
    as = other.as;

    other.myType = Type::NIL;
  }
  return *this;
}

const Value::Array& Value::AsArray() const
{
  if (IsArray()) {
    return *as.myArray;
  }
  throw InvalidParametersFault();
}

const Value::Binary& Value::AsBinary() const
{
  if (IsBinary()) {
    return *as.myBinary;
  }
  throw InvalidParametersFault();
}

const bool& Value::AsBoolean() const
{
  if (IsBoolean()) {
    return as.myBoolean;
  }
  throw InvalidParametersFault();
}

const Value::DateTime& Value::AsDateTime() const
{
  if (IsDateTime()) {
    return *as.myDateTime;
  }
  throw InvalidParametersFault();
}

const double& Value::AsDouble() const
{
  if (IsDouble()) {
    return as.myDouble;
  }
  throw InvalidParametersFault();
}

const int32_t& Value::AsInteger32() const
{
  if (IsInteger32()) {
    return as.myInteger32;
  }
  throw InvalidParametersFault();
}

const int64_t& Value::AsInteger64() const
{
  if (IsInteger64()) {
    return as.myInteger64;
  }
  throw InvalidParametersFault();
}

const Value::String& Value::AsString() const
{
  if (IsString()) {
    return *as.myString;
  }
  throw InvalidParametersFault();
}

const Value::Struct& Value::AsStruct() const
{
  if (IsStruct()) {
    return *as.myStruct;
  }
  throw InvalidParametersFault();
}

std::string Value::GetTypeName(Type type)
{
  switch (type) {
    case Type::ARRAY:
      return ARRAY_TAG;
    case Type::BINARY:
      return BASE_64_TAG;
    case Type::BOOLEAN:
      return BOOLEAN_TAG;
    case Type::DATE_TIME:
      return DATE_TIME_TAG;
    case Type::DOUBLE:
      return DOUBLE_TAG;
    case Type::INTEGER_32:
      return INTEGER_32_TAG;
    case Type::INTEGER_64:
      return INTEGER_64_TAG;
    case Type::NIL:
      return NIL_TAG;
    case Type::STRING:
      return STRING_TAG;
    case Type::STRUCT:
      return STRUCT_TAG;
  }
  return {};
}

void Value::Print(tinyxml2::XMLPrinter& printer) const
{
  printer.OpenElement(VALUE_TAG);

  bool compact = false;
  switch (myType) {
    case Type::ARRAY: {
      printer.OpenElement(ARRAY_TAG);
      printer.OpenElement(DATA_TAG);
      for (const auto& element : *as.myArray) {
        element.Print(printer);
      }
      printer.CloseElement();
      printer.CloseElement();
      break;
    }
    case Type::BINARY:
      printer.OpenElement(BASE_64_TAG);
      printer.PushText(util::Base64Encode(*as.myBinary).c_str());
      printer.CloseElement();
      break;
    case Type::BOOLEAN:
      compact = true;
      printer.OpenElement(BOOLEAN_TAG, compact);
      printer.PushText(as.myBoolean);
      printer.CloseElement(compact);
      break;
    case Type::DATE_TIME:
      compact = true;
      printer.OpenElement(DATE_TIME_TAG, compact);
      printer.PushText(FormatIso8601DateTime(*as.myDateTime).c_str());
      printer.CloseElement(compact);
      break;
    case Type::DOUBLE:
      compact = true;
      printer.OpenElement(DOUBLE_TAG, compact);
      printer.PushText(as.myDouble);
      printer.CloseElement(compact);
      break;
    case Type::INTEGER_32:
      compact = true;
      printer.OpenElement(INTEGER_32_TAG, compact);
      printer.PushText(as.myInteger32);
      printer.CloseElement(compact);
      break;
    case Type::INTEGER_64:
      compact = true;
      printer.OpenElement(INTEGER_64_TAG, compact);
      printer.PushText(std::to_string(as.myInteger64).c_str());
      printer.CloseElement(compact);
      break;
    case Type::NIL:
      compact = true;
      printer.OpenElement(NIL_TAG, compact);
      printer.CloseElement(compact);
      break;
    case Type::STRING:
      compact = true;
      printer.OpenElement(STRING_TAG, compact);
      printer.PushText(as.myString->c_str());
      printer.CloseElement(compact);
      break;
    case Type::STRUCT: {
      printer.OpenElement(STRUCT_TAG);
      for (const auto& element : *as.myStruct) {
        printer.OpenElement(MEMBER_TAG);

        printer.OpenElement(NAME_TAG);
        printer.PushText(element.first.c_str());
        printer.CloseElement(true);

        element.second.Print(printer);
        printer.CloseElement();
      }
      printer.CloseElement();
      break;
    }
  }

  printer.CloseElement(compact);
}

void Value::Reset()
{
  switch (myType) {
    case Type::ARRAY:
      delete as.myArray;
      break;
    case Type::BINARY:
      delete as.myBinary;
      break;
    case Type::DATE_TIME:
      delete as.myDateTime;
      break;
    case Type::STRING:
      delete as.myString;
      break;
    case Type::STRUCT:
      delete as.myStruct;
      break;

    case Type::BOOLEAN:
    case Type::DOUBLE:
    case Type::INTEGER_32:
    case Type::INTEGER_64:
    case Type::NIL:
      break;
  }

  myType = Type::NIL;
}

std::ostream& operator<<(std::ostream& os, const Value& value)
{
  switch (value.GetType()) {
    case Value::Type::ARRAY: {
      os << '[';
      auto& a = value.AsArray();
      for (auto it = a.begin(); it != a.end(); ++it) {
        if (it != a.begin()) {
          os << ", ";
        }
        os << *it;
      }
      os << ']';
      break;
    }
    case Value::Type::BINARY:
      os << util::Base64Encode(value.AsBinary());
      break;
    case Value::Type::BOOLEAN:
      os << value.AsBoolean();
      break;
    case Value::Type::DATE_TIME:
      os << FormatIso8601DateTime(value.AsDateTime());
      break;
    case Value::Type::DOUBLE:
      os << value.AsDouble();
      break;
    case Value::Type::INTEGER_32:
      os << value.AsInteger32();
      break;
    case Value::Type::INTEGER_64:
      os << value.AsInteger64();
      break;
    case Value::Type::NIL:
      os << "<nil>";
      break;
    case Value::Type::STRING:
      os << '"' << value.AsString() << '"';
      break;
    case Value::Type::STRUCT: {
      os << '{';
      auto& s = value.AsStruct();
      for (auto it = s.begin(); it != s.end(); ++it) {
        if (it != s.begin()) {
          os << ", ";
        }
        os << it->first << ": " << it->second;
      }
      os << '}';
      break;
    }
  }
  return os;
}

} // namespace xsonrpc
