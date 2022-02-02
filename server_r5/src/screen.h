
#ifdef WIN32
#ifdef _CONSOLE
#define NB_SCREEN 1
#else
#define NB_SCREEN 64
#endif

#else
#define NB_SCREEN 1
#endif

#define W_READER  2