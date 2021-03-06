// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000100, "/dev/video#", 12);
  res = syz_open_dev(0x20000100, 0x3f, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 1;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0x20000000;
  *(uint32_t*)0x20000000 = 0x980914;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x2000000c = 0x20000080;
  *(uint8_t*)0x20000080 = 0;
  syscall(__NR_ioctl, r[0], 0xc0205648, 0x20000040);
  memcpy((void*)0x20000280, "/dev/video#", 12);
  res = syz_open_dev(0x20000280, 3, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_read, r[1], 0x200002c0, 0xba);
  return 0;
}
