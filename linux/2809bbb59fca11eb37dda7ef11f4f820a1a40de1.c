// BUG: unable to handle kernel paging request in dummy_set_vf_vlan
// https://syzkaller.appspot.com/bug?id=2809bbb59fca11eb37dda7ef11f4f820a1a40de1
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "dummy0\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x20000050 = 0;
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000040);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000050;
  *(uint64_t*)0x2000a000 = 0;
  *(uint32_t*)0x2000a008 = 0;
  *(uint64_t*)0x2000a010 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000280;
  memcpy((void*)0x20000280, "\x38\x00\x00\x00\x13\x00\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000294 = r[1];
  memcpy((void*)0x20000298,
         "\xff\xff\xff\xfa\x00\x00\x00\x00\x18\x00\x16\x00\x14\x00\x01\x00\x10"
         "\x00\x02\x00\x00\x00\x00\xae\xfd\x02\x00\x1b\x00\x21\x15\x96\x05\xfd"
         "\x22\x15\x80\x15\xf7\x60\x4f\xdf\xa7\x96\x14\x45\x42\x56\xc6\x9c\xf2"
         "\xfb\x3b\x34\x80\x39\x10\x2e\xd8\xb9\xf1\xf4\xe1\x7d\x61\xad\x0f\xae"
         "\x6b\xb0\x47\x85\xf1\xef\x5c\x8c\xa4\xd9\xd2\x17\xf5\x0d\x03\x61\x98"
         "\x52\x54\xee\x4e\x98\xaf\x7d\xcf\x72\x63\xf1\x0e\x74\x25\x8a\x7f\xa5"
         "\xbb\x0b\x67\xed\x13\xe6\x2f\x9b\x0e\xf7\x6e\xb5\xa5\x08\xf2\x70\xba"
         "\x0b\x0b\x6f\xec\x11\x45\x74\x34\x79\x44\x25\xeb\x37\xe1\x97\x57\x2c"
         "\xd1\xe3\x3a\xb7\x0e\xfe\x3d\x13\xf6\xb2\xc8\xe7\x09\x5d\x2d\x8a\x8d"
         "\xe9\x7a\x0d\x8e\x10\x92\x59\xe6\x5c\x06\x00\x3e\xcc\x10\xc1\xad\x23"
         "\xc0\x2a\x38\x00\x00\x00\x00\x00\x00\x00\x00\x00\x68\xd8\xc8\xc3\x20"
         "\x62\xc2\x79\x6f\xf3\xf2\xa6\x65\x9a\x51\xaa\xad\xe6\xca\x70\x3e\x50"
         "\xcf\x76\xbc\x54\x4e\x80\xa2\xf9\x24\xa6\x5e\xff\x5e\x87\x69\x74\x34"
         "\x5b\x6d\x7d\x35\x50\x97\x63\x5a\x42\xb4\x17\x84\x23\xb5\x65\xec\x84"
         "\xf2\x2e\xf9\xca\x36\x15\x91\x97\xc3\x68\xe2\xf3\x15\x34\xbf\x73\x6f"
         "\x14\x0d\x28\x8c\x3e\x07\xd3\xc2\x77\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         276);
  *(uint64_t*)0x20000088 = 0x38;
  *(uint64_t*)0x2000a018 = 1;
  *(uint64_t*)0x2000a020 = 0;
  *(uint64_t*)0x2000a028 = 0;
  *(uint32_t*)0x2000a030 = 0;
  syscall(__NR_sendmsg, r[0], 0x2000a000, 0);
  return 0;
}