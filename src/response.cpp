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
#include "writer.h"

namespace xsonrpc {

Response::Response(Value value)
  : myResult(std::move(value)),
    myIsFault(false)
{
}

Response::Response(int32_t faultCode, std::string faultString)
  : myIsFault(true),
    myFaultCode(faultCode),
    myFaultString(std::move(faultString))
{
}

void Response::Write(Writer& writer) const
{
  writer.StartDocument();
  if (myIsFault) {
    writer.StartFaultResponse();
    writer.WriteFault(myFaultCode, myFaultString);
    writer.EndFaultResponse();
  }
  else {
    writer.StartResponse();
    myResult.Write(writer);
    writer.EndResponse();
  }
  writer.EndDocument();
}

void Response::ThrowIfFault() const
{
  if (!IsFault()) {
    return;
  }

  // TODO: use myFaultCode
  throw Fault(myFaultString);
}

} // namespace xsonrpc
