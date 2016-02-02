#ifndef PERF_GLOBAL_H
#define PERF_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PERF_LIBRARY)
#  define PERFSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PERFSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PERF_GLOBAL_H
