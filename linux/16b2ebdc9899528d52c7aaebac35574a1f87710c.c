// BUG: unable to handle kernel NULL pointer dereference in tc_bind_tclass
// https://syzkaller.appspot.com/bug?id=16b2ebdc9899528d52c7aaebac35574a1f87710c
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>

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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[2], 0x200001c0, 0);
  *(uint32_t*)0x20000200 = 0xa;
  res = syscall(__NR_getsockname, r[2], 0x20000100, 0x20000200);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000540;
  *(uint32_t*)0x20000540 = 0x148;
  *(uint16_t*)0x20000544 = 0x24;
  *(uint16_t*)0x20000546 = 0x507;
  *(uint32_t*)0x20000548 = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint8_t*)0x20000550 = 0;
  *(uint32_t*)0x20000554 = r[3];
  *(uint16_t*)0x20000558 = 0;
  *(uint16_t*)0x2000055a = 0;
  *(uint16_t*)0x2000055c = -1;
  *(uint16_t*)0x2000055e = -1;
  *(uint16_t*)0x20000560 = 0;
  *(uint16_t*)0x20000562 = 0;
  *(uint16_t*)0x20000564 = 8;
  *(uint16_t*)0x20000566 = 1;
  memcpy((void*)0x20000568, "red\000", 4);
  *(uint16_t*)0x2000056c = 0x11c;
  *(uint16_t*)0x2000056e = 2;
  *(uint16_t*)0x20000570 = 0x104;
  *(uint16_t*)0x20000572 = 2;
  memcpy((void*)0x20000574,
         "\x7c\xba\xf4\xa5\x8f\x3a\x09\xe8\x5c\x09\x77\x3c\x86\x54\x9f\x79\x33"
         "\x96\x7d\x31\x70\xab\xd4\x7c\x81\xcc\x22\x12\xa8\xde\xa1\x8c\xa1\xf8"
         "\x13\xbe\x47\x68\x02\xb1\xf3\x8c\x18\x2a\x6e\x7e\xde\x33\x05\x54\x64"
         "\xcc\xa6\xfa\x40\x8a\xef\x9e\x0f\x31\xb4\xe9\xa7\x70\x82\x70\x2e\x46"
         "\xbc\xd4\xa0\x18\x7f\x6f\x08\x6f\x7e\xe0\x2a\xaf\x1c\x88\x35\x56\xe0"
         "\x1c\xde\xba\xf6\x66\x9a\x47\x05\xf8\x03\x92\x58\x5e\x6a\xa8\x45\xd0"
         "\xc0\xba\x11\xe4\xf6\xef\x81\x09\xb4\xd9\x35\x9e\x98\x34\x88\x1f\x99"
         "\xe6\xf3\x86\xeb\xcd\x23\xc0\xf4\x70\x40\x72\xb9\x7a\x67\x9c\xdd\x8d"
         "\x62\x1f\x36\xd7\x28\x2d\xf7\x6c\x0f\x03\xbe\x45\x7d\x49\xca\xe4\xe0"
         "\x23\x96\x95\x2b\xe7\x6c\xfc\x2a\xda\x7a\x67\x5c\x82\x03\x65\x5a\x55"
         "\xc5\xbe\x68\x47\xbe\x85\x03\xfe\x70\x1d\xae\x45\xd6\x0e\x62\x36\xf3"
         "\x24\xee\x47\x16\x18\xbc\x9d\x07\xf9\xcd\x3f\x47\x6e\x70\xaa\xb7\x8d"
         "\x1f\x64\x32\x1b\x29\x11\xff\x2d\xcb\x7f\x1a\x9e\x40\x9e\x0b\x11\x50"
         "\x5f\xe8\x4c\xe7\x8e\x17\xee\x88\xd0\x3f\x2e\xf7\x7d\xd1\x34\xf1\x42"
         "\x99\x70\x3e\xf6\x37\xc5\xba\x6b\x33\x63\xd8\x60\x42\x87\x83\x4c\x4e"
         "\x9a",
         256);
  *(uint16_t*)0x20000674 = 0x14;
  *(uint16_t*)0x20000676 = 1;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint8_t*)0x20000684 = 0;
  *(uint8_t*)0x20000685 = 0;
  *(uint8_t*)0x20000686 = 0;
  *(uint8_t*)0x20000687 = 0;
  *(uint64_t*)0x20000088 = 0x148;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000240, 0);
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0xfffffffb;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000300;
  memcpy((void*)0x20000300, "\x24\x00\x00\x00\x29\x00\x01\x00\xef\x00\x59\x12"
                            "\x65\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000314 = r[3];
  memcpy((void*)0x20000318, "\xf4\xff\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00",
         12);
  *(uint64_t*)0x20000088 = 0xb5;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}