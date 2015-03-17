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

#ifndef XSONRPC_FAULT_H
#define XSONRPC_FAULT_H

#include "value.h"

#include <cassert>
#include <exception>

namespace xsonrpc {

class Fault : public std::exception
{
public:
  const int32_t RESERVED_CODE_MIN = -32768;
  const int32_t RESERVED_CODE_MAX = -32000;

  Fault(std::string faultString, int32_t faultCode = 0)
    : myFaultString(std::move(faultString)),
      myFaultCode(faultCode)
  {
    // Applications are not allowed to set these fault codes
    assert(myFaultCode < RESERVED_CODE_MIN || myFaultCode > RESERVED_CODE_MAX);
    if (myFaultCode >= RESERVED_CODE_MIN && myFaultCode <= RESERVED_CODE_MAX) {
      myFaultCode = 0;
    }
  }

  int32_t GetCode() const { return myFaultCode; }
  const std::string& GetString() const { return myFaultString; }

  const char* what() const noexcept override
  {
    return myFaultString.c_str();
  }

  operator Value() const;

private:
  Fault(int32_t faultCode, std::string faultString)
    : myFaultString(std::move(faultString)),
      myFaultCode(faultCode)
  {}

  std::string myFaultString;
  int32_t myFaultCode;

  friend class NotWellFormedFault;
  friend class InvalidCharacterFault;
  friend class InvalidXmlRpcFault;
  friend class MethodNotFoundFault;
  friend class InvalidParametersFault;
  friend class InternalFault;
};

class NotWellFormedFault : public Fault
{
public:
  static const int32_t CODE;
  NotWellFormedFault()
    : Fault(CODE, "XML document not well formed") {}
};

class InvalidCharacterFault : public Fault
{
public:
  static const int32_t CODE;
  InvalidCharacterFault()
    : Fault(CODE, "Invalid character for encoding") {}
};

class InvalidXmlRpcFault : public Fault
{
public:
  static const int32_t CODE;
  InvalidXmlRpcFault(const std::string& error)
    : Fault(CODE, "Invalid XML-RPC: " + error) {}
};

class MethodNotFoundFault : public Fault
{
public:
  static const int32_t CODE;
  MethodNotFoundFault(const std::string& methodName)
    : Fault(CODE, "No such method " + methodName) {}
};

class InvalidParametersFault : public Fault
{
public:
  static const int32_t CODE;
  InvalidParametersFault()
    : Fault(CODE, "Invalid parameters") {}
};

class InternalFault : public Fault
{
public:
  static const int32_t CODE;
  InternalFault(std::string error)
    : Fault(CODE, std::move(error)) {}
};

} // namespace xsonrpc

#endif
