# Detecting `task_for_pid` and `pid_for_task` via `CS_INVALID_ALLOWED`. First public detection method pid_for_task.

## Overview

When a process obtains another process's Mach task port, whether through `task_for_pid()` or the `pid_for_task()` workaround (Jonathan Levin's technique), the XNU kernel sets the `CS_INVALID_ALLOWED` (0x20) flag on the **target** process's code signing flags.

This flag can be read from userspace via the `csops()`.

## The problem
`TASK_EXTMOD_INFO` (flavor 0x13 via `task_info()`) provides a `task_for_pid_count` counter, but it is **blind** to `pid_for_task`, the counter stays at 0 when the task port is obtained through the workaround.

## The solution
`csops(pid, CS_OPS_STATUS)` returns the process's code signing flags. Bit `0x20` (`CS_INVALID_ALLOWED`) is set by the kernel when **any** method is used to obtain the task port:

## Detection code

```c
// from xnu/osfmk/kern/cs_blobs.h
#define CS_INVALID_ALLOWED 0x00000020

extern int csops(pid_t pid, unsigned int ops, void *useraddr, size_t usersize);

int detect_tfp_or_pft()
{
  uint32_t cs_flags = 0;
  csops(getpid(), 0, &cs_flags, sizeof(cs_flags));
  if (cs_flags & CS_INVALID_ALLOWED) return 1;
  return 0;
}
```

## PoC

Three binaries:

- **test.c**, target process, polls `csops()` every 5 seconds and prints `1` if `CS_INVALID_ALLOWED` is set
- **tfp.c**, calls `task_for_pid()` on the target
- **pft.c**, uses `pid_for_task()` workaround

### Build

```bash
clang -o test test.c
clang -o tfp tfp.c
clang -o pft pft.c
```

### Test 1: `task_for_pid`

```
$ ./test
pid 10392
cs_flags=0x22020201
0                        <-- clean
cs_flags=0x22020201
0

$ sudo ./tfp 10392       <-- task_for_pid called
tfp 2563

cs_flags=0x22020221
1                        <-- DETECTED
```

### Test 2: `pid_for_task` workaround

```
$ ./test
pid 10503
cs_flags=0x22020201
0                        <-- clean

$ sudo ./pft 10503       <-- pid_for_task workaround
So far so good
TASK 0 PID :1673
TASK 1 PID :1706
TASK 2 PID :37262
TASK 3 PID :37278
TASK 4 PID :47080
TASK 5 PID :87939
TASK 6 PID :4322
TASK 7 PID :4398
TASK 8 PID :5912
TASK 9 PID :9669
TASK 10 PID :10167
TASK 11 PID :10366
TASK 12 PID :10503
TASK 12 PID :10503
pft 4099

cs_flags=0x22020221
1                        <-- DETECTED
```

## Notes
- Tested on macOS 27.0 beta 2, arm64.
- Userspace detection of previously granted Mach task-port access, covering both task_for_pid() and task enumeration followed by pid_for_task().

## References

- [xnu/osfmk/kern/cs_blobs.h](https://github.com/apple-oss-distributions/xnu/blob/94d3b452840153a99b38a3a9659680b2a006908e/osfmk/kern/cs_blobs.h), `CS_INVALID_ALLOWED` definition
- [pid_for_task](https://newosxbook.com/articles/PST2.html), Jonathan Levin, `pid_for_task` workaround for `task_for_pid`
