#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>

int main(int argc, char **argv)
{
  pid_t pid = atoi(argv[1]);
  task_t task;
  kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
  if (kr != KERN_SUCCESS) return -1;
  printf("tfp %d", task);
  return 0;
}
