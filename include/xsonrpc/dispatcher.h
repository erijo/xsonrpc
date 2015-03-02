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

#ifndef XSONRPC_DISPATCHER_H
#define XSONRPC_DISPATCHER_H

#include "request.h"
#include "response.h"
#include "value.h"

#include <functional>
#include <utility>
#include <vector>

namespace xsonrpc {

class MethodWrapper
{
public:
  typedef std::function<Value(const Request::Parameters&)> Method;

  explicit MethodWrapper(Method method) : myMethod(method) {}

  MethodWrapper(const MethodWrapper&) = delete;
  MethodWrapper& operator=(const MethodWrapper&) = delete;

  void SetHelpText(std::string help) { myHelpText = std::move(help); }
  const std::string& GetHelpText() const { return myHelpText; }
  bool HasHelpText() const { return !myHelpText.empty(); }

  template<typename... ParameterTypes>
  void AddSignature(Value::Type returnType,
                    ParameterTypes... parameterTypes)
  {
    mySignatures.emplace_back(
        std::initializer_list<Value::Type>{returnType, parameterTypes...});
  }

  const std::vector<std::vector<Value::Type>>&
  GetSignatures() const { return mySignatures; }

  Value operator()(const Request::Parameters& params) const
  {
    return myMethod(params);
  }

private:
  Method myMethod;
  std::string myHelpText;
  std::vector<std::vector<Value::Type>> mySignatures;
};

class Dispatcher
{
public:
  MethodWrapper& AddMethod(
      std::string name, MethodWrapper::Method method);
  void RemoveMethod(const std::string& name);

  Response Invoke(const std::string& name,
                  const Request::Parameters& parameters) const;

private:
  std::map<std::string, MethodWrapper> myMethods;
};

} // namespace xsonrpc

#endif
