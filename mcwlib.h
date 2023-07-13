#ifndef MCWLIB_H
#define MCWLIB_H

#include <QtCore/qglobal.h>

#if defined(HY_EXPORT_LIB)
#define MCWLIB_EXPORT   Q_DECL_EXPORT
#else
#define MCWLIB_EXPORT Q_DECL_IMPORT
#endif

#endif // MCWLIB_H
