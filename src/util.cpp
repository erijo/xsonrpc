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

namespace {

const char BASE_64_ALPHABET[64 + 1] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int8_t BASE_64_LUT[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

inline char Base64Char0(uint8_t byte0)
{
  return BASE_64_ALPHABET[byte0 >> 2];
}
inline char Base64Char1(uint8_t byte0, uint8_t byte1)
{
  return BASE_64_ALPHABET[((byte0 << 4) | (byte1 >> 4)) & 0x3f];
}
inline char Base64Char2(uint8_t byte1, uint8_t byte2)
{
  return BASE_64_ALPHABET[((byte1 << 2) | (byte2 >> 6)) & 0x3f];
}
inline char Base64Char3(uint8_t byte2)
{
  return BASE_64_ALPHABET[byte2 & 0x3f];
}

} // namespace

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

std::string Base64Encode(const uint8_t* data, size_t size)
{
  const size_t lineLength = 76;
  static_assert(lineLength % 4 == 0, "invalid line length");

  const size_t encodedSize = 4 * ((size + 2) / 3);
  std::string str(encodedSize + 2 * (encodedSize / lineLength), '=');

  size_t in = 0;
  size_t out = 0;
  for (size_t wrap = lineLength; in + 3 <= size; in += 3, wrap -= 4) {
    if (wrap == 0) {
      str[out++] = '\r';
      str[out++] = '\n';
      wrap = lineLength;
    }
    str[out++] = Base64Char0(data[in]);
    str[out++] = Base64Char1(data[in], data[in + 1]);
    str[out++] = Base64Char2(data[in + 1], data[in + 2]);
    str[out++] = Base64Char3(data[in + 2]);
  }

  if (in < size) {
    str[out++] = Base64Char0(data[in]);
    if (in + 1 < size) {
      str[out++] = Base64Char1(data[in], data[in + 1]);
      str[out++] = Base64Char2(data[in + 1], 0);
      out += 1;
    }
    else {
      str[out++] = Base64Char1(data[in], 0);
      out += 2;
    }
  }
  str.resize(out);
  return str;
}

std::vector<uint8_t> Base64Decode(const char* str, size_t size)
{
  const size_t maxDecodedSize = 3 * ((size + 3) / 4);

  std::vector<uint8_t> data(maxDecodedSize);

  size_t out = 0;
  uint32_t bits = 0;
  size_t bitCount = 0;

  for (size_t in = 0; in < size; ++in) {
    const int value = BASE_64_LUT[static_cast<uint8_t>(str[in])];
    if (value != -1) {
      bits = (bits << 6) | value;
      bitCount += 6;
      if (bitCount == 24) {
        data[out++] = bits >> 16;
        data[out++] = bits >> 8;
        data[out++] = bits;

        bits = 0;
        bitCount = 0;
      }
    }
  }

  if (bitCount >= 12) {
    bits = bits >> (bitCount % 8);
    if (bitCount == 18) {
      data[out++] = bits >> 8;
    }
    data[out++] = bits;
  }

  data.resize(out);
  return data;
}

} // namespace util
} // namespace xsonrpc
