// KMSAN: kernel-infoleak in copy_page_to_iter
// https://syzkaller.appspot.com/bug?id=2840599d5137366802ca7d80de4d34ad51d94c23
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <string.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

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
        if (collide && call % 2)
          break;
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

uint64_t r[1] = {0x0};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    syscall(__NR_clone, 0x200, 0x20fbf000, 0x20000140, 0x20000100, 0x200000c0);
    break;
  case 1:
    res = syscall(__NR_gettid);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    NONFAILING(*(uint64_t*)0x20000440 = 0x20000540);
    NONFAILING(*(uint64_t*)0x20000448 = 0xed);
    NONFAILING(*(uint64_t*)0x200006c0 = 0x20000640);
    NONFAILING(*(uint64_t*)0x200006c8 = 0x48);
    syscall(__NR_process_vm_readv, r[0], 0x20000440, 1, 0x200006c0, 1, 0);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x20363ff8, "./file0", 8));
    syscall(__NR_open, 0x20363ff8, 0x401, 0);
    break;
  }
}

void loop()
{
  execute(4);
  collide = 1;
  execute(4);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}