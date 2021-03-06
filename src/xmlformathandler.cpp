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

#include "xmlformathandler.h"

#include "xmlreader.h"
#include "xmlwriter.h"

namespace {

const char TEXT_XML[] = "text/xml";

} // namespace

namespace xsonrpc {

XmlFormatHandler::XmlFormatHandler(std::string requestPath)
  : myRequestPath(std::move(requestPath))
{
}

bool XmlFormatHandler::CanHandleRequest(
  const std::string& path, const std::string& contentType)
{
  return path == myRequestPath && contentType == TEXT_XML;
}

std::string XmlFormatHandler::GetContentType()
{
  return TEXT_XML;
}

bool XmlFormatHandler::UsesId()
{
  return false;
}

std::unique_ptr<Reader> XmlFormatHandler::CreateReader(std::string data)
{
  return std::unique_ptr<Reader>(new XmlReader(data.data(), data.size()));
}

std::unique_ptr<Writer> XmlFormatHandler::CreateWriter()
{
  return std::unique_ptr<Writer>(new XmlWriter());
}

} // namespace xsonrpc
