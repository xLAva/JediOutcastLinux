
#include "g_local.h"
#ifdef __linux__
unsigned int timeGetTime()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec/1000;
}
#endif