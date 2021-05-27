#if !defined(MAX_PATH)
#if defined(PATH_MAX)
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif
#endif
