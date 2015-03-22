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

#include "client.h"

#include <iostream>
#include <limits>

int main()
{
  xsonrpc::Client::GlobalInit();

  xsonrpc::Client client("localhost", 8080);

  std::cout << "3+2=" << client.Call("add", 3, 2) << "\n";
  std::cout << client.Call("concat", "Hello, ", "World!") << "\n";

  xsonrpc::Request::Parameters params;
  {
    xsonrpc::Value::Array a;
    a.emplace_back(1000);
    a.emplace_back(std::numeric_limits<int32_t>::max());
    params.push_back(std::move(a));
  }
  std::cout << "add_array: " << client.Call("add_array", params) << "\n";

  params.clear();
  {
    xsonrpc::Value::Array a;
    a.emplace_back(12);
    a.emplace_back("foobar");
    a.emplace_back(a);
    params.push_back(std::move(a));
  }
  std::cout << "to_struct: " << client.Call("to_struct", params) << "\n";

  return 0;
}
