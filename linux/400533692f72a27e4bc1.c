// BUG: unable to handle kernel paging request in do_syscall_64
// https://syzkaller.appspot.com/bug?id=400533692f72a27e4bc1
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
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
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

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

#define MAX_FDS 30

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
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

static int wait_for_loop(int pid)
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

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 7; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  close_fds();
  if (!collide) {
    collide = 1;
    goto again;
  }
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syz_open_dev(0xc, 4, 1);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000000 = 0;
    *(uint16_t*)0x20000002 = 0;
    *(uint16_t*)0x20000004 = 0;
    *(uint16_t*)0x20000006 = 0;
    *(uint16_t*)0x20000008 = 4;
    *(uint16_t*)0x2000000a = 4;
    syscall(__NR_ioctl, r[0], 0x560a, 0x20000000ul);
    break;
  case 2:
    memcpy((void*)0x20000180, "/dev/fb0\000", 9);
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000180ul, 0ul, 0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0x18b;
    *(uint32_t*)0x2000000c = 0x360;
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = 0;
    *(uint32_t*)0x20000018 = 4;
    *(uint32_t*)0x2000001c = 0;
    *(uint32_t*)0x20000020 = 0;
    *(uint32_t*)0x20000024 = 0;
    *(uint32_t*)0x20000028 = 0;
    *(uint32_t*)0x2000002c = 0;
    *(uint32_t*)0x20000030 = 0;
    *(uint32_t*)0x20000034 = 0;
    *(uint32_t*)0x20000038 = 0;
    *(uint32_t*)0x2000003c = 0;
    *(uint32_t*)0x20000040 = 0;
    *(uint32_t*)0x20000044 = 0;
    *(uint32_t*)0x20000048 = 0;
    *(uint32_t*)0x2000004c = 0;
    *(uint32_t*)0x20000050 = 5;
    *(uint32_t*)0x20000054 = 0;
    *(uint32_t*)0x20000058 = 0;
    *(uint32_t*)0x2000005c = 0;
    *(uint32_t*)0x20000060 = 0;
    *(uint32_t*)0x20000064 = 0;
    *(uint32_t*)0x20000068 = 0;
    *(uint32_t*)0x2000006c = 0;
    *(uint32_t*)0x20000070 = 0;
    *(uint32_t*)0x20000074 = 0;
    *(uint32_t*)0x20000078 = 0;
    *(uint32_t*)0x2000007c = 0;
    *(uint32_t*)0x20000080 = 0;
    *(uint32_t*)0x20000084 = 0;
    *(uint32_t*)0x20000088 = 0;
    *(uint32_t*)0x2000008c = 0;
    *(uint32_t*)0x20000090 = 0;
    *(uint32_t*)0x20000094 = 0;
    *(uint32_t*)0x20000098 = 0;
    *(uint32_t*)0x2000009c = 0;
    syscall(__NR_ioctl, r[1], 0x4601, 0x20000000ul);
    break;
  case 4:
    res = syz_open_dev(0xc, 4, 1);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    res = syscall(__NR_dup, r[2]);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    *(uint32_t*)0x20000000 = 8;
    memcpy(
        (void*)0x20000004,
        "\x11\x77\x9b\x3f\xaf\x89\x5c\x10\x37\xa4\x07\x3c\xc2\x6a\xa1\xab\x8c"
        "\xc0\x0f\x07\x0f\xdc\xb1\x70\x86\x5d\x56\xe0\x9f\xd0\x78\xd0\xb8\x16"
        "\x80\xc0\xa6\xf4\xfe\x0d\xd9\xa8\x9e\x9b\x5e\x87\xaf\x36\x90\x01\x87"
        "\x23\x99\x4c\x47\xeb\x83\x09\x56\xb0\x05\xb2\x0a\x66\x0e\x49\x8a\x36"
        "\x81\xe3\x7e\x7c\x81\x94\xe5\xec\x02\x34\x41\xfd\xd9\x63\x14\xa8\x78"
        "\xad\x46\x08\xf1\x9c\x0a\xce\xc1\x45\xd3\x32\x20\x9d\xce\x27\xd4\x6a"
        "\x53\xfe\x2d\x0b\x39\xbc\x3a\x4c\x93\x10\x0d\x54\xe6\x63\x63\x9b\x38"
        "\x13\xd8\x6f\x17\x8c\x70\x01\x3b\xee\xe0\xbc\x31\xaa\x07\x61\xbe\xa5"
        "\x2e\x10\x8a\x90\xac\x8a\x5c\x07\x32\x1c\x98\xa9\x61\xd6\x20\x9c\xea"
        "\x61\x0e\x37\x77\x4f\x5e\xc2\x0f\xd3\x33\x7d\x6b\xb0\x14\xaa\xc6\x97"
        "\x14\x82\x75\x36\xb8\x6f\x82\x31\x51\x3b\x40\xde\x92\x56\x8e\xa8\x54"
        "\x72\xbb\xca\x1e\xb1\xd0\x33\xdd\x12\x23\xa3\xc2\xe2\x6d\xc6\x29\xbf"
        "\x8f\x3b\x2d\xbb\x68\xe7\x8a\x84\x6d\x75\x59\x69\xdd\xdf\x7b\x85\x35"
        "\x3f\x3f\x0a\xb6\x6b\x1b\xd6\xd3\x3d\x04\x46\xe4\xd3\xbd\xb6\xda\x07"
        "\xa6\x88\x16\x02\x29\x55\xe4\x15\x9d\x17\x2b\x54\xd5\x79\x59\xcd\x99"
        "\x3f\xcf\x30\x9a\x2d\x36\xa1\xea\x97\x08\x13\x78\x63\x84\x54\x5f\x36"
        "\x17\xd4\x46\x10\x5a\x82\x64\x9a\xa3\xe4\xd6\xc0\x7f\x86\x2a\x3c\x8b"
        "\xe9\x33\xc9\x82\x6c\x13\xf4\xf5\xea\x4a\x38\x16\x39\xc3\x37\x65\xe5"
        "\xe4\x74\x4e\x89\x4d\x2f\xec\x10\x14\xc0\x91\x6d\xa5\xa6\x8b\x6e\x14"
        "\x66\x59\xb5\x4d\xc9\xe6\x12\xbe\x27\x6b\xe6\xb4\x1f\x19\xed\x42\x1e"
        "\xeb\x09\x4e\x9d\x1b\x8b\x24\x80\x42\xef\x0e\x01\x5a\x8d\x64\x36\x60"
        "\x9c\x67\xb4\xd4\xde\xe7\x6d\xba\x8a\x39\x48\xef\xa6\x6a\x6a\xf6\xe3"
        "\x1b\x63\x96\x25\xc3\x65\xc5\x25\x18\x39\x16\x89\x2b\x3c\x5f\x11\x1c"
        "\x9b\x02\x25\x26\x29\x66\x25\xad\x67\x6a\xf8\xed\xfb\xd2\x40\xa2\xc1"
        "\xbc\x97\x76\x58\x7c\x66\x00\xd5\xfb\xcc\x59\x94\x6a\xfa\xa7\x15\xc3"
        "\x98\xac\x4f\xcf\xc5\x8e\xe0\x26\x14\x0a\x30\xe8\xf8\x37\x03\xe9\xde"
        "\x5d\xec\x75\xae\x78\x45\xcf\x9c\xfe\x56\xb6\xbd\xd5\x15\xd7\x5b\xfc"
        "\xd9\xda\x36\xf3\x43\x39\x6d\x48\x79\x3e\xd1\x89\xff\x09\x6b\x48\xd7"
        "\x66\xd4\x84\xe5\xca\x5a\xd8\xa7\x67\xc4\x8c\xe1\xeb\x64\x28\x63\x38"
        "\x21\x3c\x30\xde\xe1\x27\x31\xf4\x47\x07\x42\x7f\x1b\xe3\xbb\x1b\x92"
        "\xaf\x3a\x48\xca\xdb\x03\x57\xda\x62\x52\x97\x0c\xa5\x50\x41\x84\xed"
        "\x53\xbf\x11\x43\x31\xc2\x53\x4d\xf9\xf8\xb6\xd0\xb5\xd7\x0d\xde\x1a"
        "\x52\xff\x77\xa9\x76\x03\x72\x25\x1e\xd2\x0e\x57\xe7\xaa\xc3\x67\x1d"
        "\xaa\xa7\x2b\x42\xfc\x2c\x6a\x5d\x9c\x3a\x2b\xa3\x12\x4d\x45\xea\xf6"
        "\x11\xd9\x8b\x6c\x2a\x06\xc6\xb2\x4e\x91\xa5\x1c\x4d\x2d\xf3\xc9\x94"
        "\xba\x8f\xd8\xf6\xae\xe1\x6d\x73\x7d\x17\xc0\x4d\xc6\x96\x09\x2d\xf2"
        "\xa7\x47\x19\x85\x42\x22\xc7\xe6\xeb\xe4\x9b\x9a\xc2\x69\x94\x73\x12"
        "\x11\x0c\xed\x11\x2b\xbf\x87\x3d\x33\x5b\xf2\x5e\x6f\x66\x10\x5e\xf4"
        "\xf0\xf4\x0c\x35\xc1\x21\xec\x74\x5b\xa9\x3b\x9d\xad\xc4\x89\xa7\xfd"
        "\xa0\xeb\x59\xd6\x2c\xdc\x71\x8c\xbb\xb1\xf4\x2f\x6e\xde\x50\xbd\x24"
        "\x55\xa2\x83\x78\x8d\xbd\x64\xda\x39\x10\x75\xd2\x41\xb1\x03\x99\xb2"
        "\xce\x67\x6d\x95\xa2\xf8\xaf\x7d\xf5\x3c\xde\x9b\x3c\xf6\x65\x0e\x23"
        "\x9a\x98\x39\x09\xbf\x86\x9a\xb5\x00\x62\xf2\xef\xd1\xae\x84\xef\xe7"
        "\x6c\xda\x15\x25\xd4\xff\x6a\x17\x50\xdb\xfc\x77\xb7\x10\x93\xfb\xf4"
        "\x61\xc3\xdc\xb6\x01\x63\x03\x3a\x7e\x87\x60\x76\x43\xfa\x9b\xa0\xb2"
        "\x96\xe3\x4c\x04\x11\x5f\x93\xe2\x09\xf1\xcd\xda\x93\x9e\x58\xf2\xd4"
        "\x01\x32\x6b\x99\x06\x1c\xe1\xe9\x30\xf0\xfa\xeb\x7e\x60\x1b\x5f\x36"
        "\xbd\x87\x9d\x24\x66\xd4\x84\x35\xbb\xc0\x22\x75\x60\xf0\x04\xc9\x47"
        "\x02\xdc\x84\xe4\x1f\xcf\x6d\x50\x04\x9a\x58\x44\x97\x27\xb5\x79\xd2"
        "\x38\x81\x7b\x68\x46\x42\x11\x3d\x1a\x53\x15\x33\x49\x9b\x63\x47\x13"
        "\x4c\xf6\xee\x29\x83\x55\x5a\x9c\x2c\xa7\x5f\x3e\x5b\x1d\x5d\x8e\xf7"
        "\xcb\x5b\x7a\x6f\xff\xc2\x5a\xd0\xf8\xd1\x77\xf6\x65\x9a\xdc\x1a\x87"
        "\x74\x0b\xe2\xbc\x69\x18\xe3\x47\x6d\x15\x20\x26\x83\xf5\x32\x16\xe0"
        "\x25\x06\x02\xd7\x5c\x64\xd5\xc1\x00\x6b\xc0\xd7\x43\xe3\xb4\x9a\xab"
        "\x24\xdc\x05\x20\xf5\x52\x8e\x92\x1b\xe7\x17\x4f\x03\x06\x79\xd7\x71"
        "\xe3\xb2\xaa\x80\x42\x13\x91\x90\xa5\xa9\xf3\x13\x1a\x78\x2c\xee\xd8"
        "\xb0\x7a\x23\xc5\x67\x9b\xbf\xbe\x67\x9d\x46\xd5\x6e\xa8\x65\xd7\x77"
        "\x99\x3b\x21\xf8\xce\xeb\xe7\x55\xe8\x5f\x33\xc1\xb6\x9f\x8d\x8b\xd1"
        "\x89\x5f\xbd\x52\x42\xd5\x0f\xc5\xf2\x5f\x74\x0e\x8a\xd3\xd4\xab\x19"
        "\x4e\x92\x87\x7c\x90\xcd\xec\x66\xe9\x90\x7e\xbd\x33\x4e\x86\xec\x77"
        "\x5b\x62\x98\xd8\x82\x5b\xf7\x6e\x34\x26\x9c\x23\x0e\xef\x49\x6a\x66"
        "\x41\x76\x0a\x60\x0e\x41\x47\x8b\x1a\x0c\xdb\xab\x37\x80\x13\x07\x6a"
        "\x1a\xd1\xce\x6a\xf0\xbc\xd9\xb6\xfe\x08\x4b\xf7\xc4\x59\x97\x0e\xa3"
        "\x6f\x2a\xd0\x17\x62\x38\x5e\x83\x3f\x27\x08\xf9\xdb\x34\x74\xe9\x94"
        "\xa1\xdb\x83\xeb\xa9\xab\xb1\x58\xb4\x2d\xfd\x90\x65\xd4\x58\x93\x5b"
        "\xfa\x6a\x75\xc2\x70\x89\xc2\x2c\x21\xd6\x6e\xa8\xe2\x83\xe3\x2e\x23"
        "\x16\xa3\xd0\x51\x7b\x1b\x61\xff\xed\x6d\x2c\x4c\x92\x1b\x2f\xa1\xbf"
        "\xb9\x1f\xa9\x74\x68\xb5\x92\x0f\xa7\x05\xb0\x92\x49\x65\x11\x86\x90"
        "\x8b\xe7\x31\x00\x8f\x77\x70\xca\xf2\x30\x7c\x3e\xd0\x2b\x20\x75\x12"
        "\x0d\x8f\x64\x9e\x12\x87\x07\xe8\x0e\x46\x55\xb7\xe9\x1c\x6f\x20\xc4"
        "\xb1\xeb\x8f\x53\xb2\x9a\x6f\xdc\xc0\x0f\xb0\xab\x0a\x93\xde\x70\x85"
        "\x53\x51\x26\x27\x98\x02\x61\x57\x26\xaf\xd6\xe5\xba\x62\x91\xb2\x30"
        "\xfb\x77\x60\x0e\xaa\x02\x35\x81\x28\xfb\x10\x58\xe7\x7b\x95\x77\x03"
        "\x37\x21\x79\x55\x53\xc0\x4a\x57\xbb\xf9\xbd\x28\xb2\xab\xac\xc2\x4c"
        "\xf3\x76\xd4\x19\xfb\x7e\x72\xbd\x89\x7b\x07\x71\xae\x76\x2d\xaa\xd8"
        "\xf5\xc0\x1e\x42\x2d\x84\xfc\x5b\x44\x3a\x4d\xa2\x13\xd6\xd1\xfe\x79"
        "\x35\xbf\x3c\xd1\xe8\xf8\x2e\x39\x20\x37\xbf\x4e\xf1\x76\x5d\xde\x0d"
        "\x5e\xf0\xf4\xff\x9d\xec\xdd\x83\x82\x80\x1a\xff\x46\xd0\xba\x1c\xc6"
        "\xf1\xb4\x86\xcf\xcc\xe2\xac\xab\x65\x83\xf7\x33\x64\x9f\x6e\xbd\xe3"
        "\xa2\xd9\x42\x20\xf2\x5e\xb0\x16\x94\xd2\x22\xfd\x5b\x9f\x11\x2b\xeb"
        "\xf7\xa5\x15\x02\x72\x09\x21\xf4\x52\x4f\xcd\xb0\x27\x49\x20\x5c\xa5"
        "\xba\x8d\x4d\x7e\xb4\x3d\x63\x56\xca\x79\xc8\x42\xa9\x15\xef\x7d\x34"
        "\x3f\xf8\x8d\x01\x41\x76\x07\xe0\x60\x08\xcd\x62\x0a\x1a\xb8\x53\x08"
        "\x94\x3d\x55\x65\x34\x8a\x2a\xf2\x00\xac\xcf\xf7\x95\x25\xe9\xf6\xa5"
        "\x60\xf8\x38\x35\x9a\xe7\xbb\x08\xf2\xa8\xaa\x4e\x77\x51\xa7\x89\xbc"
        "\x55\x1e\xcd\x7e\xf1\x27\xba\x50\x83\x71\x8e\x68\x03\xdf\x0b\x98\xa4"
        "\x19\x0c\x87\xbb\xfb\x12\xd9\xcb\xf9\x1e\x9d\xa8\x77\x66\x0e\x4d\x6f"
        "\x48\xad\x22\x1e\xf9\x10\xf9\xc6\xf1\x03\xc9\x5e\x5a\xc6\x44\x4e\x1b"
        "\x48\xde\xbf\xce\x23\xca\x29\xb9\x76\x5b\x12\xf1\x7d\xed\x7d\x6d\xd3"
        "\x67\x0d\xe2\x7a\xc0\x14\x98\x8d\x3b\x70\x93\x69\x4c\x8f\xb0\xcc\xb7"
        "\x9f\x82\x08\xf0\x00\x12\xcd\x76\xc3\x03\x53\x79\x55\x61\x20\xae\xec"
        "\xeb\x86\x50\xc3\xeb\x21\x97\x17\xd3\x71\x28\x25\x53\x8d\xf9\x97\x66"
        "\x14\xf4\x7e\xa3\x6e\x2f\xcd\x9d\xe1\x85\x0f\x37\xe8\x2d\x98\x56\xad"
        "\xf8\x46\x45\xdc\x5f\xd0\xae\x93\x23\x0f\x0b\xec\x88\xa5\x8d\x0c\x5f"
        "\x9b\x01\x79\x79\x2f\x40\xf1\x37\x7e\x39\x2f\x10\x82\xf6\x26\xeb\x4d"
        "\xb0\x8e\x40\x2c\xb0\x70\x1c\xe3\x64\x1b\xbf\x9a\x58\xbd\x37\xe9\xae"
        "\x46\xef\x13\x4a\x28\x0f\x2d\x30\xe4\x9c\xb9\x34\xf1\x9e\x53\x81\x6a"
        "\xe7\x02\x19\x68\x8a\x90\xe9\x9b\xd7\xde\x80\x32\x5c\x7b\xe4\x59\x6e"
        "\xf1\x19\xae\x8c\x09\x8d\x6e\xcc\x39\xf7\x16\x70\xd1\x34\xea\xd0\xb5"
        "\x40\xf2\xc1\x94\x8c\x3d\x83\x06\x27\x74\x96\x63\x99\x70\xa9\xd4\x9f"
        "\x50\x11\x51\xec\x63\x98\x78\x21\xdc\x4a\x3b\x05\x49\xe1\xc6\x69\xf9"
        "\x45\x18\x4b\xef\x54\x60\xcf\x9d\xd4\x67\xa0\xb1\x7f\xa7\xb0\x5c\x14"
        "\xbf\x7b\x0b\x74\x29\x28\x5e\x6b\xe6\x28\xbf\x5d\x61\x96\x10\x82\xb5"
        "\x25\xda\xbd\xca\x24\xa9\xa2\x42\x2b\x03\x89\x59\xea\x42\xf7\x45\xdd"
        "\x93\xb9\xa9\xb1\xfe\xa0\x08\xf9\xe5\x96\xd0\x25\xfe\x0d\x51\x1c\x7f"
        "\x5b\x6e\xf5\xf0\xc0\x15\xcc\x81\xa9\xab\x90\x57\x1b\x38\x1f\x04\x7f"
        "\x51\x0e\x5e\xc8\xff\xb1\xbe\xd8\x0a\xa2\x73\xd9\xcd\xa7\x88\x48\x46"
        "\xef\x4c\xdd\x77\x72\xc2\x06\x52\x79\x65\x23\x59\x20\xca\xee\x07\x41"
        "\x69\x4a\x75\xa6\x9f\x7f\x87\xb4\x18\x77\xfd\x13\x6e\x0a\xa7\xed\xa9"
        "\x6d\x42\x9d\x00\xd5\x1c\xf8\x61\x3f\xcf\xd2\x58\xe0\xfe\x7a\xa6\x08"
        "\xde\x0e\xbe\x03\xf5\x36\xb2\x9f\xad\xac\xe4\x07\x05\x42\x14\x69\x19"
        "\xd1\xad\xea\x84\x66\xfe\x13\x30\xb9\x36\x1a\x38\xcf\x18\xf9\x08\xd2"
        "\xa6\x74\x28\x34\x81\xe7\x5e\x1f\xa6\x41\x7a\xc7\x92\xe1\x92\x53\x1e"
        "\x57\xc4\x85\x9c\xe8\xa5\x8f\x30\x6b\x91\xfb\x47\x24\x10\x01\xb3\x26"
        "\x50\x9d\xc4\x14\x0f\x01\xc1\xd2\x6b\xa8\xb4\xf9\x12\xab\x76\x0a\xc2"
        "\x14\xf0\x08\x55\x67\xaf\x3a\xde\xa5\x38\x9f\x81\x5a\x3e\x85\xab\x47"
        "\xa9\x71\x06\xa4\x3b\x30\x48\xa0\x6a\xfd\xa6\xe1\xbf\xd6\x2f\x45\x1e"
        "\x37\xa9\x6f\x15\x68\x1c\xa2\x79\xb2\xc3\xe5\xd5\xf1\x77\x34\xb2\x9e"
        "\x8b\xc7\x5c\x6b\x7d\xf6\x8a\x69\x85\x9f\x7b\xc7\xf1\x9e\xec\x75\xb7"
        "\x40\x50\xcd\xbd\x77\xf3\xa1\x37\x68\x14\xfa\x2e\xb9\xe3\x7a\x2d\xdf"
        "\xc5\x4f\xc1\x9b\x59\xa8\xef\x7f\x4b\x8c\x20\x8c\x7b\x1c\xdb\xd0\x0e"
        "\xa7\x16\xbf\x79\x62\x4e\x7d\xcf\x61\xb5\x44\x35\x71\x27\xc0\xdb\x00"
        "\x91\x2b\xda\x57\xe7\x9d\x59\x35\x3e\x18\xf0\xe3\x21\x7a\xc9\xa6\x41"
        "\x56\xe9\x12\xcf\x7a\x3b\x20\x61\x19\xbe\xb4\x3e\xa1\xff\x89\xbc\x2a"
        "\x0c\x92\x04\xeb\x01\xb1\x71\x05\xa2\x95\xe4\xd0\x7b\x7b\xed\x21\xad"
        "\xbe\x06\xd0\xd9\x70\x68\x86\x45\xf0\xef\x9c\x89\xf8\x62\x06\x6a\xa7"
        "\x1a\xab\x85\x81\x06\x48\xb0\x08\x01\xce\x5a\xd1\xf2\x8c\x93\xd7\x54"
        "\x3a\x69\xc4\xbd\xf6\x8f\x24\x66\xbd\xb3\xce\x80\x4d\xc4\x97\x9d\x77"
        "\x01\x73\x12\x09\xb9\x91\x1f\x46\x4c\x3e\x97\x28\xc0\xd8\x26\x6f\xb2"
        "\xef\x06\x58\xa9\xac\xf4\x92\x17\xd6\xf9\x9e\x5b\xe3\x25\x89\x7a\xe2"
        "\x67\x42\xd4\x43\x4d\x3f\x11\x43\xc8\x91\x93\x02\xaa\xa1\xeb\xa2\xbb"
        "\x33\x31\xd4\x6d\x70\x26\x49\x93\x9c\xdd\x22\x4e\xc8\x4a\x52\xcf\xd4"
        "\x56\xd6\xa2\xea\x40\xf5\x64\x39\x3e\xbe\x7e\x11\xc8\xc1\xf0\x97\xcf"
        "\xae\x21\x44\x46\xf4\xe9\x4f\xff\xf4\x96\x1d\x53\xc7\x0b\x26\xc0\xbb"
        "\x2b\xf9\x93\x95\x63\x93\x67\x5c\x9d\x70\x6f\x31\x08\xe4\x5f\x81\x01"
        "\xf4\xc3\x57\xf5\x32\x0d\x56\x3b\xa7\x11\xe8\x9a\x7f\xcb\x34\x66\x90"
        "\x77\x3e\x70\x9e\xe5\x95\xae\x38\x61\x12\xf6\x1a\xd2\x93\x68\xca\x25"
        "\x6e\x76\xb4\x94\xe0\x03\x00\xfc\x77\x11\x11\x58\xf5\xed\x33\x35\x04"
        "\x28\xf0\x97\xc0\x80\xa6\xfa\xba\xcc\x4e\x9b\xa6\x08\xfc\x38\x94\xcb"
        "\xeb\xf9\x2c\x3d\xd3\x04\x94\x0b\xa3\x11\x03\x91\xd1\x5b\xa8\x3a\xbb"
        "\xcb\x15\x8c\xea\x82\xd2\xd6\x6f\x12\xf4\xb2\x63\x2c\xe9\x71\xdd\xe2"
        "\xc2\x8a\x88\x7a\x85\x01\xc0\xa1\x7e\x10\xf4\x1b\xdb\x07\xc1\xcf\x6a"
        "\x58\xd9\x84\x90\x24\x7a\x21\x7d\xaf\x89\x16\xa1\xc0\x3f\x6a\x0a\x4a"
        "\x17\x41\xbc\x3e\x23\x19\xe8\x45\x31\xac\xba\x0f\xe9\x98\x39\x3d\x3c"
        "\xf7\x89\x27\x2a\x80\xb6\xa4\x8d\x51\xf4\x2c\x41\x96\x02\x76\x27\x45"
        "\xcf\xe7\x67\x97\x79\x7e\x4c\x72\x15\x50\x9f\xa7\x39\xf2\xe3\x8c\x6f"
        "\x19\x1a\xe1\x82\xf0\x31\x46\x17\x86\xdd\xfa\xac\xd3\x57\x8c\x70\x09"
        "\xb9\x46\x61\x8e\xa3\x8a\x5f\x19\xac\x96\xe6\xf5\xf4\x3d\x72\x86\x51"
        "\xbb\xb4\xf5\x49\x2c\xbb\xbc\xf7\x1a\xd0\x11\x9d\x68\x8c\x47\x8d\x7f"
        "\x75\x3d\x51\xb0\xd8\xef\x30\x73\x0f\xf6\x4a\x43\xa4\x65\xcf\x3a\xd0"
        "\xbd\xec\xcb\xfe\x3e\x18\x8d\x79\x08\x14\x7a\x32\x46\xf9\xec\xb5\xe8"
        "\xb0\x2f\xa2\xfd\x2e\xd1\x8e\xb1\x7a\x2d\xbd\x2f\x43\xf1\xd1\xd9\xa1"
        "\xc1\xbf\xde\xdb\x9b\x8c\x76\x6d\x0e\x91\x57\x15\x52\x7b\x02\x7d\x33"
        "\x42\x10\xf0\x67\x57\xc6\xd9\xf2\x73\xab\xc3\xaf\xe4\xbc\x49\x9e\xb2"
        "\xfb\xb3\x59\x1c\x60\xfc\x6e\xec\x58\xb9\xc4\x52\xe6\x59\xca\xf5\x0b"
        "\xbf\x8d\x31\x94\x51\xce\x32\x4e\xee\x1d\x3e\xe2\x57\x20\x1d\x44\x94"
        "\x64\xba\x94\xd7\x77\xa3\x5f\x51\x8c\xee\xc4\xc0\x0b\x42\xa9\xea\x40"
        "\x30\x40\x5a\xe9\x2f\xc9\x36\x32\xc9\x3e\xeb\xf4\xad\x07\xe1\x19\xb6"
        "\x07\xb1\x0f\xc2\xa9\x14\x5d\x43\xf8\x79\x30\x58\xf8\xaa\x51\x3a\x85"
        "\x85\x38\xa2\xd5\xfc\xd7\x58\x8a\x44\xba\x68\xe5\x1b\x7b\x9d\x24\x3f"
        "\xee\x68\x8e\xc9\xc0\x65\x87\x20\x09\x3e\x15\xd9\xff\x9c\xd2\x50\x97"
        "\xad\x55\x2c\x58\x86\x15\x08\x45\xd5\x1c\xbb\x46\xc2\xe5\x78\x65\x93"
        "\xb8\xc9\x22\xe3\x51\x4a\x02\x39\xa4\x19\xb5\x4f\xe7\x3d\x1b\x49\x66"
        "\x99\xde\xad\x1f\x41\x72\x0b\xad\xdd\x43\x33\x5e\xf7\x68\x4f\x7c\x40"
        "\xe3\xdb\xfb\x42\x88\xc4\xe4\x7c\xd0\x41\x40\xc4\x8c\x6e\xe4\xf0\x39"
        "\x74\x07\x54\x27\xa9\x76\x4a\x14\x99\x99\x71\x2c\xab\xe6\x39\x13\x87"
        "\x0f\xcf\x30\x94\x90\xd1\x1a\x37\x1b\xf4\xb7\x36\x66\x77\xce\x59\x26"
        "\x0f\xfa\xc3\x3f\xda\x50\x20\x56\xfb\xcd\xfd\x91\x86\x19\x93\xbc\x1a"
        "\x97\x26\xf9\x1d\x99\x1a\x9d\x3e\x2f\xff\x28\x27\xc3\x73\xc0\x48\x72"
        "\x1c\x92\x10\xe4\xfe\x75\x19\xa0\xfd\x99\x6b\x7f\xdb\xdd\x4f\xdf\x01"
        "\x46\x62\x0f\xf7\x8d\xd0\x2e\x76\xd0\xb8\x3a\xc5\x47\xaa\xb0\xbf\x27"
        "\x18\x16\xc8\xd8\xa6\x3a\x68\xb2\x19\x23\x84\xcf\x5a\x96\x91\xbb\x29"
        "\x29\x12\xf2\xf6\xea\x6e\xbd\x31\xe5\xf4\xb8\xea\x2e\x81\xf6\x3d\xcb"
        "\x44\x31\x3c\xd1\xfb\x40\x22\x2b\x1f\xfc\x74\xef\x1d\x9d\x9e\xa2\x99"
        "\x5d\x02\x6b\xdc\x11\x44\xba\x66\x3f\x30\x0d\xfb\x43\xf2\xd6\x8f\x0e"
        "\x8a\x1a\x43\x45\xe6\xff\x69\x2a\xe6\xed\xf5\x24\xb0\x30\x07\x87\xa7"
        "\x11\xc4\xd8\xa1\xbe\x72\xc9\xb3\x0a\x7a\x1c\xf7\x9c\x89\x24\x4c\xd5"
        "\x98\x6e\xf7\xab\x68\x7b\x31\xe4\x4c\x96\xeb\x25\x9f\x9b\x69\x08\x89"
        "\x0d\xda\x46\xd1\x6c\x68\xec\xc3\x78\x0e\x24\xff\x67\x28\xf9\x59\x6d"
        "\x80\x81\x23\xde\xcb\xfc\x81\xf5\x45\x16\x80\x70\x55\x80\x8f\x7a\xfd"
        "\xf7\x88\x68\x4c\x36\xb4\x05\x7a\x7f\x46\x68\x85\x93\x74\xd6\x7b\x69"
        "\xb8\xc0\x79\x6a\x77\x48\x1e\x83\x06\x7c\x26\xe0\x45\xc5\xc8\xbc\x03"
        "\xb9\x55\xe7\xb6\x71\x8a\x7e\xed\x8f\xa7\x65\xc0\xf0\xad\xd8\x5c\xfd"
        "\xb1\x6e\x13\x6f\xd5\xc0\x46\x14\x94\x6e\x53\x4e\x4d\x3a\x0f\xdc\xe8"
        "\xd6\x53\x1e\x14\xee\xab\xb7\x05\xca\x76\x8c\xa8\x9c\x01\xc5\x70\x21"
        "\x61\x4d\x06\x1f\xda\x48\xb8\xbd\x05\xcd\xd4\x3c\x3d\xc8\xa8\x4d\x1d"
        "\x1a\xd4\xdb\x18\x0b\x07\x02\x74\x70\xbd\x6d\x22\x76\x77\x4d\xad\xed"
        "\xf9\xa7\xc7\x43\x06\x64\x6a\xab\xa3\xeb\xfc\x26\x91\x1a\x75\x6c\x56"
        "\xb5\x9f\x7f\x01\x87\x41\x83\x0f\xad\xe7\x72\xeb\x2d\x3f\x71\x5d\x95"
        "\x91\x1d\x24\xb0\x7d\xbf\xf0\x06\xe8\x68\x9b\x27\xeb\xd0\xf6\xbb\x02"
        "\x96\xcb\xee\xc4\x95\xd5\x97\xa0\xc0\x09\x26\x18\xbb\xd1\x23\x6a\xe1"
        "\xdc\xbc\x69\x31\x63\x25\xe2\xc1\x4b\x03\x31\x8a\x88\x6a\xc5\xdf\xfe"
        "\x4a\x70\x16\x64\x8f\x82\x32\xd4\xf6\xb3\x64\x5d\x28\xdc\xe4\x0e\x7c"
        "\x9e\xcf\xa0\xc0\xea\x54\x70\xdf\xeb\x2e\xaf\xa2\x2c\x50\xaa\x2b\xb2"
        "\x9e\x6b\x1d\x3e\x4a\x9d\xc0\xe5\x6e\xca\x48\xaa\x85\x9f\xcb\xce\x56"
        "\x44\xd8\x0c\x67\x99\x33\xe1\x9d\x70\xe2\x3b\xc0\x5e\xbd\x0f\x59\x3a"
        "\x62\x7f\xa8\x72\x8b\x4c\x8e\x05\x16\x52\x96\xff\x33\xe9\xa1\x2e\x02"
        "\x7f\xc0\x06\x4e\x86\x33\x8f\xe0\x69\x7e\xe2\xb9\x99\x0e\xe7\xab\x0d"
        "\x4e\xbc\x8f\x8b\xd0\x27\xda\x65\x2b\xaa\xf1\xcb\x59\xce\x5a\xf7\x9e"
        "\x6f\xbc\x1b\xf1\xb3\x09\x1e\x3d\x7c\xe3\xea\x94\x31\x28\x36\x52\x65"
        "\xac\xba\xd0\xee\x55\x2e\x70\x44\x74\xaf\x67\xc6\x1c\xa3\x56\xec\x19"
        "\x97\xac\x61\x9f\xb9\x4b\x74\x64\xa3\x3f\xe0\x5c\x4b\x0c\xf9\x3d\xdd"
        "\xef\x34\x0c\xf7\x0c\xc2\x3e\x68\xeb\x3e\x17\x94\x32\x70\x3a\x8c\x88"
        "\x3e\x19\xfe\xc3\x10\x03\xf5\xf0\xc5\x36\xa7\x17\xbd\xb8\x14\x92\x06"
        "\x70\x53\xdd\xaf\x00\xb0\x69\x47\x83\x6d\x30\xe4\x07\xbf\x4f\x56\x5b"
        "\x0a\xaa\xbd\x21\x4a\xe8\x3f\xcc\xe3\x37\x48\x9d\x6d\x4d\xdf\xee\x48"
        "\xd2\xc4\x42\xbe\x7d\xcf\x9b\xde\x4b\x70\xc9\x4d\x31\xd1\x99\xfc\x10"
        "\x1b\x44\x6f\x82\x54\xe7\xf5\x3c\x57\xdc\xbe\x5a\xb3\x9a\x5c\xbc\xc9"
        "\x6c\x08\xa2\xfb\x59\x47\x6d\x61\x56\xed\xf1\x30\xa1\x5a\x19\xb5\x0a"
        "\xbd\xc0\xfb\xbf\xbd\x5e\x44\x8d\x7c\x89\xd0\x49\x00\x2f\xbc\x86\x3d"
        "\xf6\x58\xd2\x5d\x6e\x58\x99\x04\xd7\x44\x59\x57\x34\xa0\xe2\x11\xb5"
        "\x55\xae\x03\xf2\x54\x8f\x57\x92\xce\x78\x69\xe3\xd1\x23\x96\xb0\x17"
        "\xad\x2c\x33\xb2\xec\xa7\x44\x2a\x8f\x3b\x9f\x6c\x98\xa1\xe0\x4c\x97"
        "\x7e\xb1\xa5\x52\xe5\x72\x58\x2e\x7b\xd2\x87\x6b\xc7\x73\x55\xb9\x18"
        "\x76\x3a\x80\x08\x99\xf4\x26\x41\x8e\x24\x04\x5f\x4c\xdb\xf1\x39\xc3"
        "\x9b\xf1\xb1\x61\x08\x34\x7a\x74\xbe\x0c\x5d\xe7\x8c\xba\x91\x83\x89"
        "\x19\xb2\x34\x50\x34\xbd\xb3\x04\xb9\x4a\x2a\xd1\x7e\xdd\x1d\xc0\x64"
        "\x62\xb5\x30\x67\x1c\xdf\x2c\xba\x29\x6b\xfb\xe4\x15\xa1\xf3\xaf\xae"
        "\x68\x6d\x22\x9b\x9f\x71\x83\x2e\x16\xc4\x9a\x14\x45\xa0\x84\xaa\x61"
        "\x49\x84\x2e\x92\x31\x64\x52\x43\x2b\xd5\x13\x8f\xed\x11\xd6\x28\x80"
        "\xb2\x28\x58\xa8\x25\x8d\x58\x05\x48\x48\xdd\x51\x46\x55\xc6\x6f\x31"
        "\xa4\xc3\x67\xc3\xef\x1e\x25\x7c\x94\x8c\x85\x66\xb7\x42\x29\xf8\xf9"
        "\x50\xd5\x38\xb2\x7b\xeb\xf2\x11\x55\x98\x1e\xb6\x47\xd8\x98\xdd\x02"
        "\x75\x85\xf0\x12\x6e\x51\x0d\x86\x57\xcd\x49\x15\x10\x53\x1a\x5d\x44"
        "\x0b\xf0\x18\x4f\xe2\xcb\x33\x9b\xa0\xc7\x4b\x86\xc1\xbe\x7e\xc9\x32"
        "\xc9\xad\xc7\xe3\x28\x58\x85\x2f\x5d\x73\x44\xd2\xdf\xcc\x1d\x81\x1c"
        "\x13\x8d\xf9\x92\x67\x94\x71\x54\xd5\xfa\xae\x3a\x33\xf1\x09\x14\x74"
        "\xda\x38\x6f\x4e\xf5\x85\x19\xe0\xfd\x94\x58\x68\x2d\x42\x39\xfa\x40"
        "\x9a\xe0\x00\x29\x65\x51\x04\x6d\x06\x73\x4a\x1b\x0b\x68\x42\xa1\x36"
        "\xeb\xee\x90\x90\x81\x94\x15\x85\x7f\x26\xfe\x39\x09\x33\x91\x93\x04"
        "\xb5\x36\x1f\xee\xd2\xe2\x9f\xac\xe0\x59\xf3\x5b\xa3\x25\xe8\x59\xd2"
        "\xf6\x7c\x8f\x22\x43\x43\xf4\xd9\x32\xcd\x66\x98\x4a\x0b\x62\x04\x0e"
        "\xd5\xa7\xb8\x4b\xa2\xcb\xbd\xe6\xed\x5a\x78\x2d\x50\x3f\xe3\x6c\x47"
        "\x97\xf2\x96\x99\xe6\xfe\x99\xc6\xc9\x11\x17\x09\x32\xd2\x30\x7f\x80"
        "\xd5\x43\xd2\x4d\x46\x90\x3d\xdd\xfd\x70\xbc\x39\x84\x01\xbc\x75\x5d"
        "\x6b\x68\xc1\xab\x75\x1b\xf8\x84\x6c\x61\x27\x79\x3c\xa7\xe1\xc5\x7d"
        "\xaf\x22\x6d\xcd\x57\x8d\x53\x74\xa8\x9e\x35\xf7\xe9\xf3\xb6\x65\x28"
        "\x7d\x18\x27\x0b\x02\xcf\x7a\x70\x89\x8d\xc9\x63\x55\xeb\x4e\x20\x04"
        "\xbc\x8e\x3e\x24\xf0\x92\x7f\x24\x82\x90\x24\xa7\xf4\x61\xd6\x5c\xed"
        "\x06\x6b\x75\xb6\x49\x15\x68\xcb\x3f\x6b\x5f\x50\x86\x63\x31\xc9\x5e"
        "\xf4\x74\x37\x9b\xbd\x13\xb0\x3b\x5a\xf0\x1e\xa7\x90\x6c\x7c\xa0",
        4096);
    *(uint16_t*)0x20001004 = 0x1000;
    syscall(__NR_write, r[3], 0x20000000ul, 0x1006ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
