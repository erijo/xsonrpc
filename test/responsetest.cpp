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

#include "fault.h"
#include "response.h"
#include "../src/xmlwriter.h"

#include <catch.hpp>

using namespace xsonrpc;

namespace {

std::string ToXml(const Response& response)
{
  XmlWriter writer;
  response.Write(writer);
  return std::string(writer.GetData(), writer.GetSize());
}

} // namespace

TEST_CASE("bool response")
{
  Response response(Value(true));
  CHECK_FALSE(response.IsFault());
  CHECK_NOTHROW(response.ThrowIfFault());
  CHECK(response.GetResult().AsBoolean());

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
  Response response(123, "test");
  CHECK(response.IsFault());
  CHECK_THROWS_AS(response.ThrowIfFault(), Fault);

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
