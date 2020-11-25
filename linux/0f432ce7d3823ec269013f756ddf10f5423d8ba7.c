// INFO: rcu detected stall in do_iter_write
// https://syzkaller.appspot.com/bug?id=0f432ce7d3823ec269013f756ddf10f5423d8ba7
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

#ifndef __NR_sched_setattr
#define __NR_sched_setattr 314
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint64_t*)0x20000280 = 9;
  *(uint64_t*)0x20000288 = 0x81;
  syscall(__NR_prlimit64, 0, 0xeul, 0x20000280ul, 0ul);
  *(uint32_t*)0x20000040 = 0x38;
  *(uint32_t*)0x20000044 = 2;
  *(uint64_t*)0x20000048 = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 3;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  syscall(__NR_sched_setattr, 0, 0x20000040ul, 0ul);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 3;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000ul, 0, -1ul, -1, 0ul);
  memcpy((void*)0x20000700, "./bus\000", 6);
  res = syscall(__NR_creat, 0x20000700ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200006c0 = 0;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0;
  *(uint64_t*)0x200006e0 = 0;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0;
  *(uint64_t*)0x20000708 = 0;
  *(uint64_t*)0x20000710 = 0;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0;
  *(uint64_t*)0x20000728 = 0;
  *(uint64_t*)0x20000730 = 0x20000740;
  memcpy((void*)0x20000740,
         "\xaa\x47\x2e\x1f\xf8\xce\xa9\xf3\x22\x01\xc6\xdf\x7a\x20\x1e\x43\xf4"
         "\x01\x8f\xab\x00\xc7\x82\x77\x29\x0b\xaa\xd4\x58\x21\x1e\x53\x74\x56"
         "\xb6\x1e\xf7\xb2\x90\x1a\x08\x09\x2d\xa2\x8d\xf9\x1c\xdd\x6b\xc6\x6b"
         "\x29\xde\xfb\x46\x90\x89\x6c\xdd\xfb\x0c\xdd\x5c\x06\x5d\xdb\x7d\x29"
         "\xb2\x1e\xf1\xa6\x0e\x6b\x37\x1d\x34\x13\x00\xf3\xde\x60\xdd\x15\xcb"
         "\x14\xce\x3c\x03\xe5\x65\x1f\xd7\x50\x33\x02\x7e\xe1\x8b\xff\xcd\x21"
         "\x73\x51\xa8\x78\xda\x61\xa0\x83\x9c\x8e\x7e\x4c\xb8\x25\x29\xe4\x55"
         "\x38\x30\x92\xaf\x6c\xda\x1f\x25\x34\xd1\xa7\x49\x7c\xd3\xc0\x98\xd0"
         "\x5f\x8e\x39\xd0\x63\x43\x27\x4c\x2e\xef\x27\xb0\xc8\xa9\xac\xc6\x5c"
         "\x84\x76\x0b\x90\xe5\xa9\x02\x37\x94\x43\x96\xf6\x11\xec\xa0\x11\xdf"
         "\x51\x91\x3d\xcc\x47\x34\x02\xc2\x65\xde\x5e\x40\x8c\x21\xf5\x53\x36"
         "\xfc\x10\xb9\xd1\x91\xfc\x3c\x14\x6c\x9c\x34\x74\xcb\x99\xd4\x04\xce"
         "\xbc\x35\x27\x88\x4a\xf0\x0d\x10\x08\xf2",
         214);
  *(uint64_t*)0x20000738 = 0xd6;
  syscall(__NR_writev, r[0], 0x200006c0ul, 8ul);
  memcpy((void*)0x20000240, "./bus\000", 6);
  res = syscall(__NR_open, 0x20000240ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_sendfile, r[0], r[1], 0ul, 0x8000fffffffeul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}