// KASAN: use-after-free Read in kvm_vm_ioctl_unregister_coalesced_mmio
// https://syzkaller.appspot.com/bug?id=77c67c409dad0ba32099
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffff9cul, 0x20000000ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20001080 = 0;
  *(uint32_t*)0x20001084 = 0;
  *(uint64_t*)0x20001088 = 0;
  *(uint64_t*)0x20001090 = 0;
  *(uint64_t*)0x20001098 = 0;
  *(uint64_t*)0x200010a0 = 0;
  *(uint8_t*)0x200010a8 = 0;
  *(uint8_t*)0x200010a9 = 0;
  *(uint8_t*)0x200010aa = 0;
  *(uint8_t*)0x200010ab = 0;
  *(uint8_t*)0x200010ac = 0;
  *(uint8_t*)0x200010ad = 0;
  *(uint8_t*)0x200010ae = 0;
  *(uint8_t*)0x200010af = 0;
  *(uint8_t*)0x200010b0 = 0;
  *(uint8_t*)0x200010b1 = 0;
  *(uint8_t*)0x200010b2 = 0;
  *(uint8_t*)0x200010b3 = 0;
  *(uint8_t*)0x200010b4 = 0;
  *(uint8_t*)0x200010b5 = 0;
  *(uint8_t*)0x200010b6 = 0;
  *(uint8_t*)0x200010b7 = 0;
  *(uint8_t*)0x200010b8 = 0;
  *(uint8_t*)0x200010b9 = 0;
  *(uint8_t*)0x200010ba = 0;
  *(uint8_t*)0x200010bb = 0;
  *(uint8_t*)0x200010bc = 0;
  *(uint8_t*)0x200010bd = 0;
  *(uint8_t*)0x200010be = 0;
  *(uint8_t*)0x200010bf = 0;
  *(uint8_t*)0x200010c0 = 0;
  *(uint8_t*)0x200010c1 = 0;
  *(uint8_t*)0x200010c2 = 0;
  *(uint8_t*)0x200010c3 = 0;
  *(uint8_t*)0x200010c4 = 0;
  *(uint8_t*)0x200010c5 = 0;
  *(uint8_t*)0x200010c6 = 0;
  *(uint8_t*)0x200010c7 = 0;
  *(uint8_t*)0x200010c8 = 0;
  *(uint8_t*)0x200010c9 = 0;
  *(uint8_t*)0x200010ca = 0;
  *(uint8_t*)0x200010cb = 0;
  *(uint8_t*)0x200010cc = 0;
  *(uint8_t*)0x200010cd = 0;
  *(uint8_t*)0x200010ce = 0;
  *(uint8_t*)0x200010cf = 0;
  *(uint8_t*)0x200010d0 = 0;
  *(uint8_t*)0x200010d1 = 0;
  *(uint8_t*)0x200010d2 = 0;
  *(uint8_t*)0x200010d3 = 0;
  *(uint8_t*)0x200010d4 = 0;
  *(uint8_t*)0x200010d5 = 0;
  *(uint8_t*)0x200010d6 = 0;
  *(uint8_t*)0x200010d7 = 0;
  *(uint8_t*)0x200010d8 = 0;
  *(uint8_t*)0x200010d9 = 0;
  *(uint8_t*)0x200010da = 0;
  *(uint8_t*)0x200010db = 0;
  *(uint8_t*)0x200010dc = 0;
  *(uint8_t*)0x200010dd = 0;
  *(uint8_t*)0x200010de = 0;
  *(uint8_t*)0x200010df = 0;
  *(uint8_t*)0x200010e0 = 0;
  *(uint8_t*)0x200010e1 = 0;
  *(uint8_t*)0x200010e2 = 0;
  *(uint8_t*)0x200010e3 = 0;
  *(uint8_t*)0x200010e4 = 0;
  *(uint8_t*)0x200010e5 = 0;
  *(uint8_t*)0x200010e6 = 0;
  *(uint8_t*)0x200010e7 = 0;
  syscall(__NR_ioctl, r[1], 0x4010ae67, 0x20001080ul);
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  syscall(__NR_ioctl, r[1], 0x4010ae67, 0x20000140ul);
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  inject_fault(0);
  syscall(__NR_ioctl, r[1], 0x4010ae68, 0x20000180ul);
  return 0;
}