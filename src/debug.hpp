
#include <stdio.h>

#define BOOL_STRING(arg) arg ? "true" : "false"

#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif
