// kernel panic: Out of memory and no killable processes...
// https://syzkaller.appspot.com/bug?id=94100cff4d9a032b284ae1f06f9a8baaddf0a419
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
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
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 2, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000,
         "\x6d\x61\x6e\xa4\x66\xa9\x82\x9c\x40\x05\x08\x7e\x0b\x31\x21\xdc\xe7"
         "\x02\x00\xaf\xd6\x1a\x1e\x5b\x45\xa3\xcd\x21\x00\x04\xe1\x62\xa1\xfb"
         "\xd0\x19\x00\x00\x00\x00\x88\x80\xc6\x86\x8a\xc5\x8d\xe1\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x09\x00\xd2\x72\x4c\x2e\xbb\x63\x0f\x27\x16"
         "\xc2\x65\x4b\x00\x3d\x00\x00\x56\x93\x46\x52\x48\x8e\xdd\xce\x75\x0a"
         "\x38\x00",
         87);
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20000000, 1);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
