#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>

// from https://newosxbook.com/articles/PST2.html
mach_port_t task_for_pid_workaround(int Pid)
{

  host_t        myhost = mach_host_self(); // host self is host priv if you're root anyway..
  mach_port_t   psDefault;
  mach_port_t   psDefault_control;

  task_array_t  tasks;
  mach_msg_type_number_t numTasks;
  int i;

   thread_array_t       threads;
   thread_info_data_t   tInfo;

  kern_return_t kr;

  kr = processor_set_default(myhost, &psDefault);

  kr = host_processor_set_priv(myhost, psDefault, &psDefault_control);
 if (kr != KERN_SUCCESS) { fprintf(stderr, "host_processor_set_priv failed with error %x\n", kr);
         mach_error("host_processor_set_priv",kr); exit(1);}

  printf("So far so good\n");

  kr = processor_set_tasks(psDefault_control, &tasks, &numTasks);
  if (kr != KERN_SUCCESS) { fprintf(stderr,"processor_set_tasks failed with error %x\n",kr); exit(1); }

  for (i = 0; i < numTasks; i++)
        {
                int pid;
                pid_for_task(tasks[i], &pid);
                printf("TASK %d PID :%d\n", i,pid);
                if (pid == Pid) return (tasks[i]);
        }

   return (MACH_PORT_NULL);
} // end workaround

int main(int argc, char **argv)
{
    int pid = atoi(argv[1]);
    mach_port_t task = task_for_pid_workaround(pid);
    if (task == MACH_PORT_NULL) return -1;
    printf("pft %d\n", task);
    return 0;
}
