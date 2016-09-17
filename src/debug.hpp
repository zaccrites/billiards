
#include <stdio.h>

// For use with format strings. Converts a boolean argument into a "true" or "false" string.
#define BOOL_STRING(arg) arg ? "true" : "false"

#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, "DEBUG: "); fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif
