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

#include "xsonrpc/client.h"
#include "xsonrpc/xmlformathandler.h"

#include <cstring>
#include <iostream>
#include <limits>

int main(int argc, char** argv)
{
  xsonrpc::Client::GlobalInit();

  xsonrpc::XmlFormatHandler xmlFormatHandler;
  xsonrpc::Client client("localhost", 8080, xmlFormatHandler);

  std::cout << "add: 3+2=" << client.Call("add", 3, 2) << "\n";
  std::cout << "concat: " << client.Call("concat", "Hello, ", "World!")
            << "\n";

  xsonrpc::Request::Parameters params;
  {
    xsonrpc::Value::Array a;
    a.emplace_back(1000);
    a.emplace_back(std::numeric_limits<int32_t>::max());
    params.push_back(std::move(a));
  }
  std::cout << "add_array: " << client.Call("add_array", params) << "\n";

  std::cout << "to_binary: " << client.Call("to_binary", "Hello World!")
            << "\n";
  std::cout << "from_binary: "
            << client.Call("from_binary",
                           xsonrpc::Value::Binary{'H', 'i', '!'})
            << "\n";

  params.clear();
  {
    xsonrpc::Value::Array a;
    a.emplace_back(12);
    a.emplace_back("foobar");
    a.emplace_back(a);
    params.push_back(std::move(a));
  }
  std::cout << "to_struct: " << client.Call("to_struct", params) << "\n";

  params.clear();
  {
    xsonrpc::Value::Array calls;
    {
      xsonrpc::Value::Struct call;
      call["methodName"] = "add";
      {
        xsonrpc::Value::Array params;
        params.emplace_back(23);
        params.emplace_back(19);
        call["params"] = std::move(params);
      }
      calls.emplace_back(std::move(call));
    }
    {
      xsonrpc::Value::Struct call;
      call["methodName"] = "does.NotExist";
      calls.emplace_back(std::move(call));
    }
    {
      xsonrpc::Value::Struct call;
      call["methodName"] = "concat";
      {
        xsonrpc::Value::Array params;
        params.emplace_back("Hello ");
        params.emplace_back("multicall!");
        call["params"] = std::move(params);
      }
      calls.emplace_back(std::move(call));
    }
    params.emplace_back(std::move(calls));
  }
  std::cout << "multicall: " << client.Call("system.multicall", params)
            << "\n";
  std::cout << "methods: " << client.Call("system.listMethods") << "\n";
  std::cout << "help(add): " << client.Call("system.methodHelp", "add")
            << "\n";
  std::cout << "params(add): "
            << client.Call("system.methodSignature", "add") << "\n";

  if (argc >= 2 && strcmp(argv[1], "-e") == 0) {
    std::cout << "exit: " << client.Call("exit") << "\n";
  }

  return 0;
}
