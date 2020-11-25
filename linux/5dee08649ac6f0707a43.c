// KMSAN: uninit-value in __crc32c_le_base (2)
// https://syzkaller.appspot.com/bug?id=5dee08649ac6f0707a43
// status:0 arch:386
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

#ifndef __NR_accept4
#define __NR_accept4 364
#endif
#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_sendfile
#define __NR_sendfile 187
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "/proc/sys/net/ipv4/vs/sync_ports\000", 33);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000040, 2, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000480 = 0x26;
  memcpy((void*)0x20000482, "hash\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000490 = 0;
  *(uint32_t*)0x20000494 = 0;
  memcpy((void*)0x20000498, "crc32c-"
                            "generic\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000",
         64);
  syscall(__NR_bind, (intptr_t)r[1], 0x20000480, 0x58);
  res = syscall(__NR_accept4, (intptr_t)r[1], 0, 0, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendfile, (intptr_t)r[2], (intptr_t)r[0], 0, -1);
  return 0;
}