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
#include "../src/xmlreader.h"
#include "../src/xmlwriter.h"

#include <catch.hpp>
#include <memory>

using namespace xsonrpc;

namespace {

std::string ToXml(const Request& request)
{
  XmlWriter writer;
  request.Write(writer);
  return std::string(writer.GetData(), writer.GetSize());
}

} // namespace

TEST_CASE("invalid request")
{
  std::unique_ptr<XmlReader> reader;

  CHECK_THROWS_AS(reader.reset(new XmlReader(NULL, 0)), std::exception);

  GIVEN("no method name")
  {
    std::string doc = "<methodCall/>";
    reader.reset(new XmlReader(doc.data(), doc.size()));
  }

  GIVEN("empty method name")
  {
    std::string doc = "<methodCall><methodName/></methodCall>";
    reader.reset(new XmlReader(doc.data(), doc.size()));
  }

  CHECK_THROWS_AS(reader->GetRequest(), std::exception);
}

TEST_CASE("only method name")
{
  std::string document;

  GIVEN("no params tag")
  {
    document = "<methodCall><methodName>test</methodName></methodCall>";
  }

  GIVEN("empty params tag")
  {
    document =
      "<methodCall><methodName>test</methodName><params/></methodCall>";
  }

  Request request = XmlReader(document.data(), document.size()).GetRequest();
  CHECK(request.GetMethodName() == "test");
  CHECK(request.GetParameters().empty());
  CHECK(ToXml(request) ==
        "<?xml version=\"1.0\"?>"
        "<methodCall>"
        "<methodName>test</methodName>"
        "<params/>"
        "</methodCall>");
}

TEST_CASE("one parameter")
{
  std::string document =
    "<methodCall><methodName>test</methodName>"
    "<params><param><value><int>47</int></value></param></params>"
    "</methodCall>";

  Request request = XmlReader(document.data(), document.size()).GetRequest();
  CHECK(request.GetMethodName() == "test");
  REQUIRE_FALSE(request.GetParameters().empty());
  CHECK(request.GetParameters()[0].IsInteger32());
  CHECK(ToXml(request) ==
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
  std::string document =
    "<methodCall><methodName>test</methodName><params>"
    "<param><value><int>47</int></value></param>"
    "<param><value><int>46</int></value></param>"
    "<param><value><int>45</int></value></param>"
    "</params></methodCall>";

  Request request = XmlReader(document.data(), document.size()).GetRequest();
  CHECK(request.GetMethodName() == "test");
  REQUIRE(request.GetParameters().size() == 3);
  CHECK(request.GetParameters()[0].AsInteger32() == 47);
  CHECK(request.GetParameters()[1].AsInteger32() == 46);
  CHECK(request.GetParameters()[2].AsInteger32() == 45);
  CHECK(ToXml(request) ==
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
