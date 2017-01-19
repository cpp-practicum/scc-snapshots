/*
@(#)File:           $RCSfile: scc.c,v $
@(#)Version:        $Revision: 4.4 $
@(#)Last changed:   $Date: 2008/08/09 20:15:06 $
@(#)Purpose:        Strip C comments
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1991,1993,1997-98,2003,2005
*/

/*TABSTOP=4*/

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

/*
**	This processor removes any C comments and replaces them by a
**	single space.  It will be used as part of a formatting pipeline
**	for checking the equivalence of C code.
**
**  Note that backslashes at the end of a line can extend even a C++
**  style comment over several lines.  It matters not whether there is
**  one backslash or several -- the line splicing (logically) takes
**  place before any other tokenisation.
*/

#include <assert.h>
#include <stdio.h>
#include <unistd.h>	/* getopt() on MacOS X 10.2 */
#include "stderr.h"
#include "filter.h"

typedef enum { NonComment, CComment, CppComment } Comment;

static int Cflag = 0;	/* Strip C++/C99 style (//) comments */
static int wflag = 0;	/* Warn about nested C-style comments */
static int nline = 0;	/* Line counter */

static const char usestr[] = "[-wCV] [file ...]";

#ifndef lint
static const char rcs[] = "@(#)$Id: scc.c,v 4.4 2008/08/09 20:15:06 jleffler Exp $";
#endif

static int getch(FILE *fp)
{
	int c = getc(fp);
	if (c == '\n')
		nline++;
	return(c);
}

static int ungetch(char c, FILE *fp)
{
	if (c == '\n')
		nline--;
	return(ungetc(c, fp));
}

static int peek(FILE *fp)
{
	int	c;

	if ((c = getch(fp)) != EOF)
		ungetch(c, fp);
	return(c);
}

static void warning(const char *str, const char *file, int line)
{
	err_report(ERR_REM, ERR_STAT, "%s:%d: %s\n", file, line, str);
}

static void warning2(const char *s1, const char *s2, const char *file, int line)
{
	err_report(ERR_REM, ERR_STAT, "%s:%d: %s %s\n", file, line, s1, s2);
}

static void endquote(char q, FILE *fp, char *fn, const char *msg)
{
	int 	c;

	while ((c = getch(fp)) != EOF && c != q)
	{
		putchar(c);
		if (c == '\\')
		{
			if ((c = getch(fp)) == EOF)
				break;
			putchar(c);
			if (c == '\\' && peek(fp) == '\n')
				putchar(getch(fp));
		}
		else if (c == '\n')
			warning2("newline in", msg, fn, nline);
	}
	if (c == EOF)
	{
		warning2("EOF in", msg, fn, nline);
		return;
	}
	putchar(q);
}

/* Count the number of backslash newline pairs that immediately follow in the input stream */
/* On entry, peek() might return the backslash of a backslash newline pair, or some other character. */
/* On exit, getch() will return the first character after the sequence of n (n >= 0) backslash newline pairs */
/* Relies on two characters of pushback (but only of data already read) - not mandated by POSIX or C standards. */
static int read_bsnl(FILE *fp)
{
	int n = 0;
	int c;

	while ((c = peek(fp)) != EOF)
	{
		if (c != '\\')
			return(n);
		c = getch(fp);
		if ((c = peek(fp)) != '\n')
		{
			ungetch('\\', fp);
			return(n);
		}
		n++;
		c = getch(fp);
	}
	return(n);
}

static void write_bsnl(FILE *fp, int bsnl)
{
	while (bsnl-- > 0)
	{
		putc('\\', fp);
		putc('\n', fp);
	}
}

static void scc(FILE *fp, char *fn)
{
	int bsnl;
	int oc;
	int pc;
	int c;
	int l_nest = 0;	/* Last line with a nested comment warning */
	int l_cend = 0;	/* Last line with a comment end warning */
	Comment	status = NonComment;

	nline = 1;
	for (oc = '\0'; (c = getch(fp)) != EOF; oc = c)
	{
		switch (status)
		{
		case CComment:
			if (c == '*')
			{
				bsnl = read_bsnl(fp);
				if (peek(fp) == '/')
				{
					status = NonComment;
					c = getch(fp);
					putchar(' ');
				}
			}
			else if (wflag == 1 && c == '/' && peek(fp) == '*')
			{
				/* NB: does not detect star backslash newline slash as embedded end of comment */
				if (l_nest != nline)
					warning("nested C-style comment", fn, nline);
				l_nest = nline;
			}
			break;
		case CppComment:
			if (c == '\n' && oc != '\\')
			{
				status = NonComment;
				putchar(c);
			}
			break;
		case NonComment:
			if (c == '*' && wflag == 1 && peek(fp) == '/')
			{
				/* NB: does not detect star backslash newline slash as stray end of comment */
				if (l_cend != nline)
					warning("C-style comment end marker not in a comment",
							fn, nline);
				l_cend = nline;
			}
			if (c == '\'')
			{
				putchar(c);
				/*
				** Single quotes can contain multiple characters, such as
				** '\\', '\'', '\377', '\x4FF', 'ab', '/ *' (with no space, and
				** the reverse character pair) , etc.  Scan for an unescaped
				** closing single quote.  Newlines are not acceptable either,
				** unless preceded by a backslash -- so both '\<nl>\n' and
				** '\\<nl>n' are OK, and are equivalent to a newline character
				** (when <nl> is a physical newline in the source code).
				*/
				endquote('\'', fp, fn, "character constant");
			}
			else if (c == '"')
			{
				putchar(c);
				/* Double quotes are relatively simple, except that */
				/* they can legitimately extend over several lines */
				/* when each line is terminated by a backslash */
				endquote('\"', fp, fn, "string literal");
			}
			else if (c != '/')
			{
				putchar(c);
			}
			/* c is a slash from here on - potential start of comment */
			else
			{
				bsnl = read_bsnl(fp);
				if ((pc = peek(fp)) == '*')
				{
					status = CComment;
					c = getch(fp);
				}
				else if (Cflag == 1 && pc == '/')
				{
					status = CppComment;
					c = getch(fp);
				}
				else
				{
					putchar(c);
					write_bsnl(stdout, bsnl);
				}
			}
			break;
		}
	}
	if (status != NonComment)
		warning("unterminated C-style comment", fn, nline);
}

int main(int argc, char **argv)
{
	int opt;

	err_setarg0(argv[0]);

	while ((opt = getopt(argc, argv, "wCV")) != EOF)
	{
		switch (opt)
		{
		case 'w':
			wflag = 1;
			break;
		case 'C':
			Cflag = 1;
			break;
		case 'V':
			err_version("SCC", "$Revision: 4.4 $ ($Date: 2008/08/09 20:15:06 $)");
			break;
		default:
			err_usage(usestr);
			break;
		}
	}

	filter(argc, argv, optind, scc);
	return(0);
}
