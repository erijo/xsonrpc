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

#include "fault.h"
#include "request.h"
#include "util.h"
#include "writer.h"

#include <tinyxml2.h>

namespace {

const char METHOD_CALL_TAG[] = "methodCall";
const char METHOD_NAME_TAG[] = "methodName";
const char PARAMS_TAG[] = "params";
const char PARAM_TAG[] = "param";

} // namespace

namespace xsonrpc {

Request::Request(const tinyxml2::XMLElement* root)
{
  if (!root || !util::IsTag(*root, METHOD_CALL_TAG)) {
    throw InvalidXmlRpcFault("missing method call element");
  }

  auto name = root->FirstChildElement(METHOD_NAME_TAG);
  if (!name || util::HasEmptyText(*name)) {
    throw InvalidXmlRpcFault("missing method name");
  }
  myMethodName = name->GetText();

  auto params = root->FirstChildElement(PARAMS_TAG);
  if (!params) {
    return;
  }

  for (auto param = params->FirstChildElement(PARAM_TAG);
       param; param = param->NextSiblingElement(PARAM_TAG)) {
    myParameters.emplace_back(param->FirstChildElement());
  }
}

void Request::Write(Writer& writer) const
{
  Write(myMethodName, myParameters, writer);
}

void Request::Write(const std::string& methodName, const Parameters& params,
                    Writer& writer)
{
  writer.StartDocument();
  writer.StartRequest(methodName);
  for (auto& param : params) {
    writer.StartParameter();
    param.Write(writer);
    writer.EndParameter();
  }
  writer.EndRequest();
  writer.EndDocument();
}

} // namespace xsonrpc
