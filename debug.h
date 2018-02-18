#ifndef __debug_h__
#define __debug_h__

#ifndef NDEBUG
#define debug(M, ... ) fprintf( stderr, "DEBUG: %s:%d: " M "\n", __FILE__,   __LINE__, ##__VA_ARGS__ )
#else
#define debug(M, ... )
#endif

#endif
