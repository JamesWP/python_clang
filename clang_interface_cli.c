#include "clang_interface.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
  clang_interface_init();

  clang_interface_EnvironmentHandle handle;
  memset(&handle, '\0', sizeof(clang_interface_EnvironmentHandle));

  int rc = 0;

  if (0 != (rc = clang_interface_createEnvironment(&handle)))
  {
    printf("Unable to create env. rc=%d\n", rc);
    clang_interface_fini();
    return 1;
  }

  char *code_buff = malloc(BUFF_SIZE);

  int num = 10;

  rc = snprintf(code_buff, BUFF_SIZE, "static int a = %d; int go() { return a++; }", num);

  assert(rc > 0);

  size_t code_size = rc;
  printf("Code: ");
  fwrite(code_buff, code_size, 1, stdout);
  printf("\n");

  if (0 != (rc = clang_interface_compileCode(&handle, code_buff, code_size)))
  {
    printf("Unable to compile code. rc=%d\n", rc);
    goto exit_and_destroy;
  }

  int output = 0;
  if (0 != (rc = clang_interface_runCode(&handle, &output)))
  {
    printf("Error executing code. rc=%d\n", rc);
    goto exit_and_destroy;
  }

  printf("Code executed. output=%d\n", output);

  if (0 != (rc = clang_interface_runCode(&handle, &output)))
  {
    printf("Error executing code. rc=%d\n", rc);
    goto exit_and_destroy;
  }

  printf("Code executed. output=%d\n", output);


exit_and_destroy:
  free(code_buff);

  if (0 != (rc = clang_interface_destroyEnvironment(&handle)))
  {
    printf("Unable to destroy env. rc=%d\n", rc);
    return 1;
  }

  clang_interface_fini();

  return 0;
}
