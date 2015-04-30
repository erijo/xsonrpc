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

#include <cstring>
#include <iostream>
#include <limits>

int main(int argc, char** argv)
{
  xsonrpc::Client::GlobalInit();

  xsonrpc::Client client("localhost", 8080);

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

  if (argc >= 2 && strcmp(argv[1], "-e") == 0) {
    std::cout << "exit: " << client.Call("exit") << "\n";
  }

  return 0;
}
