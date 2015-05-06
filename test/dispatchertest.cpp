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

bool TestMethodValue(const Value& value)
{
  return value.AsBoolean();
}

class Test
{
public:
  static Value StaticMethod(const Request::Parameters& params)
  {
    return Value(params[0]);
  }
  static bool StaticMethodBool(bool value)
  {
    return value;
  }
  static Value StaticMethodValue(const Value& value)
  {
    return Value(value);
  }

  bool operator()(bool value)
  {
    return value;
  }

  Value Method(const Request::Parameters& params)
  {
    return Value(params[0]);
  }
  Value MethodConst(const Request::Parameters& params) const
  {
    return Value(params[0]);
  }
  bool MethodBool(bool value)
  {
    return value;
  }
  bool MethodBoolConst(bool value) const
  {
    return value;
  }
  bool MethodValue(const Value& value)
  {
    return value.AsBoolean();
  }
  bool MethodValueConst(const Value& value) const
  {
    return value.AsBoolean();
  }
};

class Test2
{
public:
  bool operator()(const Value& value)
  {
    return value.AsBoolean();
  }
};

} // namespace

TEST_CASE("method wrapper")
{
  MethodWrapper method(&TestMethod);

  CHECK_FALSE(method.HasHelpText());
  method.SetHelpText("test help");
  CHECK(method.HasHelpText());
  CHECK(method.GetHelpText() == "test help");

  method.AddSignature(Value::Type::BOOLEAN);
  method.AddSignature(Value::Type::STRING, Value::Type::STRING);
  method.AddSignature(Value::Type::INTEGER_32, Value::Type::STRING,
                      Value::Type::BOOLEAN);

  auto& signatures = method.GetSignatures();
  REQUIRE(signatures.size() == 3);
  CHECK(signatures[0] == std::vector<Value::Type>{Value::Type::BOOLEAN});
  CHECK(signatures[1] == (std::vector<Value::Type>{
        Value::Type::STRING, Value::Type::STRING}));
  CHECK(signatures[2] == (std::vector<Value::Type>{
        Value::Type::INTEGER_32, Value::Type::STRING, Value::Type::BOOLEAN}));

  Request::Parameters params;
  CHECK(method(params).AsBoolean());

  params.emplace_back("foobar");
  CHECK(method(params).AsString() == "foobar");
}

TEST_CASE("dispatcher")
{
  Dispatcher dispatcher;
  Test test;
  Test2 test2;

  GIVEN("function")
  {
    dispatcher.AddMethod("test", &TestMethod);
  }
  GIVEN("function with regular types")
  {
    dispatcher.AddMethod("test", &TestMethodBool);
  }
  GIVEN("function with value type")
  {
    dispatcher.AddMethod("test", &TestMethodValue);
  }

  GIVEN("lambda")
  {
    dispatcher.AddMethod("test",
                         [](const Request::Parameters& params)
                         {
                           return Value(params[0]);
                         });
  }
  GIVEN("lambda with regular types")
  {
    dispatcher.AddMethod("test", [](bool value) { return value; });
  }
  GIVEN("lambda with value type")
  {
    dispatcher.AddMethod("test",
                         [](const Value& value) { return Value(value); });
  }

  GIVEN("static method")
  {
    dispatcher.AddMethod("test", &Test::StaticMethod);
  }
  GIVEN("static method with regular types")
  {
    dispatcher.AddMethod("test", &Test::StaticMethodBool);
  }
  GIVEN("static method with value type")
  {
    dispatcher.AddMethod("test", &Test::StaticMethodValue);
  }

  GIVEN("method")
  {
    dispatcher.AddMethod("test", &Test::Method, test);
  }
  GIVEN("const method")
  {
    dispatcher.AddMethod("test", &Test::MethodConst, test);
  }
  GIVEN("method with regular types")
  {
    dispatcher.AddMethod("test", &Test::MethodBool, test);
  }
  GIVEN("const method with regular types")
  {
    dispatcher.AddMethod("test", &Test::MethodBoolConst, test);
  }
  GIVEN("method with value type")
  {
    dispatcher.AddMethod("test", &Test::MethodValue, test);
  }
  GIVEN("const method with value type")
  {
    dispatcher.AddMethod("test", &Test::MethodValueConst, test);
  }

  GIVEN("functor")
  {
    dispatcher.AddMethod("test", test);
  }
  GIVEN("function with value type")
  {
    dispatcher.AddMethod("test", test2);
  }

  {
    auto response = dispatcher.Invoke("test", {true});
    CAPTURE(response.GetResult());
    CHECK_FALSE(response.IsFault());
    CHECK(response.GetResult().AsBoolean());
  }

  dispatcher.RemoveMethod("test");

  {
    auto response = dispatcher.Invoke("test", {true});
    CHECK(response.IsFault());
  }
}

TEST_CASE("dispatcher returning void")
{
  Dispatcher dispatcher;

  int value = 0;
  dispatcher.AddMethod(
    "test",
    [&] (int32_t a, const Value& b)
    {
      value = a + b.AsInteger32();
    });

  auto response = dispatcher.Invoke("test", {123, 321});
  CAPTURE(response.GetResult());
  CHECK_FALSE(response.IsFault());
  CHECK(response.GetResult().IsNil());
  CHECK(value == 444);
}
