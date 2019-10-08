// INFO: task hung in do_ip_vs_set_ctl (2)
// https://syzkaller.appspot.com/bug?id=26aa22915f5e3b7ca2cfca76a939f12c25d624db
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x2b, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20000240 = 2;
    memcpy((void*)0x20000244,
           "\x69\x70\x5f\x76\x74\x69\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000254 = 0;
    syscall(__NR_setsockopt, r[0], 0, 0x48b, 0x20000240, 0x18);
    break;
  case 2:
    *(uint32_t*)0x20000140 = 2;
    memcpy((void*)0x20000144,
           "\x65\x71\x6c\x00\x00\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000154 = 0;
    syscall(__NR_setsockopt, r[0], 0, 0x48c, 0x20000140, 0x18);
    break;
  }
}

void execute_one()
{
  execute(3);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}