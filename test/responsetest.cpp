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

#include "response.h"

#include <catch.hpp>
#include <tinyxml2.h>

using namespace xsonrpc;

namespace {

std::string ToXml(const Response& response)
{
  tinyxml2::XMLPrinter printer(nullptr);
  response.Print(printer);
  return printer.CStr();
}

} // namespace

TEST_CASE("bool response")
{
  Response response(Value(true));
  CHECK_FALSE(response.IsFault());
  CHECK_NOTHROW(response.ThrowIfFault());
  CHECK(response.GetResult().AsBoolean());

  CHECK(ToXml(response) ==
        "<?xml version=\"1.0\"?>\n"
        "<methodResponse>\n"
        "    <params>\n"
        "        <param>\n"
        "            <value><boolean>1</boolean></value>\n"
        "        </param>\n"
        "    </params>\n"
        "</methodResponse>\n");
}

TEST_CASE("fault response")
{
  Response response(Fault("test", 123));
  CHECK(response.IsFault());
  CHECK_THROWS_AS(response.ThrowIfFault(), Fault);

  CHECK(ToXml(response) ==
        "<?xml version=\"1.0\"?>\n"
        "<methodResponse>\n"
        "    <fault>\n"
        "        <value>\n"
        "            <struct>\n"
        "                <member>\n"
        "                    <name>faultCode</name>\n"
        "                    <value><i4>123</i4></value>\n"
        "                </member>\n"
        "                <member>\n"
        "                    <name>faultString</name>\n"
        "                    <value><string>test</string></value>\n"
        "                </member>\n"
        "            </struct>\n"
        "        </value>\n"
        "    </fault>\n"
        "</methodResponse>\n");
}
