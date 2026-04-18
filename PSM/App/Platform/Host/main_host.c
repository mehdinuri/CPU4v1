/*
 * App/Platform/Host/main_host.c
 *
 * Host entry point for manual smoke testing with mock adapters.
 * The CTest-registered test executables in Tests/ are the primary
 * test runners; this binary is for interactive development.
 */

#include <stdio.h>

int main(void)
{
  printf("PSM host runner — Domain compiled, mock adapters ready.\n");
  printf("Run 'ctest' in the build directory to execute the test suite.\n");

  return 0;
}
