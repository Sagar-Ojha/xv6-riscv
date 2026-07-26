#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  int totalBytes  = 4096*5;
  int parsedBytes = 0;

  char* secret = sbrk(totalBytes);

  while ((parsedBytes < totalBytes) &&
        !((('0' < *secret) && (*secret < '9')) ||
          (('a' < *secret) && (*secret < 'z')) ||
          (('A' < *secret) && (*secret < 'Z'))))
  {
    parsedBytes++;
    secret++;
  }

  if (parsedBytes < totalBytes)
  {
    if (strcmp(secret, "This may help.") == 0)
    {
      secret = secret + 16;
    }
    fprintf(2, "%s", secret);
  }

  exit(1);
}
