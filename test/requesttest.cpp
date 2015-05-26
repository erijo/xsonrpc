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

#include "request.h"

#include "jsonformathandler.h"
#include "xmlformathandler.h"
#include "../src/reader.h"
#include "../src/writer.h"

#include <catch.hpp>
#include <memory>

using namespace xsonrpc;

namespace {

std::string ToJson(const Request& request)
{
  auto writer = JsonFormatHandler().CreateWriter();
  request.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

std::unique_ptr<Request> FromJson(const char* json)
{
  auto reader = JsonFormatHandler().CreateReader(json);
  return std::unique_ptr<Request>{new Request(reader->GetRequest())};
}

std::string ToXml(const Request& request)
{
  auto writer = XmlFormatHandler().CreateWriter();
  request.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

std::unique_ptr<Request> FromXml(std::string xml)
{
  auto reader = XmlFormatHandler().CreateReader(std::move(xml));
  return std::unique_ptr<Request>(new Request(reader->GetRequest()));
}

} // namespace

TEST_CASE("invalid request")
{
  CHECK_THROWS_AS(
    FromJson(
      R"({"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz])"),
    std::exception);
  CHECK_THROWS_AS(
    FromJson(
      R"({"jsonrpc": "2.0", "method": 1, "params": "bar"})"),
    std::exception);

  CHECK_THROWS_AS(FromXml(""), std::exception);
  CHECK_THROWS_AS(FromXml("<methodCall/>"), std::exception);
  CHECK_THROWS_AS(FromXml("<methodCall><methodName/></methodCall>"),
                  std::exception);
}

TEST_CASE("only method name")
{
  std::unique_ptr<Request> request;

  GIVEN("json: no params")
  {
    request = FromJson(R"({"jsonrpc": "2.0", "method": "test"})");
  }

  GIVEN("xml: no params")
  {
    request = FromXml(
      "<methodCall><methodName>test</methodName></methodCall>");
  }

  GIVEN("json: empty params")
  {
    request = FromJson(
      R"({"jsonrpc": "2.0", "method": "test", "params": []})");
  }

  GIVEN("xml: empty params")
  {
    request = FromXml(
      "<methodCall><methodName>test</methodName><params/></methodCall>");
  }

  CHECK(request->GetMethodName() == "test");
  CHECK(request->GetParameters().empty());
  CHECK(ToJson(*request) ==
        R"({"jsonrpc":"2.0","method":"test","params":[]})");
  CHECK(ToXml(*request) ==
        "<?xml version=\"1.0\"?>"
        "<methodCall>"
        "<methodName>test</methodName>"
        "<params/>"
        "</methodCall>");
}

TEST_CASE("one parameter")
{
  std::unique_ptr<Request> request;

  GIVEN("from json")
  {
    request = FromJson(
      R"({"jsonrpc": "2.0", "method": "test", "params": [47]})");
  }

  GIVEN("from xml")
  {
    request = FromXml(
      "<methodCall><methodName>test</methodName>"
      "<params><param><value><int>47</int></value></param></params>"
      "</methodCall>");
  }

  CHECK(request->GetMethodName() == "test");
  REQUIRE_FALSE(request->GetParameters().empty());
  CHECK(request->GetParameters()[0].IsInteger32());
  CHECK(ToJson(*request) ==
        R"({"jsonrpc":"2.0","method":"test","params":[47]})");
  CHECK(ToXml(*request) ==
        "<?xml version=\"1.0\"?>"
        "<methodCall>"
        "<methodName>test</methodName>"
        "<params><param>"
        "<value><i4>47</i4></value>"
        "</param></params>"
        "</methodCall>");
}

TEST_CASE("three parameters")
{
  std::unique_ptr<Request> request;

  GIVEN("from json")
  {
    request = FromJson(
      R"({"jsonrpc": "2.0", "method": "test", "params": [47,46,45]})");
  }

  GIVEN("from xml")
  {
    request = FromXml(
      "<methodCall><methodName>test</methodName><params>"
      "<param><value><int>47</int></value></param>"
      "<param><value><int>46</int></value></param>"
      "<param><value><int>45</int></value></param>"
      "</params></methodCall>");
  }

  CHECK(request->GetMethodName() == "test");
  REQUIRE(request->GetParameters().size() == 3);
  CHECK(request->GetParameters()[0].AsInteger32() == 47);
  CHECK(request->GetParameters()[1].AsInteger32() == 46);
  CHECK(request->GetParameters()[2].AsInteger32() == 45);
  CHECK(ToJson(*request) ==
        R"({"jsonrpc":"2.0","method":"test","params":[47,46,45]})");
  CHECK(ToXml(*request) ==
        "<?xml version=\"1.0\"?>"
        "<methodCall>"
        "<methodName>test</methodName>"
        "<params>"
        "<param><value><i4>47</i4></value></param>"
        "<param><value><i4>46</i4></value></param>"
        "<param><value><i4>45</i4></value></param>"
        "</params>"
        "</methodCall>");
}
