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

#include "util.h"

#include <cstring>
#include <tinyxml2.h>

namespace xsonrpc {
namespace util {

bool IsTag(const tinyxml2::XMLElement& element, const char* tag)
{
  return strcmp(element.Name(), tag) == 0;
}

bool HasEmptyText(const tinyxml2::XMLElement& element)
{
  auto text = element.GetText();
  return !text || text[0] == '\0';
}

} // namespace util
} // namespace xsonrpc
