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

#include "../src/util.h"

#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  if (argc != 2) {
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    return 1;
  }

  const size_t size = 28671;
  std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);

  ssize_t res;
  while ((res = read(fd, buffer.get(), size)) > 0) {
    std::cout << xsonrpc::util::Base64Encode(buffer.get(), res) << "\r\n";
  }

  close(fd);
  return 0;
}
