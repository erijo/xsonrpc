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

#include "value.h"

#include "fault.h"
#include "jsonformathandler.h"
#include "xmlformathandler.h"
#include "../src/reader.h"
#include "../src/writer.h"

#include <catch.hpp>
#include <memory>

using namespace xsonrpc;

namespace {

std::string ToJson(const Value& value)
{
  auto writer = JsonFormatHandler().CreateWriter();
  value.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

std::unique_ptr<Value> FromJson(const char* json)
{
  auto reader = JsonFormatHandler().CreateReader(json);
  return std::unique_ptr<Value>{new Value(reader->GetValue())};
}

std::string ToXml(const Value& value)
{
  auto writer = XmlFormatHandler().CreateWriter();
  value.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

std::unique_ptr<Value> FromXml(const char* xml)
{
  std::string document("<value>");
  document += xml;
  document += "</value>";
  auto reader = XmlFormatHandler().CreateReader(std::move(document));
  return std::unique_ptr<Value>{new Value(reader->GetValue())};
}

} // namespace

TEST_CASE("nil")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    value = std::unique_ptr<Value>{new Value};
  }

  GIVEN("from json")
  {
    value = FromJson("null");
  }

  GIVEN("from xml")
  {
    value = FromXml("<nil></nil>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(ToJson(*value) == "null");
  CHECK(ToXml(*value) == "<value><nil/></value>");
}

TEST_CASE("array")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    Value::Array array;
    array.emplace_back(1);
    array.emplace_back(false);

    value = std::unique_ptr<Value>{new Value(std::move(array))};
  }

  GIVEN("from json")
  {
    value = FromJson("[1, false]");
  }

  GIVEN("from xml")
  {
    value = FromXml(
      "<array><data>"
      "  <value><int>1</int></value>"
      "  <value><boolean>0</boolean></value>"
      "</data></array>");
  }

  // Type check
  CHECK(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_NOTHROW(value->AsArray());
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  REQUIRE(value->AsArray().size() == 2);
  CHECK(value->AsArray()[0].AsInteger32() == 1);
  CHECK_FALSE(value->AsArray()[1].AsBoolean());

  CHECK((*value)[0].AsInteger32() == 1);
  CHECK_FALSE((*value)[1].AsBoolean());
  CHECK_THROWS_AS((*value)[2], std::out_of_range);

  CHECK(ToJson(*value) == "[1,false]");
  CHECK(ToXml(*value) ==
        "<value><array><data>"
        "<value><i4>1</i4></value>"
        "<value><boolean>0</boolean></value>"
        "</data></array></value>");
}

TEST_CASE("binary")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    Value::String binary{'"', 'h', 'e', 'l', 'l', '\0', '!', '"'};
    value = std::unique_ptr<Value>{new Value(std::move(binary), true)};
  }

  GIVEN("from json")
  {
    value = FromJson(R"("\"hell\u0000!\"")");
  }

  GIVEN("from xml")
  {
    value = FromXml("<base64>ImhlbGwAISI=</base64>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsBinary());
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsString());
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  REQUIRE(value->AsBinary().size() == 8);
  CHECK(value->AsBinary() ==
        (Value::String{'"', 'h', 'e', 'l', 'l', '\0', '!', '"'}));
  CHECK(value->AsBinary() == value->AsString());

  CHECK(ToJson(*value) == R"("\"hell\u0000!\"")");
  CHECK(ToXml(*value) == "<value><base64>ImhlbGwAISI=</base64></value>");
}

TEST_CASE("boolean")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    value = std::unique_ptr<Value>{new Value(true)};
  }

  GIVEN("from json")
  {
    value = FromJson("true");
  }

  GIVEN("from xml")
  {
    value = FromXml("<boolean>1</boolean>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsBoolean());
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(value->AsBoolean());

  CHECK(ToJson(*value) == "true");
  CHECK(ToXml(*value) == "<value><boolean>1</boolean></value>");
}

TEST_CASE("date time")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    const time_t local = 1427890394;
    value = std::unique_ptr<Value>{new Value(*gmtime(&local))};
  }

  GIVEN("from json")
  {
    value = FromJson("\"20150401T12:13:14\"");
  }

  GIVEN("from xml")
  {
    value = FromXml("<dateTime.iso8601>20150401T12:13:14</dateTime.iso8601>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsDateTime());
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  auto dt = value->AsDateTime();
  CHECK(dt.tm_year == 2015 - 1900);
  CHECK(dt.tm_mon == 4 - 1);
  CHECK(dt.tm_mday == 1);
  CHECK(dt.tm_hour == 12);
  CHECK(dt.tm_min == 13);
  CHECK(dt.tm_sec == 14);
  //CHECK(dt.tm_wday == 3);
  //CHECK(dt.tm_yday == 90);
  CHECK(dt.tm_isdst == -1);

  CHECK(ToJson(*value) == "\"20150401T12:13:14\"");
  CHECK(ToXml(*value) ==
        "<value><dateTime.iso8601>"
        "20150401T12:13:14</dateTime.iso8601></value>");
}

TEST_CASE("double")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    value = std::unique_ptr<Value>{new Value(1.5)};
  }

  GIVEN("from json")
  {
    value = FromJson("1.50");
  }

  GIVEN("from xml")
  {
    value = FromXml("<double>1.50</double>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsDouble());
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(value->AsDouble() == 1.5);

  CHECK(ToJson(*value) == "1.5");
  CHECK(ToXml(*value) == "<value><double>1.5</double></value>");
}

TEST_CASE("integer 32")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    value = std::unique_ptr<Value>{new Value(42)};
  }

  GIVEN("from json")
  {
    value = FromJson("42");
  }

  GIVEN("from xml")
  {
    value = FromXml("<int>42</int>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsDouble());
  CHECK_NOTHROW(value->AsInteger32());
  CHECK_NOTHROW(value->AsInteger64());
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(value->AsDouble() == 42.0);
  CHECK(value->AsInteger32() == 42);
  CHECK(value->AsInteger64() == 42ll);

  CHECK(ToJson(*value) == "42");
  CHECK(ToXml(*value) == "<value><i4>42</i4></value>");
}

TEST_CASE("integer 64")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    value = std::unique_ptr<Value>{new Value(int64_t(-5000000000ll))};
  }

  GIVEN("from json")
  {
    value = FromJson("-5000000000");
  }

  GIVEN("from xml")
  {
    value = FromXml("<i8>-5000000000</i8>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsDouble());
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsInteger64());
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(value->AsDouble() == -5000000000.0);
  CHECK(value->AsInteger64() == -5000000000ll);

  CHECK(ToJson(*value) == "-5000000000");
  CHECK(ToXml(*value) == "<value><i8>-5000000000</i8></value>");
}

TEST_CASE("string")
{
  std::unique_ptr<Value> value;

  GIVEN("from std::string constructor")
  {
    value = std::unique_ptr<Value>{
      new Value(std::string("1 2 3 &amp;"))};
  }

  GIVEN("from const char constructor")
  {
    value = std::unique_ptr<Value>{new Value("1 2 3 &amp;")};
  }

  GIVEN("from json")
  {
    value = FromJson("\"1 2 3 &amp;\"");
  }

  GIVEN("from xml")
  {
    value = FromXml("<string>1 2 3 &amp;amp;</string>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK(value->IsString());
  CHECK_FALSE(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsString());
  CHECK_THROWS_AS(value->AsStruct(), InvalidParametersFault);

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);
  CHECK_THROWS_AS((*value)["notthere"], InvalidParametersFault);

  CHECK(value->AsString() == "1 2 3 &amp;");

  CHECK(ToJson(*value) == "\"1 2 3 &amp;\"");
  CHECK(ToXml(*value) == "<value><string>1 2 3 &amp;amp;</string></value>");
}

TEST_CASE("struct")
{
  std::unique_ptr<Value> value;

  GIVEN("from constructor")
  {
    Value::Struct data;
    data["foo"] = Value(true);
    data["test"] = Value(-34);

    Value::Array array;
    array.emplace_back("a string");
    data["bar"] = Value(std::move(array));

    value = std::unique_ptr<Value>{new Value(std::move(data))};
  }

  GIVEN("from json")
  {
    value = FromJson(R"({"bar": ["a string"], "foo": true, "test":-34})");
  }

  GIVEN("from xml")
  {
    value = FromXml(
      "<struct><member>"
      "  <name>bar</name>"
      "  <value><array><data>"
      "    <value><string>a string</string></value>"
      "  </data></array></value>"
      "</member><member>"
      "  <name>foo</name>"
      "  <value><boolean>1</boolean></value>"
      "</member><member>"
      "  <name>test</name>"
      "  <value><i4>-34</i4></value>"
      "</member></struct>");
  }

  // Type check
  CHECK_FALSE(value->IsArray());
  CHECK_FALSE(value->IsBinary());
  CHECK_FALSE(value->IsBoolean());
  CHECK_FALSE(value->IsDateTime());
  CHECK_FALSE(value->IsDouble());
  CHECK_FALSE(value->IsInteger32());
  CHECK_FALSE(value->IsInteger64());
  CHECK_FALSE(value->IsNil());
  CHECK_FALSE(value->IsString());
  CHECK(value->IsStruct());

  // Getter
  CHECK_THROWS_AS(value->AsArray(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsBoolean(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsDouble(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger32(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsInteger64(), InvalidParametersFault);
  CHECK_THROWS_AS(value->AsString(), InvalidParametersFault);
  CHECK_NOTHROW(value->AsStruct());

  CHECK_THROWS_AS((*value)[0], InvalidParametersFault);

  REQUIRE(value->AsStruct().size() == 3);
  CHECK(value->AsStruct().at("bar").IsArray());
  CHECK(value->AsStruct().at("foo").AsBoolean());
  CHECK(value->AsStruct().at("test").AsInteger32() == -34);

  CHECK((*value)["bar"].IsArray());
  CHECK((*value)["foo"].AsBoolean());
  CHECK((*value)["test"].AsInteger32() == -34);
  CHECK_THROWS_AS((*value)["notthere"], std::out_of_range);

  CHECK(ToJson(*value) ==
        "{\"bar\":[\"a string\"],\"foo\":true,\"test\":-34}");
  CHECK(ToXml(*value) ==
        "<value><struct>"
        "<member><name>bar</name>"
        "<value><array><data>"
        "<value><string>a string</string></value>"
        "</data></array></value></member>"
        "<member><name>foo</name>"
        "<value><boolean>1</boolean></value></member>"
        "<member><name>test</name>"
        "<value><i4>-34</i4></value></member>"
        "</struct></value>");
}
