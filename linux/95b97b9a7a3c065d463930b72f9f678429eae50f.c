// lost connection to test machine
// https://syzkaller.appspot.com/bug?id=95b97b9a7a3c065d463930b72f9f678429eae50f
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
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

long r[15];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0x2000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20001000, "\x2f\x64\x65\x76\x2f\x73\x67\x23\x00", 9);
  r[2] = syz_open_dev(0x20001000ul, 0x0ul, 0x2ul);
  *(uint64_t*)0x20001fdc = (uint64_t)0x0;
  *(uint64_t*)0x20001fe4 = (uint64_t)0x0;
  *(uint16_t*)0x20001fec = (uint16_t)0x3;
  *(uint16_t*)0x20001fee = (uint16_t)0x4;
  *(uint32_t*)0x20001ff0 = (uint32_t)0x9;
  *(uint64_t*)0x20001ff4 = (uint64_t)0x0;
  *(uint64_t*)0x20001ffc = (uint64_t)0x2710;
  *(uint16_t*)0x20002004 = (uint16_t)0x7;
  *(uint16_t*)0x20002006 = (uint16_t)0x7;
  *(uint32_t*)0x20002008 = (uint32_t)0x8;
  r[13] = syscall(__NR_write, r[2], 0x20001fdcul, 0x30ul);
  r[14] = syscall(__NR_read, r[2], 0x20000000ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
