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

#include "response.h"

#include "fault.h"
#include "jsonformathandler.h"
#include "xmlformathandler.h"
#include "../src/reader.h"
#include "../src/writer.h"

#include <catch.hpp>

using namespace xsonrpc;

namespace {

std::string ToJson(const Response& response)
{
  auto writer = JsonFormatHandler().CreateWriter();
  response.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

std::string ToXml(const Response& response)
{
  auto writer = XmlFormatHandler().CreateWriter();
  response.Write(*writer);
  return std::string(writer->GetData(), writer->GetSize());
}

} // namespace

TEST_CASE("bool response")
{
  Response response(Value(true), 13);
  CHECK_FALSE(response.IsFault());
  CHECK_NOTHROW(response.ThrowIfFault());
  CHECK(response.GetResult().AsBoolean());

  CHECK(ToJson(response) ==
        R"({"jsonrpc":"2.0","id":13,"result":true})");
  CHECK(ToXml(response) ==
        "<?xml version=\"1.0\"?>"
        "<methodResponse>"
        "<params><param>"
        "<value><boolean>1</boolean></value>"
        "</param></params>"
        "</methodResponse>");
}

TEST_CASE("fault response")
{
  Response response(123, "test", "1");
  CHECK(response.IsFault());
  CHECK_THROWS_AS(response.ThrowIfFault(), Fault);

  CHECK(ToJson(response) ==
        R"({"jsonrpc":"2.0","id":"1","error":{"code":123,"message":"test"}})");
  CHECK(ToXml(response) ==
        "<?xml version=\"1.0\"?>"
        "<methodResponse>"
        "<fault><value><struct>"
        "<member><name>faultCode</name>"
        "<value><i4>123</i4></value></member>"
        "<member><name>faultString</name>"
        "<value><string>test</string></value></member>"
        "</struct></value></fault>"
        "</methodResponse>");
}
