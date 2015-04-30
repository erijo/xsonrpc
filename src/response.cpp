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
#include "util.h"

#include <tinyxml2.h>

namespace {

const char METHOD_RESPONSE_TAG[] = "methodResponse";
const char PARAMS_TAG[] = "params";
const char PARAM_TAG[] = "param";
const char FAULT_TAG[] = "fault";

const char FAULT_CODE_NAME[] = "faultCode";
const char FAULT_STRING_NAME[] = "faultString";

} // namespace

namespace xsonrpc {

Response::Response(const tinyxml2::XMLElement* root)
{
  if (!root || !util::IsTag(*root, METHOD_RESPONSE_TAG)) {
    throw InvalidXmlRpcFault("missing method response element");
  }

  auto value = root->FirstChildElement(PARAMS_TAG);
  if (value) {
    myIsFault = false;
    value = value->FirstChildElement(PARAM_TAG);
  }
  else {
    myIsFault = true;
    value = root->FirstChildElement(FAULT_TAG);
  }

  if (!value) {
    throw InvalidXmlRpcFault("missing param or fault element");
  }

  myResult = Value(value->FirstChildElement());

  if (myIsFault) {
    if (!myResult.IsStruct()) {
      throw InvalidXmlRpcFault("fault is not a struct");
    }
    auto& s = myResult.AsStruct();
    if (s.find(FAULT_CODE_NAME) == s.end()
        || !s.find(FAULT_CODE_NAME)->second.IsInteger32()) {
      throw InvalidXmlRpcFault("missing or invalid fault code");
    }
    if (s.find(FAULT_STRING_NAME) == s.end()
        || !s.find(FAULT_STRING_NAME)->second.IsString()) {
      throw InvalidXmlRpcFault("missing or invalid fault string");
    }
  }
}

Response::Response(Value value)
  : myResult(std::move(value)),
    myIsFault(false)
{
}

Response::Response(const Fault& fault)
  : myIsFault(true)
{
  Value::Struct data;
  data[FAULT_CODE_NAME] = fault.GetCode();
  data[FAULT_STRING_NAME] = fault.GetString();
  myResult = std::move(data);
}

void Response::Print(tinyxml2::XMLPrinter& printer) const
{
  printer.PushHeader(false, true);
  printer.OpenElement(METHOD_RESPONSE_TAG);

  if (!myIsFault) {
    printer.OpenElement(PARAMS_TAG);
  }
  printer.OpenElement(myIsFault ? FAULT_TAG : PARAM_TAG);
  myResult.Print(printer);
  printer.CloseElement();
  if (!myIsFault) {
    printer.CloseElement();
  }

  printer.CloseElement();
}

void Response::ThrowIfFault() const
{
  if (!IsFault()) {
    return;
  }

  //TODO: auto faultCode = myResult[FAULT_CODE_NAME];
  auto& faultString = myResult[FAULT_STRING_NAME];
  throw Fault(faultString.AsString());
}

} // namespace xsonrpc
