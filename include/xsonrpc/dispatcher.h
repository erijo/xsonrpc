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

#if __cplusplus <= 201103L
#include "integer_seq.h"
namespace std {
using redi::index_sequence;
using redi::index_sequence_for;
} // namespace std
#endif

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

  MethodWrapper& SetHelpText(std::string help);
  const std::string& GetHelpText() const { return myHelpText; }
  bool HasHelpText() const { return !myHelpText.empty(); }

  template<typename... ParameterTypes>
  MethodWrapper& AddSignature(Value::Type returnType,
                              ParameterTypes... parameterTypes)
  {
    mySignatures.emplace_back(
        std::initializer_list<Value::Type>{returnType, parameterTypes...});
    return *this;
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
  Dispatcher();
  
  MethodWrapper& AddMethod(
      std::string name, MethodWrapper::Method method);

  template<typename ReturnType, typename... ParameterTypes>
  typename std::enable_if<!std::is_same<ReturnType, Value>::value,
                          MethodWrapper>::type&
  AddMethod(std::string name,
            std::function<ReturnType(ParameterTypes...)> method)
  {
    return AddMethodInternal(
        std::move(name),
        std::move(method),
        std::index_sequence_for<ParameterTypes...>{});
  }

  template<typename ReturnType, typename... ParameterTypes>
  typename std::enable_if<!std::is_same<ReturnType, Value>::value,
                          MethodWrapper>::type&
  AddMethod(std::string name,
            ReturnType (*function)(ParameterTypes...))
  {
    return AddMethodInternal(
        std::move(name),
        std::function<ReturnType(ParameterTypes...)>(function),
        std::index_sequence_for<ParameterTypes...>{});
  }

  void RemoveMethod(const std::string& name);

  Response Invoke(const std::string& name,
                  const Request::Parameters& parameters) const;

private:
  template<typename ReturnType, typename... ParameterTypes,
           std::size_t... index>
  MethodWrapper& AddMethodInternal(
      std::string name, std::function<ReturnType(ParameterTypes...)> method,
      std::index_sequence<index...>)
  {
    return AddMethod(
        std::move(name),
        [method] (const Request::Parameters& params) -> Value
        {
          if (params.size() != sizeof...(ParameterTypes)) {
            throw InvalidParametersFault();
          }
          return method(
              params[index].AsType<
              typename std::remove_cv<
              typename std::remove_reference<ParameterTypes>::type
              >::type
              >()...);
        });
  }

  Value SystemMulticall(const Request::Parameters& parameters) const;

  std::map<std::string, MethodWrapper> myMethods;
};

} // namespace xsonrpc

#endif
