#ifndef SEH_COMPAT_H_
#define SEH_COMPAT_H_

#if !defined(_MSC_VER)
#ifndef __try
#define __try try
#endif
#ifndef __except
#define __except(...) catch(...)
#endif
#endif

#endif
