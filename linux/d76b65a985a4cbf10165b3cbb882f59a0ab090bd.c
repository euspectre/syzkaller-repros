// general protection fault in netlink_unicast
// https://syzkaller.appspot.com/bug?id=d76b65a985a4cbf10165b3cbb882f59a0ab090bd
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000040;
  *(uint64_t*)0x20000040 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x24\x00\x00\x00\x10\x00\x07\x03\x1d\xff\x22\x94\x6f\xa2\x83\x00\x5f"
         "\x20\x0a\x00\x09\x00\x03\x00\x00\x1d\x85\x68\x7f\x00\x00\x00\x04\x00"
         "\xff\x7e\x28\x00\x00\x00\x00\x0a\x43\xba\x5d\x80\x60\x55\xb6\xfd\xd8"
         "\x0b\x40\x00\x00\x00\x06\x00\x01\x00\x00\x29\xec\x24\x00\x02\x0c\xd3"
         "\x7e\x99\xd6\x9c\xda\x45\xa9\x5f",
         76);
  *(uint64_t*)0x20000048 = 0x4c;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000, 0);
  return 0;
}
