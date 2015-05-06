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

#include "dispatcher.h"
#include "xmlformathandler.h"

#include <catch.hpp>

using namespace xsonrpc;

namespace {

Value TestMethod(const Request::Parameters& params)
{
  if (params.empty()) {
    return true;
  }
  return Value(params[0]);
}

bool TestMethodBool(bool value)
{
  return value;
}

} // namespace

TEST_CASE("multicall")
{
  Dispatcher dispatcher;
  dispatcher.AddMethod("test", &TestMethodBool);
  dispatcher.AddMethod("foobar", &TestMethod);

  XmlFormatHandler xmlFormatHandler(dispatcher);

  Value::Array args;

  {
    Value::Array params;
    params.emplace_back(1);
    params.emplace_back("test");

    Value::Struct call;
    call["methodName"] = "foobar";
    call["params"] = std::move(params);

    args.push_back(std::move(call));
  }

  {
    Value::Array params;
    params.emplace_back("test");

    Value::Struct call;
    call["methodName"] = "nosuchmethod";
    call["params"] = std::move(params);

    args.push_back(std::move(call));
  }

  {
    Value::Array params;
    params.emplace_back(true);

    Value::Struct call;
    call["methodName"] = "test";
    call["params"] = std::move(params);

    args.push_back(std::move(call));
  }

  Request::Parameters parameters;
  parameters.push_back(std::move(args));

  auto response = dispatcher.Invoke("system.multicall", parameters);
  CAPTURE(response.GetResult());
  REQUIRE_FALSE(response.IsFault());

  auto& value = response.GetResult();
  REQUIRE(value.IsArray());
  REQUIRE(value.AsArray().size() == 3);

  // First result
  CHECK(value[0].IsArray());
  CHECK(value[0].AsArray().size() == 1);
  CHECK(value[0][0].AsInteger32() == 1);

  // Second result
  CHECK(value[1].IsStruct());
  CHECK_NOTHROW(value[1]["faultString"]);
  CHECK(value[1]["faultCode"].AsInteger32() == Fault::METHOD_NOT_FOUND);

  // Third result
  CHECK(value[2].IsArray());
  CHECK(value[2].AsArray().size() == 1);
  CHECK(value[2][0].AsBoolean());
}
