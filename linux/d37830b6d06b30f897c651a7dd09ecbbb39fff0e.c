// INFO: task hung in __fuse_request_send
// https://syzkaller.appspot.com/bug?id=d37830b6d06b30f897c651a7dd09ecbbb39fff0e
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

static void loop(void)
{
  int i, call, thread;
  for (call = 0; call < 9; call++) {
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
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000180, "./file0\000", 8);
    memcpy((void*)0x20000000, "fusectl\000", 8);
    syscall(__NR_mount, 0, 0x20000180, 0x20000000, 3, 0);
    break;
  case 1:
    memcpy((void*)0x20000080, "./file0\000", 8);
    syscall(__NR_mkdir, 0x20000080, 0);
    break;
  case 2:
    memcpy((void*)0x20000180, "/dev/fuse\000", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    memcpy((void*)0x20000200, "./file0\000", 8);
    memcpy((void*)0x20000240, "fuse\000", 5);
    memcpy((void*)0x20000400, "fd", 2);
    *(uint8_t*)0x20000402 = 0x3d;
    sprintf((char*)0x20000403, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20000415 = 0x2c;
    memcpy((void*)0x20000416, "rootmode", 8);
    *(uint8_t*)0x2000041e = 0x3d;
    sprintf((char*)0x2000041f, "%023llo", (long long)0x4000);
    *(uint8_t*)0x20000436 = 0x2c;
    memcpy((void*)0x20000437, "user_id", 7);
    *(uint8_t*)0x2000043e = 0x3d;
    sprintf((char*)0x2000043f, "%020llu", (long long)0);
    *(uint8_t*)0x20000453 = 0x2c;
    memcpy((void*)0x20000454, "group_id", 8);
    *(uint8_t*)0x2000045c = 0x3d;
    sprintf((char*)0x2000045d, "%020llu", (long long)0);
    *(uint8_t*)0x20000471 = 0x2c;
    *(uint8_t*)0x20000472 = 0;
    syscall(__NR_mount, 0, 0x20000200, 0x20000240, 0, 0x20000400);
    break;
  case 4:
    syscall(__NR_read, r[0], 0x20002540, 0x1000);
    break;
  case 5:
    memcpy((void*)0x20000580, "./file0/file1\000", 14);
    syscall(__NR_stat, 0x20000580, 0);
    break;
  case 6:
    syscall(__NR_read, r[0], 0x20003540, 0x1000);
    break;
  case 7:
    *(uint32_t*)0x20000100 = 0x50;
    *(uint32_t*)0x20000104 = 0;
    *(uint64_t*)0x20000108 = 1;
    *(uint32_t*)0x20000110 = 7;
    *(uint32_t*)0x20000114 = 0x1f;
    *(uint32_t*)0x20000118 = 0;
    *(uint32_t*)0x2000011c = 0;
    *(uint16_t*)0x20000120 = 0;
    *(uint16_t*)0x20000122 = 0;
    *(uint32_t*)0x20000124 = 0;
    *(uint32_t*)0x20000128 = 0;
    *(uint32_t*)0x2000012c = 0;
    *(uint32_t*)0x20000130 = 0;
    *(uint32_t*)0x20000134 = 0;
    *(uint32_t*)0x20000138 = 0;
    *(uint32_t*)0x2000013c = 0;
    *(uint32_t*)0x20000140 = 0;
    *(uint32_t*)0x20000144 = 0;
    *(uint32_t*)0x20000148 = 0;
    *(uint32_t*)0x2000014c = 0;
    syscall(__NR_write, r[0], 0x20000100, 0x50);
    break;
  case 8:
    *(uint32_t*)0x20000000 = 0x2e;
    *(uint32_t*)0x20000004 = 6;
    *(uint64_t*)0x20000008 = 0;
    *(uint64_t*)0x20000010 = 1;
    *(uint64_t*)0x20000018 = 0;
    *(uint32_t*)0x20000020 = 5;
    *(uint32_t*)0x20000024 = 0;
    memcpy((void*)0x20000028, "fuse\000", 5);
    *(uint8_t*)0x2000002d = 0;
    syscall(__NR_write, r[0], 0x20000000, 0x2e);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}