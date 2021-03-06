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

#include "../src/util.h"

#include <catch.hpp>

using namespace xsonrpc::util;

TEST_CASE("encode base64")
{
  CHECK(Base64Encode("") == "");
  CHECK(Base64Encode("f") == "Zg==");
  CHECK(Base64Encode("fo") == "Zm8=");
  CHECK(Base64Encode("foo") == "Zm9v");
  CHECK(Base64Encode("foob") == "Zm9vYg==");
  CHECK(Base64Encode("fooba") == "Zm9vYmE=");
  CHECK(Base64Encode("foobar") == "Zm9vYmFy");

  CHECK(Base64Encode(
          "this is a longer string that will make "
          "the result longer than 76 chars") ==
        "dGhpcyBpcyBhIGxvbmdlciBzdHJpbmcgdGhhdCB"
        "3aWxsIG1ha2UgdGhlIHJlc3VsdCBsb25nZXIg\r\n"
        "dGhhbiA3NiBjaGFycw==");
  CHECK(Base64Encode(
          "this gives one line and nothing more to "
          "see how it works!") ==
        "dGhpcyBnaXZlcyBvbmUgbGluZSBhbmQgbm90aGl"
        "uZyBtb3JlIHRvIHNlZSBob3cgaXQgd29ya3Mh");
}

TEST_CASE("decode base64")
{
  CHECK(Base64Decode("") == "");
  CHECK(Base64Decode("Zg==") == "f");
  CHECK(Base64Decode("Zm8=") == "fo");
  CHECK(Base64Decode("Zm9v") == "foo");
  CHECK(Base64Decode("Zm9vYg==") == "foob");
  CHECK(Base64Decode("Zm9vYmE=") == "fooba");
  CHECK(Base64Decode("Zm9vYmFy") == "foobar");

  CHECK(Base64Decode(
          "dGhpcyBpcyBhIGxvbmdlciBzdHJpbmcgdGhhdCB"
          "3aWxsIG1ha2UgdGhlIHJlc3VsdCBsb25nZXIg\r\n"
          "dGhhbiA3NiBjaGFycw==") ==
        "this is a longer string that will make "
        "the result longer than 76 chars");
}
