#include <stdio.h>
#include <unistd.h>

// from https://github.com/apple-oss-distributions/xnu/blob/94d3b452840153a99b38a3a9659680b2a006908e/osfmk/kern/cs_blobs.h
#define CS_FORCED_LV                0x00000010  /* Library Validation required by Hardened System Policy */
#define CS_INVALID_ALLOWED          0x00000020  /* (macOS Only) Page invalidation allowed by task port policy */

#define CS_HARD                     0x00000100  /* don't load invalid pages */
#define CS_KILL                     0x00000200  /* kill process if it becomes invalid */

extern int csops(pid_t pid, unsigned int  ops, void * useraddr, size_t usersize);

int detect_tfp_or_pft()
{
  uint32_t cs_flags = 0;
  csops(getpid(), 0, &cs_flags, sizeof(cs_flags));
  printf("cs_flags=0x%x\n", cs_flags);

  if (cs_flags & CS_INVALID_ALLOWED) return 1;
  return 0;
}

int main()
{
  printf("pid %d\n", getpid());
  while (1)
  {
    int a = detect_tfp_or_pft();
    printf("%d\n", a);
    sleep(5);
  }
  return 0;
}
