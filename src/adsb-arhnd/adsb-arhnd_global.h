#include <QtGlobal>

// Define ADSB_ARHND_STATIC in you applikation if you want to link against the
// static version of ADSB_ARHND

#ifdef ADSB_ARHND_STATIC
#   define ADSB_ARHND_EXPORT
#else
#  if defined(ADSB_ARHND_LIBRARY)
#    define ADSB_ARHND_EXPORT Q_DECL_EXPORT
#  else
#    define ADSB_ARHND_EXPORT Q_DECL_IMPORT
#  endif
#endif


#ifndef ADSB_ARHND_H
#define ADSB_ARHND_H

/*
   VERSION is (major << 16) + (minor << 8) + patch.
*/
#define ADSB_ARHND_VERSION ADSB_ARHND_VERSION_CHECK(ADSB_ARHND_VERSION_MAJOR, ADSB_ARHND_VERSION_MINOR, ADSB_ARHND_VERSION_PATCH)

/*
   can be used like #if (ADSB_ARHND_VERSION >= ADSB_ARHND_VERSION_CHECK(1, 3, 0))
*/
#define ADSB_ARHND_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#endif
