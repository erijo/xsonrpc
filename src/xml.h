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

#ifndef XSONRPC_XML_H
#define XSONRPC_XML_H

namespace xsonrpc {
namespace xml {

const char METHOD_CALL_TAG[] = "methodCall";
const char METHOD_NAME_TAG[] = "methodName";
const char METHOD_RESPONSE_TAG[] = "methodResponse";
const char FAULT_TAG[] = "fault";
const char PARAMS_TAG[] = "params";
const char PARAM_TAG[] = "param";

const char FAULT_CODE_NAME[] = "faultCode";
const char FAULT_STRING_NAME[] = "faultString";

const char VALUE_TAG[] = "value";

const char ARRAY_TAG[] = "array";
const char BASE_64_TAG[] = "base64";
const char BOOLEAN_TAG[] = "boolean";
const char DATE_TIME_TAG[] = "dateTime.iso8601";
const char DOUBLE_TAG[] = "double";
const char INTEGER_32_TAG[] = "i4";
const char INTEGER_64_TAG[] = "i8";
const char INTEGER_INT_TAG[] = "int";
const char NIL_TAG[] = "nil";
const char STRING_TAG[] = "string";
const char STRUCT_TAG[] = "struct";

const char DATA_TAG[] = "data";
const char MEMBER_TAG[] = "member";
const char NAME_TAG[] = "name";

} // namespace xml
} // namespace xsonrpc

#endif
