// WARNING in __alloc_pages_slowpath (3)
// https://syzkaller.appspot.com/bug?id=9e5e7e64d89f46177201bc533e4e30c76eff67c8
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  memcpy((void*)0x207c27c4, "/dev/keychord", 14);
  res = syscall(__NR_openat, 0xffffff9c, 0x207c27c4, 0x201, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0x100000001;
  syscall(__NR_write, r[0], 0x20000040, 0xffffff4e);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}