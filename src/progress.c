/*
 * Extracted from wget
 *
 * Copyright (C) 2001 Free Software Foundation, Inc. (progress.c)
 * Copyright (C) 1995, 1996, 1997, 2000, 2001 Free Software Foundation, Inc
 *   (utils.c)
 *
 * ported to compile with IOL/curl by Juan F. Codagnone (2003)
 * (the code that use tab as indentation :^D )
 *
 * GNU Wget is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNU Wget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wget; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, the Free Software Foundation
 * gives permission to link the code of its release of Wget with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables.  You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL".  If you
 * modify this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_SYS_IOCTL_H
  #include <sys/ioctl.h>
#endif

#ifdef HAVE_GETTIMEOFDAY
   #include <sys/time.h>
#endif

#ifdef WIN32
   #include <windows.h>
#endif
#include <glib.h>
#include "i18n.h"

#include "progress.h"

#define xfree g_free
#define xmalloc g_malloc
#define xrealloc g_realloc

struct dot_progress 
{	/** how many bytes have been downloaded previusly */
	long initial_length;

	/** expected total byte count when the download finishes */
	long total_length;
	
	int accumulated;
	int rows;	/**< number of rows printed so far */
	int dots;	/**< number of dots printed in this row */
	long last_timer_value;
	
};

#define DOT_BYTES	1024
#define DOT_SPACING	10
#define DOTS_IN_LINE	50

/* Engine for legible and legible_very_long; this function works on
   strings.  */

static char *
legible_1 (const char *repr)
{
  static char outbuf[128];
  int i, i1, mod;
  char *outptr;
  const char *inptr;

  /* Reset the pointers.  */
  outptr = outbuf;
  inptr = repr;
  /* If the number is negative, shift the pointers.  */
  if (*inptr == '-')
    {
      *outptr++ = '-';
      ++inptr;
    }
  /* How many digits before the first separator?  */
  mod = strlen (inptr) % 3;
  /* Insert them.  */
  for (i = 0; i < mod; i++)
    *outptr++ = inptr[i];
  /* Now insert the rest of them, putting separator before every
     third digit.  */
  for (i1 = i, i = 0; inptr[i1]; i++, i1++)
    {
      if (i % 3 == 0 && i1 != 0)
	*outptr++ = ',';
      *outptr++ = inptr[i1];
    }
  /* Zero-terminate the string.  */
  *outptr = '\0';
  return outbuf;
}

/* Print NUMBER to BUFFER in base 10.  This should be completely
   equivalent to `sprintf(buffer, "%ld", number)', only much faster.

   The speedup may make a difference in programs that frequently
   convert numbers to strings.  Some implementations of sprintf,
   particularly the one in GNU libc, have been known to be extremely
   slow compared to this function.

   Return the pointer to the location where the terminating zero was
   printed.  (Equivalent to calling buffer+strlen(buffer) after the
   function is done.)

   BUFFER should be big enough to accept as many bytes as you expect
   the number to take up.  On machines with 64-bit longs the maximum
   needed size is 24 bytes.  That includes the digits needed for the
   largest 64-bit number, the `-' sign in case it's negative, and the
   terminating '\0'.  */

static char *
number_to_string (char *buffer, long number)
{
  char *p = buffer;
  long n = number;

#if (SIZEOF_LONG != 4) && (SIZEOF_LONG != 8)
  /* We are running in a strange or misconfigured environment.  Let
     sprintf cope with it.  */
  sprintf (buffer, "%ld", n);
  p += strlen (buffer);
#else  /* (SIZEOF_LONG == 4) || (SIZEOF_LONG == 8) */

  if (n < 0)
    {
      *p++ = '-';
      n = -n;
    }

  if      (n < 10)                   { DIGITS_1 (1); }
  else if (n < 100)                  { DIGITS_2 (10); }
  else if (n < 1000)                 { DIGITS_3 (100); }
  else if (n < 10000)                { DIGITS_4 (1000); }
  else if (n < 100000)               { DIGITS_5 (10000); }
  else if (n < 1000000)              { DIGITS_6 (100000); }
  else if (n < 10000000)             { DIGITS_7 (1000000); }
  else if (n < 100000000)            { DIGITS_8 (10000000); }
  else if (n < 1000000000)           { DIGITS_9 (100000000); }
#if SIZEOF_LONG == 4
  /* ``if (1)'' serves only to preserve editor indentation. */
  else if (1)                        { DIGITS_10 (1000000000); }
#else  /* SIZEOF_LONG != 4 */
  else if (n < 10000000000L)         { DIGITS_10 (1000000000L); }
  else if (n < 100000000000L)        { DIGITS_11 (10000000000L); }
  else if (n < 1000000000000L)       { DIGITS_12 (100000000000L); }
  else if (n < 10000000000000L)      { DIGITS_13 (1000000000000L); }
  else if (n < 100000000000000L)     { DIGITS_14 (10000000000000L); }
  else if (n < 1000000000000000L)    { DIGITS_15 (100000000000000L); }
  else if (n < 10000000000000000L)   { DIGITS_16 (1000000000000000L); }
  else if (n < 100000000000000000L)  { DIGITS_17 (10000000000000000L); }
  else if (n < 1000000000000000000L) { DIGITS_18 (100000000000000000L); }
  else                               { DIGITS_19 (1000000000000000000L); }
#endif /* SIZEOF_LONG != 4 */

  *p = '\0';
#endif /* (SIZEOF_LONG == 4) || (SIZEOF_LONG == 8) */

  return p;
}

/* Legible -- return a static pointer to the legibly printed long.  */
static char *
legible (long l)
{
  char inbuf[24];
  /* Print the number into the buffer.  */
  number_to_string (inbuf, l);
  return legible_1 (inbuf);
}

/** Count the digits in a (long) integer.  */
static int
numdigit (long number)
{
	int cnt = 1;
	if (number < 0)
        {	number = -number;
        	++cnt;
        }
        while ((number /= 10) > 0)
        	++cnt;
        return cnt;
}

/* Support for timers. */

#undef TIMER_WINDOWS
#undef TIMER_GETTIMEOFDAY
#undef TIMER_TIME

/* Depending on the OS and availability of gettimeofday(), one and
   only one of the above constants will be defined.  Virtually all
   modern Unix systems will define TIMER_GETTIMEOFDAY; Windows will
   use TIMER_WINDOWS.  TIMER_TIME is a catch-all method for
   non-Windows systems without gettimeofday.

   #### Perhaps we should also support ftime(), which exists on old
   BSD 4.2-influenced systems?  (It also existed under MS DOS Borland
   C, if memory serves me.)  */

#ifdef WIN32
# define TIMER_WINDOWS
#else  /* not WINDOWS */
# ifdef HAVE_GETTIMEOFDAY
#  define TIMER_GETTIMEOFDAY
# else
#  define TIMER_TIME
# endif
#endif /* not WINDOWS */

struct wget_timer {
#ifdef TIMER_GETTIMEOFDAY
  long secs;
  long usecs;
#endif

#ifdef TIMER_TIME
  time_t secs;
#endif

#ifdef TIMER_WINDOWS
  ULARGE_INTEGER wintime;
#endif
};

/* Allocate a timer.  It is not legal to do anything with a freshly
   allocated timer, except call wtimer_reset() or wtimer_delete().  */

static struct wget_timer *
wtimer_allocate (void)
{
  struct wget_timer *wt =
    (struct wget_timer *)xmalloc (sizeof (struct wget_timer));
  return wt;
}

/* Reset timer WT.  This establishes the starting point from which
   wtimer_elapsed() will return the number of elapsed
   milliseconds.  It is allowed to reset a previously used timer.  */

static void
wtimer_reset (struct wget_timer *wt)
{
#ifdef TIMER_GETTIMEOFDAY
  struct timeval t;
  gettimeofday (&t, NULL);
  wt->secs  = t.tv_sec;
  wt->usecs = t.tv_usec;
#endif

#ifdef TIMER_TIME
  wt->secs = time (NULL);
#endif

#ifdef TIMER_WINDOWS
  FILETIME ft;
  SYSTEMTIME st;
  GetSystemTime (&st);
  SystemTimeToFileTime (&st, &ft);
  wt->wintime.HighPart = ft.dwHighDateTime;
  wt->wintime.LowPart  = ft.dwLowDateTime;
#endif
}

/* Allocate a new timer and reset it.  Return the new timer. */
static struct wget_timer *
wtimer_new (void)
{
  struct wget_timer *wt = wtimer_allocate ();
  wtimer_reset (wt);
  return wt;
}

/* Free the resources associated with the timer.  Its further use is
   prohibited.  */

static void
wtimer_delete (struct wget_timer *wt)
{
  xfree (wt);
}


/* Return the number of milliseconds elapsed since the timer was last
   reset.  It is allowed to call this function more than once to get
   increasingly higher elapsed values.  */

static long
wtimer_elapsed (struct wget_timer *wt)
{
#ifdef TIMER_GETTIMEOFDAY
  struct timeval t;
  gettimeofday (&t, NULL);
  return (t.tv_sec - wt->secs) * 1000 + (t.tv_usec - wt->usecs) / 1000;
#endif

#ifdef TIMER_TIME
  time_t now = time (NULL);
  return 1000 * (now - wt->secs);
#endif

#ifdef WIN32
  FILETIME ft;
  SYSTEMTIME st;
  ULARGE_INTEGER uli;
  GetSystemTime (&st);
  SystemTimeToFileTime (&st, &ft);
  uli.HighPart = ft.dwHighDateTime;
  uli.LowPart = ft.dwLowDateTime;
  return (long)((uli.QuadPart - wt->wintime.QuadPart) / 10000);
#endif
}

/* Return the assessed granularity of the timer implementation.  This
   is important for certain code that tries to deal with "zero" time
   intervals.  */

static long
wtimer_granularity (void)
{
#ifdef TIMER_GETTIMEOFDAY
  /* Granularity of gettimeofday is hugely architecture-dependent.
     However, it appears that on modern machines it is better than
     1ms.  */
  return 1;
#endif

#ifdef TIMER_TIME
  /* This is clear. */
  return 1000;
#endif

#ifdef TIMER_WINDOWS
  /* ? */
  return 1;
#endif
}

/**
 * Calculate the download rate and trim it as appropriate for the
 * speed.  Appropriate means that if rate is greater than 1K/s,
 * kilobytes are used, and if rate is greater than 1MB/s, megabytes
 * are used.
 *
 * UNITS is zero for B/s, one for KB/s, two for MB/s, and three for
 * GB/s.  
 */
static double
calc_rate (long bytes, long msecs, int *units)
{
	double dlrate;

	assert (msecs >= 0);
	assert (bytes >= 0);

	if (msecs == 0)
	    /* If elapsed time is 0, it means we're under the granularity of
	       the timer.  This often happens on systems that use time() for
	       the timer.  */
		msecs = wtimer_granularity ();

	dlrate = (double)1000 * bytes / msecs;
	if (dlrate < 1024.0)
		*units = 0;
	else if (dlrate < 1024.0 * 1024.0)
		*units = 1, dlrate /= 1024.0;
	else if (dlrate < 1024.0 * 1024.0 * 1024.0)
		*units = 2, dlrate /= (1024.0 * 1024.0);
	else
		/* Maybe someone will need this one day. 
		 * More realistically, it will get tickled by buggy timers. */
		 *units = 3, dlrate /= (1024.0 * 1024.0 * 1024.0);

	return dlrate;
}

/** Return a printed representation of the download rate, as
 * appropriate for the speed.  If PAD is non-zero, strings will be
 *  padded to the width of 7 characters (xxxx.xx).  
 */
static char *
retr_rate (long bytes, long msecs, int pad)
{
	static char res[20];
	static char *rate_names[] = {"B/s", "KB/s", "MB/s", "GB/s" };
	int units = 0;
	double dlrate = calc_rate (bytes, msecs, &units);

	sprintf (res, pad ? "%7.2f %s" : "%.2f %s", dlrate, rate_names[units]);

	return res;
}
                  
/* Dot-progress backend for progress_create. */
static void *
dot_create (long initial, long total)
{	 struct dot_progress *dp = g_malloc (sizeof (struct dot_progress));

	memset (dp, 0, sizeof (*dp));

	dp->initial_length = initial;
	dp->total_length   = total;

	if (dp->initial_length)
	{
		int dot_bytes = DOT_BYTES;
		long row_bytes = DOT_BYTES * DOTS_IN_LINE;
	
      		int remainder = (int) (dp->initial_length % row_bytes);
      		long skipped = dp->initial_length - remainder;

		if (skipped)
		{	/* skipped amount in K */
			int skipped_k = (int) (skipped / 1024); 
			int skipped_k_len = numdigit (skipped_k);

			if (skipped_k_len < 5)
				skipped_k_len = 5;

			  /* Align the [ skipping ... ] line with the dots.  
			   * To do that, insert the number of spaces equal to 
			   * the number of digits in the skipped amount in K.
			   */
			  printf ( _("\n%*s[ skipping %dK ]"),
				     2 + skipped_k_len, "", skipped_k);
		}
		printf ("\n%5ldK", skipped / 1024);
	
		for (; remainder >= dot_bytes; remainder -= dot_bytes)
		{
			if (dp->dots % DOT_SPACING == 0)
				printf(" ");
			printf(",");
			++dp->dots;
		}
		assert (dp->dots < DOTS_IN_LINE);
		dp->accumulated = remainder;
		dp->rows = skipped / row_bytes;
	}

	return dp;
}

static void
print_percentage (long bytes, long expected)
{
	int percentage = (int)(100.0 * bytes / expected);
	printf ( "%3d%%", percentage);
}

static void
print_download_speed (struct dot_progress *dp, long bytes, long dltime)
{
	printf ( " %s", retr_rate (bytes, dltime - dp->last_timer_value, 1));

	dp->last_timer_value = dltime;
}

/* Dot-progress backend for progress_update. */
static void
dot_update (void *progress, long howmuch, long dltime)
{
  struct dot_progress *dp = progress;
  int dot_bytes = DOT_BYTES;
  long row_bytes = DOT_BYTES * DOTS_IN_LINE;

  dp->accumulated += howmuch;
  for (; dp->accumulated >= dot_bytes; dp->accumulated -= dot_bytes)
    {
      if (dp->dots == 0)
	printf ( "\n%5ldK", dp->rows * row_bytes / 1024);

      if (dp->dots % DOT_SPACING == 0)
	printf( " ");
      printf(".");

      ++dp->dots;
      if (dp->dots >= DOTS_IN_LINE)
	{
	  long row_qty = row_bytes;
	  if (dp->rows == dp->initial_length / row_bytes)
	    row_qty -= dp->initial_length % row_bytes;

	  ++dp->rows;
	  dp->dots = 0;

	  if (dp->total_length)
	    print_percentage (dp->rows * row_bytes, dp->total_length);
	  print_download_speed (dp, row_qty, dltime);
	}
    }
    fflush(stdout);
}

/* Dot-progress backend for progress_finish. */

static void
dot_finish (void *progress, long dltime)
{
  struct dot_progress *dp = progress;
  int dot_bytes = DOT_BYTES;
  long row_bytes = DOT_BYTES * DOTS_IN_LINE;
  int i;

  if (dp->dots == 0)
    printf ( "\n%5ldK", dp->rows * row_bytes / 1024);
  for (i = dp->dots; i < DOTS_IN_LINE; i++)
    {
      if (i % DOT_SPACING == 0)
	printf(" ");
      printf(" ");
    }
  if (dp->total_length)
    {
      print_percentage (dp->rows * row_bytes
			+ dp->dots * dot_bytes
			+ dp->accumulated,
			dp->total_length);
    }

  {
    long row_qty = dp->dots * dot_bytes + dp->accumulated;
    if (dp->rows == dp->initial_length / row_bytes)
      row_qty -= dp->initial_length % row_bytes;
    print_download_speed (dp, row_qty, dltime);
  }

  printf("\n\n");
  fflush(stdout);
  xfree (dp);
}

/* "Thermometer" (bar) progress. */

/* Assumed screen width if we can't find the real value.  */
#define DEFAULT_SCREEN_WIDTH 80

/* Minimum screen width we'll try to work with.  If this is too small,
   create_image will overflow the buffer.  */
#define MINIMUM_SCREEN_WIDTH 45

static int screen_width = DEFAULT_SCREEN_WIDTH;

struct bar_progress {
  long initial_length;		/* how many bytes have been downloaded
				   previously. */
  long total_length;		/* expected total byte count when the
				   download finishes */
  long count;			/* bytes downloaded so far */

  long last_update;		/* time of the last screen update. */

  int width;			/* screen width we're using at the
				   time the progress gauge was
				   created.  this is different from
				   the screen_width global variable in
				   that the latter can be changed by a
				   signal. */
  char *buffer;			/* buffer where the bar "image" is
				   stored. */
  int tick;
};

static void create_image (struct bar_progress *, long);
static void display_image(char *);

static void *
bar_create (long initial, long total)
{
  struct bar_progress *bp = xmalloc (sizeof (struct bar_progress));

  memset (bp, 0, sizeof (*bp));

  /* In theory, our callers should take care of this pathological
     case, but it can sometimes happen. */
  if (initial > total)
    total = initial;

  bp->initial_length = initial;
  bp->total_length   = total;

  /* - 1 because we don't want to use the last screen column. */
  bp->width = screen_width - 1;
  /* + 1 for the terminating zero. */
  bp->buffer = xmalloc (bp->width + 1);
  printf("\n");

  create_image (bp, 0);
  display_image (bp->buffer);

  return bp;
}

static void
bar_update (void *progress, long howmuch, long dltime)
{
  struct bar_progress *bp = progress;
  int force_update = 0;

  bp->count += howmuch;
  if (bp->total_length > 0
      && bp->count + bp->initial_length > bp->total_length)
    /* We could be downloading more than total_length, e.g. when the
       server sends an incorrect Content-Length header.  In that case,
       adjust bp->total_length to the new reality, so that the code in
       create_image() that depends on total size being smaller or
       equal to the expected size doesn't abort.  */
    bp->total_length = bp->initial_length + bp->count;

  if (screen_width - 1 != bp->width)
    {
      bp->width = screen_width - 1;
      printf("%p\n",bp->buffer);
      bp->buffer = xrealloc (bp->buffer, bp->width + 1);
      force_update = 1;
    }

  if (dltime - bp->last_update < 200 && !force_update)
    /* Don't update more often than five times per second. */
    return;

  bp->last_update = dltime;

  create_image (bp, dltime);
  display_image (bp->buffer);
}

static void
bar_finish (void *progress, long dltime)
{
  struct bar_progress *bp = progress;

  if (bp->total_length > 0
      && bp->count + bp->initial_length > bp->total_length)
    /* See bar_update() for explanation. */
    bp->total_length = bp->initial_length + bp->count;

  if (dltime == 0)
    /* If the download was faster than the granularity of the timer,
       fake some output so that we don't get the ugly "----.--" rate
       at the download finish.  */
    dltime = 1;

  create_image (bp, dltime);
  display_image (bp->buffer);

  printf("\n\n");

  xfree (bp->buffer);
  xfree (bp);
}

#define APPEND_LITERAL(s) do {			\
  memcpy (p, s, sizeof (s) - 1);		\
  p += sizeof (s) - 1;				\
} while (0)

#ifndef MAX
# define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

static void
create_image (struct bar_progress *bp, long dltime)
{
  char *p = bp->buffer;
  long size = bp->initial_length + bp->count;

  char *size_legible = legible (size);
  int size_legible_len = strlen (size_legible);

  /* The progress bar should look like this:
     xx% [=======>             ] nn,nnn 12.34K/s ETA 00:00

     Calculate the geometry.  The idea is to assign as much room as
     possible to the progress bar.  The other idea is to never let
     things "jitter", i.e. pad elements that vary in size so that
     their variance does not affect the placement of other elements.
     It would be especially bad for the progress bar to be resized
     randomly.

     "xx% " or "100%"  - percentage               - 4 chars
     "[]"              - progress bar decorations - 2 chars
     " nnn,nnn,nnn"    - downloaded bytes         - 12 chars or very rarely more
     " 1012.56K/s"     - dl rate                  - 11 chars
     " ETA xx:xx:xx"   - ETA                      - 13 chars

     "=====>..."       - progress bar             - the rest
  */
  int dlbytes_size = 1 + MAX (size_legible_len, 11);
  int progress_size = bp->width - (4 + 2 + dlbytes_size + 11 + 13);

  if (progress_size < 5)
    progress_size = 0;

  /* "xx% " */
  if (bp->total_length > 0)
    {
      int percentage = (int)(100.0 * size / bp->total_length);

      assert (percentage <= 100);

      if (percentage < 100)
	sprintf (p, "%2d%% ", percentage);
      else
	strcpy (p, "100%");
      p += 4;
    }
  else
    APPEND_LITERAL ("    ");

  /* The progress bar: "[====>      ]" */
  if (progress_size && bp->total_length > 0)
    {
      double fraction = (double)size / bp->total_length;
      int dlsz = (int)(fraction * progress_size);
      char *begin;

      assert (dlsz <= progress_size);

      *p++ = '[';
      begin = p;

      if (dlsz > 0)
	{
	  /* Draw dlsz-1 '=' chars and one arrow char.  */
	  while (dlsz-- > 1)
	    *p++ = '=';
	  *p++ = '>';
	}

      while (p - begin < progress_size)
	*p++ = ' ';

      *p++ = ']';
    }
  else if (progress_size)
    {
      /* If we can't draw a real progress bar, then at least show
	 *something* to the user.  */
      int ind = bp->tick % (progress_size * 2 - 6);
      int i, pos;

      /* Make the star move in two directions. */
      if (ind < progress_size - 2)
	pos = ind + 1;
      else
	pos = progress_size - (ind - progress_size + 5);

      *p++ = '[';
      for (i = 0; i < progress_size; i++)
	{
	  if      (i == pos - 1) *p++ = '<';
	  else if (i == pos    ) *p++ = '=';
	  else if (i == pos + 1) *p++ = '>';
	  else
	    *p++ = ' ';
	}
      *p++ = ']';

      ++bp->tick;
    }

  /* " 234,567,890" */
  sprintf (p, " %-11s", legible (size));
  p += strlen (p);

  /* " 1012.45K/s" */
  if (dltime && bp->count)
    {
      static char *short_units[] = { "B/s", "K/s", "M/s", "G/s" };
      int units = 0;
      double dlrate = calc_rate (bp->count, dltime, &units);
      sprintf (p, " %7.2f%s", dlrate, short_units[units]);
      p += strlen (p);
    }
  else
    APPEND_LITERAL ("   --.--K/s");

  /* " ETA xx:xx:xx" */
  if (bp->total_length > 0 && bp->count > 0)
    {
      int eta, eta_hrs, eta_min, eta_sec;
      double tm_sofar = (double)dltime / 1000;
      long bytes_remaining = bp->total_length - size;

      eta = (int) (tm_sofar * bytes_remaining / bp->count);

      eta_hrs = eta / 3600, eta %= 3600;
      eta_min = eta / 60,   eta %= 60;
      eta_sec = eta;

      /* Pad until the end of screen.  The padding is dependent on the
	 hour value.  */
      if (eta_hrs == 0 || eta_hrs > 99)
	/* Hours not printed: pad with three spaces (two digits and
	   colon). */
	APPEND_LITERAL ("   ");
      else if (eta_hrs < 10)
	/* Hours printed with one digit: pad with one space. */
	*p++ = ' ';
      else
	/* Hours printed with two digits: we're using maximum width,
	   don't pad. */
	;

      APPEND_LITERAL (" ETA ");

      if (eta_hrs > 99)
	/* Bogus value, probably due to a calculation overflow.  Print
	   something safe to avoid overstepping the buffer bounds. */
	sprintf (p, "--:--");
      else if (eta_hrs > 0)
	sprintf (p, "%d:%02d:%02d", eta_hrs, eta_min, eta_sec);
      else
	sprintf (p, "%02d:%02d", eta_min, eta_sec);
      p += strlen (p);
    }
  else if (bp->total_length > 0)
    APPEND_LITERAL ("    ETA --:--");

  assert (p - bp->buffer <= bp->width);

  while (p < bp->buffer + bp->width)
    *p++ = ' ';
  *p = '\0';
}

/* Determine the width of the terminal we're running on.  If that's
   not possible, return 0.  */

int
determine_screen_width (void)
{
  /* If there's a way to get the terminal size using POSIX
     tcgetattr(), somebody please tell me.  */
#ifndef TIOCGWINSZ
  return DEFAULT_SCREEN_WIDTH;
#else  /* TIOCGWINSZ */
  int fd;
  struct winsize wsz;

  fd = fileno (stderr);
  if (ioctl (fd, TIOCGWINSZ, &wsz) < 0)
    return DEFAULT_SCREEN_WIDTH;       /* most likely ENOTTY */

  return wsz.ws_col;
#endif /* TIOCGWINSZ */
}

/* Print the contents of the buffer as a one-line ASCII "image" so
   that it can be overwritten next time.  */

static void
display_image (char *buf)
{
  printf("\r");
  printf(buf);
}

static void
bar_set_params (const char *params)
{
  int sw;

  sw = determine_screen_width ();
  if (sw && sw >= MINIMUM_SCREEN_WIDTH)
    screen_width = sw;
  else 
    screen_width = DEFAULT_SCREEN_WIDTH;
}

#ifdef SIGWINCH
RETSIGTYPE
progress_handle_sigwinch (int sig)
{
  int sw = determine_screen_width ();
  if (sw && sw >= MINIMUM_SCREEN_WIDTH)
    screen_width = sw;
  signal (SIGWINCH, progress_handle_sigwinch);
}
#endif

/* Begining of Juam's Code
 *
 * This sucks, it just a wrapper to get libcurl progress function work with 
 * wget's progress functions
 */
struct progress
{	void *data;
	void *callback;
	struct wget_timer *timer;
	long dllast;
};

struct progress *
new_progress_callback(void *callback)
{	struct progress *progress;

	progress = xmalloc(sizeof(*progress));
	if( progress )
	{	memset(progress,0,sizeof(*progress));
		progress->callback = callback;
		progress->timer = wtimer_new();
	}
	
	return progress;
}

void 
destroy_progress_callback(struct progress *progress)
{	
	if( progress )
	{	if( progress->callback )
		{	if( progress->callback == dot_progress_callback )
				dot_finish(progress->data, 
				           wtimer_elapsed(progress->timer));
			else if ( progress->callback == bar_progress_callback )
				bar_finish(progress->data,
				           wtimer_elapsed(progress->timer));
			else
				assert(0);
		}
		wtimer_delete(progress->timer);
	}
	
	xfree(progress);
}

int 
dot_progress_callback(struct progress *progress,
                      double dltotal, double dlnow,
                      double ultotal, double ulnow)
{ 	assert(ultotal==0);
	assert(ulnow==0);
 	assert(progress->dllast<= dlnow);

	if( progress->data == NULL )
 	{ 	progress->data = dot_create(0L,(long)dltotal);
		progress->dllast = 0;
		bar_set_params(NULL);
	}
	if(  ((struct dot_progress *)progress->data)->total_length != dltotal )
		((struct dot_progress *)progress->data)->total_length = dltotal;

	dot_update(progress->data, ((long)dlnow) - progress->dllast,
	                                  wtimer_elapsed(progress->timer));

	progress->dllast = (long) dlnow;
	fflush(stdout);
	return 0;
}

int
bar_progress_callback( struct progress *progress,
                      double dltotal, double dlnow,
                      double ultotal, double ulnow)
{/* 	assert(ultotal==0);
	assert(ulnow==0);*/
 	assert(progress->dllast<= dlnow);

	if( progress->data == NULL )
 	{ 	progress->data = bar_create(0L,(long)dltotal);
		progress->dllast = (long) 0;
		bar_set_params(NULL);
	}
	if(  ((struct bar_progress *)progress->data)->total_length != dltotal ) 
	  ((struct bar_progress *)progress->data)->total_length = dltotal;

	bar_update(progress->data, ((long)dlnow) - progress->dllast,
	                                   wtimer_elapsed(progress->timer));

	progress->dllast = (long) dlnow;
	fflush(stdout);
	return 0;
}
