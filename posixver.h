/*
@(#)File:           $RCSfile: posixver.h,v $
@(#)Version:        $Revision: 1.4 $
@(#)Last changed:   $Date: 2017/06/18 00:15:42 $
@(#)Purpose:        Request appropriate POSIX and X/Open Support
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2010-2017
@(#)Product:        SCC Version 8.0.2 (2022-05-29)
*/

/*TABSTOP=4*/

#ifndef JLSS_ID_POSIXVER_H
#define JLSS_ID_POSIXVER_H

/*
** Include this file before including system headers.  By default, with
** C99 support from the compiler, it requests POSIX 2008 support.  With
** C89 support only, it requests POSIX 1997 support.  Override the
** default behaviour by setting either _XOPEN_SOURCE or _POSIX_C_SOURCE.
*/

/* _XOPEN_SOURCE 700 is loosely equivalent to _POSIX_C_SOURCE 200809L */
/* _XOPEN_SOURCE 600 is loosely equivalent to _POSIX_C_SOURCE 200112L */
/* _XOPEN_SOURCE 500 is loosely equivalent to _POSIX_C_SOURCE 199506L */

#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
#if defined(__cplusplus)
#define _XOPEN_SOURCE 700   /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#elif __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 700   /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#else
#define _XOPEN_SOURCE 500   /* SUS v2, POSIX 1003.1 1997 */
#endif /* __STDC_VERSION__ */
#endif /* !_XOPEN_SOURCE && !_POSIX_C_SOURCE */

#endif /* JLSS_ID_POSIXVER_H */
