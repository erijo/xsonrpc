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

#include "client.h"

#include "fault.h"
#include "response.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <memory>
#include <stdexcept>

namespace {

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string* buffer = static_cast<std::string*>(userdata);
  buffer->append(ptr, size * nmemb);
  return size * nmemb;
}

} // namespace

namespace xsonrpc {

void Client::GlobalInit()
{
  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    throw std::runtime_error("client: failed to initialize cURL");
  }
}

Client::Client(const std::string& host, unsigned short port,
               const std::string& uri)
  : myHandle(curl_easy_init())
{
  if (!myHandle) {
    throw std::runtime_error("client: failed to initialize cURL handle");
  }
  curl_easy_setopt(myHandle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(myHandle, CURLOPT_MAXREDIRS, 20L);

  std::string url = "http://" + host + ":" + std::to_string(port) + uri;
  curl_easy_setopt(myHandle, CURLOPT_URL, url.c_str());

  curl_easy_setopt(myHandle, CURLOPT_USERAGENT, "xsonrpc/" XSONRPC_VERSION);
  curl_easy_setopt(myHandle, CURLOPT_WRITEFUNCTION, &WriteCallback);
}

Client::~Client()
{
  curl_easy_cleanup(myHandle);
}

Value Client::CallInternal(const std::string& methodName,
                           const Request::Parameters& params)
{
  XmlWriter writer;
  Request::Write(methodName, params, writer);

  curl_easy_setopt(myHandle, CURLOPT_POSTFIELDSIZE_LARGE,
                   static_cast<curl_off_t>(writer.GetSize()));
  curl_easy_setopt(myHandle, CURLOPT_POSTFIELDS, writer.GetData());

  std::unique_ptr<curl_slist, void(*)(curl_slist*)> headers(
    curl_slist_append(NULL, "Content-Type: text/xml"), &curl_slist_free_all);
  curl_easy_setopt(myHandle, CURLOPT_HTTPHEADER, headers.get());

  std::string buffer;
  curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, static_cast<void*>(&buffer));
  auto res = curl_easy_perform(myHandle);

  if (res != CURLE_OK) {
    throw std::runtime_error("client: failed to perform cURL call");
  }

  long responseCode;
  if (curl_easy_getinfo(myHandle, CURLINFO_RESPONSE_CODE, &responseCode)
      != CURLE_OK || responseCode != 200) {
    throw std::runtime_error("client: HTTP request failed");
  }

  XmlReader reader(buffer.data(), buffer.size());
  Response response = reader.GetResponse();
  response.ThrowIfFault();
  return std::move(response.GetResult());
}

} // namespace xsonrpc
