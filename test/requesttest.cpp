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

#include "request.h"

#include <catch.hpp>
#include <tinyxml2.h>

using namespace xsonrpc;

TEST_CASE("invalid request")
{
  tinyxml2::XMLDocument document;

  GIVEN("no document")
  {
    CHECK(document.RootElement() == nullptr);
  }

  GIVEN("no method name")
  {
    document.Parse("<methodCall/>");
  }

  GIVEN("empty method name")
  {
    document.Parse("<methodCall><methodName/></methodCall>");
  }
  
  CHECK_THROWS_AS(Request(document.RootElement()), std::exception);
}

TEST_CASE("only method name")
{
  tinyxml2::XMLDocument document;

  GIVEN("no params tag")
  {
    document.Parse("<methodCall>"
                   "<methodName>test</methodName>"
                   "</methodCall>");
  }
  
  GIVEN("empty params tag")
  {
    document.Parse("<methodCall>"
                   "<methodName>test</methodName>"
                   "<params/>"
                   "</methodCall>");
  }
  
  Request request(document.RootElement());
  CHECK(request.GetMethodName() == "test");
  CHECK(request.GetParameters().empty());  
}

TEST_CASE("one parameter")
{
  tinyxml2::XMLDocument document;
  document.Parse("<methodCall>"
                 "<methodName>test</methodName>"
                 "<params>"
                 "<param><value><int>47</int></value></param>"
                 "</params>"
                 "</methodCall>");

  Request request(document.RootElement());
  CHECK(request.GetMethodName() == "test");
  REQUIRE_FALSE(request.GetParameters().empty());
  CHECK(request.GetParameters()[0].IsInteger32());
}


TEST_CASE("three parameters")
{
  tinyxml2::XMLDocument document;
  document.Parse("<methodCall>"
                 "<methodName>test</methodName>"
                 "<params>"
                 "<param><value><int>47</int></value></param>"
                 "<param><value><int>46</int></value></param>"
                 "<param><value><int>45</int></value></param>"
                 "</params>"
                 "</methodCall>");

  Request request(document.RootElement());
  CHECK(request.GetMethodName() == "test");
  REQUIRE(request.GetParameters().size() == 3);
  CHECK(request.GetParameters()[0].AsInteger32() == 47);
  CHECK(request.GetParameters()[1].AsInteger32() == 46);
  CHECK(request.GetParameters()[2].AsInteger32() == 45);
}
