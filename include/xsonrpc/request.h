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

#ifndef XSONRPC_REQUEST_H
#define XSONRPC_REQUEST_H

#include "value.h"

#include <string>
#include <vector>

namespace tinyxml2 {
class XMLElement;
class XMLPrinter;
}

namespace xsonrpc {

class Request
{
public:
  typedef std::vector<Value> Parameters;

  explicit Request(const tinyxml2::XMLElement* root);

  const std::string& GetMethodName() const { return myMethodName; }
  const Parameters& GetParameters() const { return myParameters; }

  void Print(tinyxml2::XMLPrinter& printer) const;
  static void Print(const std::string& methodName, const Parameters& params,
                    tinyxml2::XMLPrinter& printer);

private:
  std::string myMethodName;
  Parameters myParameters;
};

} // namespace xsonrpc

#endif
