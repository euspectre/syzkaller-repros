// general protection fault in sg_read
// https://syzkaller.appspot.com/bug?id=4462682cd32aee8ff03a43c8b9be3963743bc506
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  memcpy((void*)0x200001c0, "/dev/sg#", 9);
  res = syz_open_dev(0x200001c0, 0, 0x40000000006);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000080, "system_u:object_r:tzdata_exec_t:s0", 34);
  *(uint8_t*)0x200000a2 = 0x20;
  memcpy((void*)0x200000a3, "system_u:object_r:tun_tap_device_t:s0", 37);
  *(uint8_t*)0x200000c8 = 0x20;
  *(uint8_t*)0x200000c9 = 0x38;
  *(uint8_t*)0x200000ca = 0x33;
  *(uint8_t*)0x200000cb = 0x20;
  memcpy((void*)0x200000cc, "/sbin/dhclient", 15);
  syscall(__NR_write, r[0], 0x20000080, 0x5b);
  *(uint64_t*)0x2085dff0 = 0x20e94000;
  *(uint64_t*)0x2085dff8 = 0x10024;
  syscall(__NR_readv, r[0], 0x2085dff0, 0x146);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
