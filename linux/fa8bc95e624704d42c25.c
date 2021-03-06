// possible deadlock in proc_pid_syscall
// https://syzkaller.appspot.com/bug?id=fa8bc95e624704d42c25
// status:0
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x200003c0, "./file1\000", 8);
  syscall(__NR_mkdir, 0x200003c0ul, 0ul);
  memcpy((void*)0x200000c0, "./bus\000", 6);
  syscall(__NR_mkdir, 0x200000c0ul, 0ul);
  memcpy((void*)0x20000040, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000040ul, 0ul);
  memcpy((void*)0x20000280, "./bus/file0\000", 12);
  syscall(__NR_mkdir, 0x20000280ul, 0ul);
  memcpy((void*)0x20000000, "./bus\000", 6);
  memcpy((void*)0x20000100, "overlay\000", 8);
  memcpy((void*)0x20000140,
         "upperdir=./file1,lowerdir=./bus,workdir=./file0,index=on", 56);
  syscall(__NR_mount, 0x400002ul, 0x20000000ul, 0x20000100ul, 0ul,
          0x20000140ul);
  memcpy((void*)0x20000080, "./bus/file0\000", 12);
  syscall(__NR_execve, 0x20000080ul, 0ul, 0ul);
  memcpy((void*)0x20000540, "./bus/file0\000", 12);
  syscall(__NR_chdir, 0x20000540ul);
  memcpy((void*)0x20000180, "./bus\000", 6);
  res = syscall(__NR_open, 0x20000180ul, 0x151042ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "/proc/consoles\000", 15);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_sendfile, r[0], r[1], 0ul, 0x4000000000010046ul);
  *(uint64_t*)0x20004500 = 0;
  *(uint32_t*)0x20004508 = 0;
  *(uint64_t*)0x20004510 = 0x20000880;
  *(uint64_t*)0x20000880 = 0;
  *(uint64_t*)0x20000888 = 0;
  *(uint64_t*)0x20004518 = 1;
  *(uint64_t*)0x20004520 = 0;
  *(uint64_t*)0x20004528 = 0;
  *(uint32_t*)0x20004530 = 0;
  *(uint32_t*)0x20004538 = 0;
  syscall(__NR_recvmmsg, -1, 0x20004500ul, 1ul, 0ul, 0ul);
  memcpy((void*)0x200000c0, "syscall\000", 8);
  res = -1;
  res = syz_open_procfs(-1, 0x200000c0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_preadv, r[2], 0x200017c0ul, 0x375ul, 0, 0);
  return 0;
}
