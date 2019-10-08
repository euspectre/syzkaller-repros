// BUG: Double free or freeing an invalid pointer (2)
// https://syzkaller.appspot.com/bug?id=f1335c716ec95468c88e883887111c463609b0a3
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

long r[17];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x202a0000,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[2] = syz_open_dev(0x202a0000ul, 0x0ul, 0x44042ul);
  *(uint32_t*)0x20f74000 = (uint32_t)0x0;
  *(uint32_t*)0x20f74004 = (uint32_t)0x0;
  *(uint32_t*)0x20f74008 = (uint32_t)0x0;
  *(uint32_t*)0x20f7400c = (uint32_t)0x0;
  *(uint32_t*)0x20f74010 = (uint32_t)0x3;
  *(uint32_t*)0x20f74014 = (uint32_t)0x40000006;
  *(uint32_t*)0x20f74018 = (uint32_t)0x0;
  *(uint32_t*)0x20f7401c = (uint32_t)0x4;
  memcpy((void*)0x20f74020,
         "\xc3\x5b\xa8\x84\x01\x00\x00\x00\x6a\xf6\x4a\xc3\xf0\xf9\x92"
         "\xb9\xab\xe7\x47\x4a\x36\x96\x34\xce\x3d\x60\xbb\x89\xbb\x5c"
         "\xce\xf7\x4b\xf6\xbd\x58\xce\x42\x70\x26\x8f\xab\xed\xc1\x0f"
         "\x3f\x35\x83\x93\x6a\xe9\xdc\xaf\xa0\x2a\x8b\xca\xef\x1e\xcf"
         "\xd8\x98\xfb\x57",
         64);
  memcpy((void*)0x20f74060, "\x4e\xcc\xf1\xbc\x48\xf2\x07\xed\x10\x94"
                            "\xce\x01\x61\xa4\xbb\x7b\xca\xf5\xa1\xb9"
                            "\xb3\x2d\x93\x6c\x1d\x02\xff\xff\x95\x28"
                            "\xc3\x76",
         32);
  *(uint64_t*)0x20f74080 = (uint64_t)0x7;
  *(uint64_t*)0x20f74088 = (uint64_t)0x400;
  *(uint32_t*)0x20f74090 = (uint32_t)0x0;
  r[16] = syscall(__NR_ioctl, r[2], 0xc0481273ul, 0x20f74000ul);
}

int main()
{
  loop();
  return 0;
}