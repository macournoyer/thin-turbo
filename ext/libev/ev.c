/*
 * libev event processing core, watcher management
 *
 * Copyright (c) 2007,2008 Marc Alexander Lehmann <libev@schmorp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 * 
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 * 
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* this big block deduces configuration from config.h */
#ifndef EV_STANDALONE
# ifdef EV_CONFIG_H
#  include EV_CONFIG_H
# else
#  include "config.h"
# endif

# if HAVE_CLOCK_GETTIME
#  ifndef EV_USE_MONOTONIC
#   define EV_USE_MONOTONIC 1
#  endif
#  ifndef EV_USE_REALTIME
#   define EV_USE_REALTIME  1
#  endif
# else
#  ifndef EV_USE_MONOTONIC
#   define EV_USE_MONOTONIC 0
#  endif
#  ifndef EV_USE_REALTIME
#   define EV_USE_REALTIME  0
#  endif
# endif

# ifndef EV_USE_NANOSLEEP
#  if HAVE_NANOSLEEP
#   define EV_USE_NANOSLEEP 1
#  else
#   define EV_USE_NANOSLEEP 0
#  endif
# endif

# ifndef EV_USE_SELECT
#  if HAVE_SELECT && HAVE_SYS_SELECT_H
#   define EV_USE_SELECT 1
#  else
#   define EV_USE_SELECT 0
#  endif
# endif

# ifndef EV_USE_POLL
#  if HAVE_POLL && HAVE_POLL_H
#   define EV_USE_POLL 1
#  else
#   define EV_USE_POLL 0
#  endif
# endif
   
# ifndef EV_USE_EPOLL
#  if HAVE_EPOLL_CTL && HAVE_SYS_EPOLL_H
#   define EV_USE_EPOLL 1
#  else
#   define EV_USE_EPOLL 0
#  endif
# endif
   
# ifndef EV_USE_KQUEUE
#  if HAVE_KQUEUE && HAVE_SYS_EVENT_H && HAVE_SYS_QUEUE_H
#   define EV_USE_KQUEUE 1
#  else
#   define EV_USE_KQUEUE 0
#  endif
# endif
   
# ifndef EV_USE_PORT
#  if HAVE_PORT_H && HAVE_PORT_CREATE
#   define EV_USE_PORT 1
#  else
#   define EV_USE_PORT 0
#  endif
# endif

# ifndef EV_USE_INOTIFY
#  if HAVE_INOTIFY_INIT && HAVE_SYS_INOTIFY_H
#   define EV_USE_INOTIFY 1
#  else
#   define EV_USE_INOTIFY 0
#  endif
# endif

# ifndef EV_USE_EVENTFD
#  if HAVE_EVENTFD
#   define EV_USE_EVENTFD 1
#  else
#   define EV_USE_EVENTFD 0
#  endif
# endif
 
#endif

#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stddef.h>

#include <stdio.h>

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#include <signal.h>

#ifdef EV_H
# include EV_H
#else
# include "ev.h"
#endif

#ifndef _WIN32
# include <sys/time.h>
# include <sys/wait.h>
# include <unistd.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# ifndef EV_SELECT_IS_WINSOCKET
#  define EV_SELECT_IS_WINSOCKET 1
# endif
#endif

/* this block tries to deduce configuration from header-defined symbols and defaults */

#ifndef EV_USE_MONOTONIC
# define EV_USE_MONOTONIC 0
#endif

#ifndef EV_USE_REALTIME
# define EV_USE_REALTIME 0
#endif

#ifndef EV_USE_NANOSLEEP
# define EV_USE_NANOSLEEP 0
#endif

#ifndef EV_USE_SELECT
# define EV_USE_SELECT 1
#endif

#ifndef EV_USE_POLL
# ifdef _WIN32
#  define EV_USE_POLL 0
# else
#  define EV_USE_POLL 1
# endif
#endif

#ifndef EV_USE_EPOLL
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4))
#  define EV_USE_EPOLL 1
# else
#  define EV_USE_EPOLL 0
# endif
#endif

#ifndef EV_USE_KQUEUE
# define EV_USE_KQUEUE 0
#endif

#ifndef EV_USE_PORT
# define EV_USE_PORT 0
#endif

#ifndef EV_USE_INOTIFY
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4))
#  define EV_USE_INOTIFY 1
# else
#  define EV_USE_INOTIFY 0
# endif
#endif

#ifndef EV_PID_HASHSIZE
# if EV_MINIMAL
#  define EV_PID_HASHSIZE 1
# else
#  define EV_PID_HASHSIZE 16
# endif
#endif

#ifndef EV_INOTIFY_HASHSIZE
# if EV_MINIMAL
#  define EV_INOTIFY_HASHSIZE 1
# else
#  define EV_INOTIFY_HASHSIZE 16
# endif
#endif

#ifndef EV_USE_EVENTFD
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 7))
#  define EV_USE_EVENTFD 1
# else
#  define EV_USE_EVENTFD 0
# endif
#endif

#if 0 /* debugging */
# define EV_VERIFY 3
# define EV_USE_4HEAP 1
# define EV_HEAP_CACHE_AT 1
#endif

#ifndef EV_VERIFY
# define EV_VERIFY !EV_MINIMAL
#endif

#ifndef EV_USE_4HEAP
# define EV_USE_4HEAP !EV_MINIMAL
#endif

#ifndef EV_HEAP_CACHE_AT
# define EV_HEAP_CACHE_AT !EV_MINIMAL
#endif

/* this block fixes any misconfiguration where we know we run into trouble otherwise */

#ifndef CLOCK_MONOTONIC
# undef EV_USE_MONOTONIC
# define EV_USE_MONOTONIC 0
#endif

#ifndef CLOCK_REALTIME
# undef EV_USE_REALTIME
# define EV_USE_REALTIME 0
#endif

#if !EV_STAT_ENABLE
# undef EV_USE_INOTIFY
# define EV_USE_INOTIFY 0
#endif

#if !EV_USE_NANOSLEEP
# ifndef _WIN32
#  include <sys/select.h>
# endif
#endif

#if EV_USE_INOTIFY
# include <sys/inotify.h>
#endif

#if EV_SELECT_IS_WINSOCKET
# include <winsock.h>
#endif

#if EV_USE_EVENTFD
/* our minimum requirement is glibc 2.7 which has the stub, but not the header */
# include <stdint.h>
# ifdef __cplusplus
extern "C" {
# endif
int eventfd (unsigned int initval, int flags);
# ifdef __cplusplus
}
# endif
#endif

/**/

#if EV_VERIFY >= 3
# define EV_FREQUENT_CHECK ev_loop_verify (EV_A)
#else
# define EV_FREQUENT_CHECK do { } while (0)
#endif

/*
 * This is used to avoid floating point rounding problems.
 * It is added to ev_rt_now when scheduling periodics
 * to ensure progress, time-wise, even when rounding
 * errors are against us.
 * This value is good at least till the year 4000.
 * Better solutions welcome.
 */
#define TIME_EPSILON  0.0001220703125 /* 1/8192 */

#define MIN_TIMEJUMP  1. /* minimum timejump that gets detected (if monotonic clock available) */
#define MAX_BLOCKTIME 59.743 /* never wait longer than this time (to detect time jumps) */
/*#define CLEANUP_INTERVAL (MAX_BLOCKTIME * 5.) /* how often to try to free memory and re-check fds, TODO */

#if __GNUC__ >= 4
# define expect(expr,value)         __builtin_expect ((expr),(value))
# define noinline                   __attribute__ ((noinline))
#else
# define expect(expr,value)         (expr)
# define noinline
# if __STDC_VERSION__ < 199901L && __GNUC__ < 2
#  define inline
# endif
#endif

#define expect_false(expr) expect ((expr) != 0, 0)
#define expect_true(expr)  expect ((expr) != 0, 1)
#define inline_size        static inline

#if EV_MINIMAL
# define inline_speed      static noinline
#else
# define inline_speed      static inline
#endif

#define NUMPRI    (EV_MAXPRI - EV_MINPRI + 1)
#define ABSPRI(w) (((W)w)->priority - EV_MINPRI)

#define EMPTY       /* required for microsofts broken pseudo-c compiler */
#define EMPTY2(a,b) /* used to suppress some warnings */

typedef ev_watcher *W;
typedef ev_watcher_list *WL;
typedef ev_watcher_time *WT;

#define ev_active(w) ((W)(w))->active
#define ev_at(w) ((WT)(w))->at

#if EV_USE_MONOTONIC
/* sig_atomic_t is used to avoid per-thread variables or locking but still */
/* giving it a reasonably high chance of working on typical architetcures */
static EV_ATOMIC_T have_monotonic; /* did clock_gettime (CLOCK_MONOTONIC) work? */
#endif

#ifdef _WIN32
# include "ev_win32.c"
#endif

/*****************************************************************************/

static void (*syserr_cb)(const char *msg);

void
ev_set_syserr_cb (void (*cb)(const char *msg))
{
  syserr_cb = cb;
}

static void noinline
syserr (const char *msg)
{
  if (!msg)
    msg = "(libev) system error";

  if (syserr_cb)
    syserr_cb (msg);
  else
    {
      perror (msg);
      abort ();
    }
}

static void *
ev_realloc_emul (void *ptr, long size)
{
  /* some systems, notably openbsd and darwin, fail to properly
   * implement realloc (x, 0) (as required by both ansi c-98 and
   * the single unix specification, so work around them here.
   */

  if (size)
    return realloc (ptr, size);

  free (ptr);
  return 0;
}

static void *(*alloc)(void *ptr, long size) = ev_realloc_emul;

void
ev_set_allocator (void *(*cb)(void *ptr, long size))
{
  alloc = cb;
}

inline_speed void *
ev_realloc (void *ptr, long size)
{
  ptr = alloc (ptr, size);

  if (!ptr && size)
    {
      fprintf (stderr, "libev: cannot allocate %ld bytes, aborting.", size);
      abort ();
    }

  return ptr;
}

#define ev_malloc(size) ev_realloc (0, (size))
#define ev_free(ptr)    ev_realloc ((ptr), 0)

/*****************************************************************************/

typedef struct
{
  WL head;
  unsigned char events;
  unsigned char reify;
#if EV_SELECT_IS_WINSOCKET
  SOCKET handle;
#endif
} ANFD;

typedef struct
{
  W w;
  int events;
} ANPENDING;

#if EV_USE_INOTIFY
/* hash table entry per inotify-id */
typedef struct
{
  WL head;
} ANFS;
#endif

/* Heap Entry */
#if EV_HEAP_CACHE_AT
  typedef struct {
    ev_tstamp at;
    WT w;
  } ANHE;

  #define ANHE_w(he)        (he).w     /* access watcher, read-write */
  #define ANHE_at(he)       (he).at    /* access cached at, read-only */
  #define ANHE_at_cache(he) (he).at = (he).w->at /* update at from watcher */
#else
  typedef WT ANHE;

  #define ANHE_w(he)        (he)
  #define ANHE_at(he)       (he)->at
  #define ANHE_at_cache(he)
#endif

#if EV_MULTIPLICITY

  struct ev_loop
  {
    ev_tstamp ev_rt_now;
    #define ev_rt_now ((loop)->ev_rt_now)
    #define VAR(name,decl) decl;
      #include "ev_vars.h"
    #undef VAR
  };
  #include "ev_wrap.h"

  static struct ev_loop default_loop_struct;
  struct ev_loop *ev_default_loop_ptr;

#else

  ev_tstamp ev_rt_now;
  #define VAR(name,decl) static decl;
    #include "ev_vars.h"
  #undef VAR

  static int ev_default_loop_ptr;

#endif

/*****************************************************************************/

ev_tstamp
ev_time (void)
{
#if EV_USE_REALTIME
  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
#else
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + tv.tv_usec * 1e-6;
#endif
}

ev_tstamp inline_size
get_clock (void)
{
#if EV_USE_MONOTONIC
  if (expect_true (have_monotonic))
    {
      struct timespec ts;
      clock_gettime (CLOCK_MONOTONIC, &ts);
      return ts.tv_sec + ts.tv_nsec * 1e-9;
    }
#endif

  return ev_time ();
}

#if EV_MULTIPLICITY
ev_tstamp
ev_now (EV_P)
{
  return ev_rt_now;
}
#endif

void
ev_sleep (ev_tstamp delay)
{
  if (delay > 0.)
    {
#if EV_USE_NANOSLEEP
      struct timespec ts;

      ts.tv_sec  = (time_t)delay;
      ts.tv_nsec = (long)((delay - (ev_tstamp)(ts.tv_sec)) * 1e9);

      nanosleep (&ts, 0);
#elif defined(_WIN32)
      Sleep ((unsigned long)(delay * 1e3));
#else
      struct timeval tv;

      tv.tv_sec  = (time_t)delay;
      tv.tv_usec = (long)((delay - (ev_tstamp)(tv.tv_sec)) * 1e6);

      select (0, 0, 0, 0, &tv);
#endif
    }
}

/*****************************************************************************/

#define MALLOC_ROUND 4096 /* prefer to allocate in chunks of this size, must be 2**n and >> 4 longs */

int inline_size
array_nextsize (int elem, int cur, int cnt)
{
  int ncur = cur + 1;

  do
    ncur <<= 1;
  while (cnt > ncur);

  /* if size is large, round to MALLOC_ROUND - 4 * longs to accomodate malloc overhead */
  if (elem * ncur > MALLOC_ROUND - sizeof (void *) * 4)
    {
      ncur *= elem;
      ncur = (ncur + elem + (MALLOC_ROUND - 1) + sizeof (void *) * 4) & ~(MALLOC_ROUND - 1);
      ncur = ncur - sizeof (void *) * 4;
      ncur /= elem;
    }

  return ncur;
}

static noinline void *
array_realloc (int elem, void *base, int *cur, int cnt)
{
  *cur = array_nextsize (elem, *cur, cnt);
  return ev_realloc (base, elem * *cur);
}

#define array_needsize(type,base,cur,cnt,init)			\
  if (expect_false ((cnt) > (cur)))				\
    {								\
      int ocur_ = (cur);					\
      (base) = (type *)array_realloc				\
         (sizeof (type), (base), &(cur), (cnt));		\
      init ((base) + (ocur_), (cur) - ocur_);			\
    }

#if 0
#define array_slim(type,stem)					\
  if (stem ## max < array_roundsize (stem ## cnt >> 2))		\
    {								\
      stem ## max = array_roundsize (stem ## cnt >> 1);		\
      base = (type *)ev_realloc (base, sizeof (type) * (stem ## max));\
      fprintf (stderr, "slimmed down " # stem " to %d\n", stem ## max);/*D*/\
    }
#endif

#define array_free(stem, idx) \
  ev_free (stem ## s idx); stem ## cnt idx = stem ## max idx = 0;

/*****************************************************************************/

void noinline
ev_feed_event (EV_P_ void *w, int revents)
{
  W w_ = (W)w;
  int pri = ABSPRI (w_);

  if (expect_false (w_->pending))
    pendings [pri][w_->pending - 1].events |= revents;
  else
    {
      w_->pending = ++pendingcnt [pri];
      array_needsize (ANPENDING, pendings [pri], pendingmax [pri], w_->pending, EMPTY2);
      pendings [pri][w_->pending - 1].w      = w_;
      pendings [pri][w_->pending - 1].events = revents;
    }
}

void inline_speed
queue_events (EV_P_ W *events, int eventcnt, int type)
{
  int i;

  for (i = 0; i < eventcnt; ++i)
    ev_feed_event (EV_A_ events [i], type);
}

/*****************************************************************************/

void inline_size
anfds_init (ANFD *base, int count)
{
  while (count--)
    {
      base->head   = 0;
      base->events = EV_NONE;
      base->reify  = 0;

      ++base;
    }
}

void inline_speed
fd_event (EV_P_ int fd, int revents)
{
  ANFD *anfd = anfds + fd;
  ev_io *w;

  for (w = (ev_io *)anfd->head; w; w = (ev_io *)((WL)w)->next)
    {
      int ev = w->events & revents;

      if (ev)
        ev_feed_event (EV_A_ (W)w, ev);
    }
}

void
ev_feed_fd_event (EV_P_ int fd, int revents)
{
  if (fd >= 0 && fd < anfdmax)
    fd_event (EV_A_ fd, revents);
}

void inline_size
fd_reify (EV_P)
{
  int i;

  for (i = 0; i < fdchangecnt; ++i)
    {
      int fd = fdchanges [i];
      ANFD *anfd = anfds + fd;
      ev_io *w;

      unsigned char events = 0;

      for (w = (ev_io *)anfd->head; w; w = (ev_io *)((WL)w)->next)
        events |= (unsigned char)w->events;

#if EV_SELECT_IS_WINSOCKET
      if (events)
        {
          unsigned long argp;
          #ifdef EV_FD_TO_WIN32_HANDLE
            anfd->handle = EV_FD_TO_WIN32_HANDLE (fd);
          #else
            anfd->handle = _get_osfhandle (fd);
          #endif
          assert (("libev only supports socket fds in this configuration", ioctlsocket (anfd->handle, FIONREAD, &argp) == 0));
        }
#endif

      {
        unsigned char o_events = anfd->events;
        unsigned char o_reify  = anfd->reify;

        anfd->reify  = 0;
        anfd->events = events;

        if (o_events != events || o_reify & EV_IOFDSET)
          backend_modify (EV_A_ fd, o_events, events);
      }
    }

  fdchangecnt = 0;
}

void inline_size
fd_change (EV_P_ int fd, int flags)
{
  unsigned char reify = anfds [fd].reify;
  anfds [fd].reify |= flags;

  if (expect_true (!reify))
    {
      ++fdchangecnt;
      array_needsize (int, fdchanges, fdchangemax, fdchangecnt, EMPTY2);
      fdchanges [fdchangecnt - 1] = fd;
    }
}

void inline_speed
fd_kill (EV_P_ int fd)
{
  ev_io *w;

  while ((w = (ev_io *)anfds [fd].head))
    {
      ev_io_stop (EV_A_ w);
      ev_feed_event (EV_A_ (W)w, EV_ERROR | EV_READ | EV_WRITE);
    }
}

int inline_size
fd_valid (int fd)
{
#ifdef _WIN32
  return _get_osfhandle (fd) != -1;
#else
  return fcntl (fd, F_GETFD) != -1;
#endif
}

/* called on EBADF to verify fds */
static void noinline
fd_ebadf (EV_P)
{
  int fd;

  for (fd = 0; fd < anfdmax; ++fd)
    if (anfds [fd].events)
      if (!fd_valid (fd) == -1 && errno == EBADF)
        fd_kill (EV_A_ fd);
}

/* called on ENOMEM in select/poll to kill some fds and retry */
static void noinline
fd_enomem (EV_P)
{
  int fd;

  for (fd = anfdmax; fd--; )
    if (anfds [fd].events)
      {
        fd_kill (EV_A_ fd);
        return;
      }
}

/* usually called after fork if backend needs to re-arm all fds from scratch */
static void noinline
fd_rearm_all (EV_P)
{
  int fd;

  for (fd = 0; fd < anfdmax; ++fd)
    if (anfds [fd].events)
      {
        anfds [fd].events = 0;
        fd_change (EV_A_ fd, EV_IOFDSET | 1);
      }
}

/*****************************************************************************/

/*
 * the heap functions want a real array index. array index 0 uis guaranteed to not
 * be in-use at any time. the first heap entry is at array [HEAP0]. DHEAP gives
 * the branching factor of the d-tree.
 */

/*
 * at the moment we allow libev the luxury of two heaps,
 * a small-code-size 2-heap one and a ~1.5kb larger 4-heap
 * which is more cache-efficient.
 * the difference is about 5% with 50000+ watchers.
 */
#if EV_USE_4HEAP

#define DHEAP 4
#define HEAP0 (DHEAP - 1) /* index of first element in heap */
#define HPARENT(k) ((((k) - HEAP0 - 1) / DHEAP) + HEAP0)
#define UPHEAP_DONE(p,k) ((p) == (k))

/* away from the root */
void inline_speed
downheap (ANHE *heap, int N, int k)
{
  ANHE he = heap [k];
  ANHE *E = heap + N + HEAP0;

  for (;;)
    {
      ev_tstamp minat;
      ANHE *minpos;
      ANHE *pos = heap + DHEAP * (k - HEAP0) + HEAP0 + 1;

      /* find minimum child */
      if (expect_true (pos + DHEAP - 1 < E))
        {
          /* fast path */                               (minpos = pos + 0), (minat = ANHE_at (*minpos));
          if (               ANHE_at (pos [1]) < minat) (minpos = pos + 1), (minat = ANHE_at (*minpos));
          if (               ANHE_at (pos [2]) < minat) (minpos = pos + 2), (minat = ANHE_at (*minpos));
          if (               ANHE_at (pos [3]) < minat) (minpos = pos + 3), (minat = ANHE_at (*minpos));
        }
      else if (pos < E)
        {
          /* slow path */                               (minpos = pos + 0), (minat = ANHE_at (*minpos));
          if (pos + 1 < E && ANHE_at (pos [1]) < minat) (minpos = pos + 1), (minat = ANHE_at (*minpos));
          if (pos + 2 < E && ANHE_at (pos [2]) < minat) (minpos = pos + 2), (minat = ANHE_at (*minpos));
          if (pos + 3 < E && ANHE_at (pos [3]) < minat) (minpos = pos + 3), (minat = ANHE_at (*minpos));
        }
      else
        break;

      if (ANHE_at (he) <= minat)
        break;

      heap [k] = *minpos;
      ev_active (ANHE_w (*minpos)) = k;

      k = minpos - heap;
    }

  heap [k] = he;
  ev_active (ANHE_w (he)) = k;
}

#else /* 4HEAP */

#define HEAP0 1
#define HPARENT(k) ((k) >> 1)
#define UPHEAP_DONE(p,k) (!(p))

/* away from the root */
void inline_speed
downheap (ANHE *heap, int N, int k)
{
  ANHE he = heap [k];

  for (;;)
    {
      int c = k << 1;

      if (c > N + HEAP0 - 1)
        break;

      c += c + 1 < N + HEAP0 && ANHE_at (heap [c]) > ANHE_at (heap [c + 1])
           ? 1 : 0;

      if (ANHE_at (he) <= ANHE_at (heap [c]))
        break;

      heap [k] = heap [c];
      ev_active (ANHE_w (heap [k])) = k;
      
      k = c;
    }

  heap [k] = he;
  ev_active (ANHE_w (he)) = k;
}
#endif

/* towards the root */
void inline_speed
upheap (ANHE *heap, int k)
{
  ANHE he = heap [k];

  for (;;)
    {
      int p = HPARENT (k);

      if (UPHEAP_DONE (p, k) || ANHE_at (heap [p]) <= ANHE_at (he))
        break;

      heap [k] = heap [p];
      ev_active (ANHE_w (heap [k])) = k;
      k = p;
    }

  heap [k] = he;
  ev_active (ANHE_w (he)) = k;
}

void inline_size
adjustheap (ANHE *heap, int N, int k)
{
  if (k > HEAP0 && ANHE_at (heap [HPARENT (k)]) >= ANHE_at (heap [k]))
    upheap (heap, k);
  else
    downheap (heap, N, k);
}

/* rebuild the heap: this function is used only once and executed rarely */
void inline_size
reheap (ANHE *heap, int N)
{
  int i;

  /* we don't use floyds algorithm, upheap is simpler and is more cache-efficient */
  /* also, this is easy to implement and correct for both 2-heaps and 4-heaps */
  for (i = 0; i < N; ++i)
    upheap (heap, i + HEAP0);
}

/*****************************************************************************/

typedef struct
{
  WL head;
  EV_ATOMIC_T gotsig;
} ANSIG;

static ANSIG *signals;
static int signalmax;

static EV_ATOMIC_T gotsig;

void inline_size
signals_init (ANSIG *base, int count)
{
  while (count--)
    {
      base->head   = 0;
      base->gotsig = 0;

      ++base;
    }
}

/*****************************************************************************/

void inline_speed
fd_intern (int fd)
{
#ifdef _WIN32
  int arg = 1;
  ioctlsocket (_get_osfhandle (fd), FIONBIO, &arg);
#else
  fcntl (fd, F_SETFD, FD_CLOEXEC);
  fcntl (fd, F_SETFL, O_NONBLOCK);
#endif
}

static void noinline
evpipe_init (EV_P)
{
  if (!ev_is_active (&pipeev))
    {
#if EV_USE_EVENTFD
      if ((evfd = eventfd (0, 0)) >= 0)
        {
          evpipe [0] = -1;
          fd_intern (evfd);
          ev_io_set (&pipeev, evfd, EV_READ);
        }
      else
#endif
        {
          while (pipe (evpipe))
            syserr ("(libev) error creating signal/async pipe");

          fd_intern (evpipe [0]);
          fd_intern (evpipe [1]);
          ev_io_set (&pipeev, evpipe [0], EV_READ);
        }

      ev_io_start (EV_A_ &pipeev);
      ev_unref (EV_A); /* watcher should not keep loop alive */
    }
}

void inline_size
evpipe_write (EV_P_ EV_ATOMIC_T *flag)
{
  if (!*flag)
    {
      int old_errno = errno; /* save errno because write might clobber it */

      *flag = 1;

#if EV_USE_EVENTFD
      if (evfd >= 0)
        {
          uint64_t counter = 1;
          write (evfd, &counter, sizeof (uint64_t));
        }
      else
#endif
        write (evpipe [1], &old_errno, 1);

      errno = old_errno;
    }
}

static void
pipecb (EV_P_ ev_io *iow, int revents)
{
#if EV_USE_EVENTFD
  if (evfd >= 0)
    {
      uint64_t counter;
      read (evfd, &counter, sizeof (uint64_t));
    }
  else
#endif
    {
      char dummy;
      read (evpipe [0], &dummy, 1);
    }

  if (gotsig && ev_is_default_loop (EV_A))
    {    
      int signum;
      gotsig = 0;

      for (signum = signalmax; signum--; )
        if (signals [signum].gotsig)
          ev_feed_signal_event (EV_A_ signum + 1);
    }

#if EV_ASYNC_ENABLE
  if (gotasync)
    {
      int i;
      gotasync = 0;

      for (i = asynccnt; i--; )
        if (asyncs [i]->sent)
          {
            asyncs [i]->sent = 0;
            ev_feed_event (EV_A_ asyncs [i], EV_ASYNC);
          }
    }
#endif
}

/*****************************************************************************/

static void
ev_sighandler (int signum)
{
#if EV_MULTIPLICITY
  struct ev_loop *loop = &default_loop_struct;
#endif

#if _WIN32
  signal (signum, ev_sighandler);
#endif

  signals [signum - 1].gotsig = 1;
  evpipe_write (EV_A_ &gotsig);
}

void noinline
ev_feed_signal_event (EV_P_ int signum)
{
  WL w;

#if EV_MULTIPLICITY
  assert (("feeding signal events is only supported in the default loop", loop == ev_default_loop_ptr));
#endif

  --signum;

  if (signum < 0 || signum >= signalmax)
    return;

  signals [signum].gotsig = 0;

  for (w = signals [signum].head; w; w = w->next)
    ev_feed_event (EV_A_ (W)w, EV_SIGNAL);
}

/*****************************************************************************/

static WL childs [EV_PID_HASHSIZE];

#ifndef _WIN32

static ev_signal childev;

#ifndef WIFCONTINUED
# define WIFCONTINUED(status) 0
#endif

void inline_speed
child_reap (EV_P_ int chain, int pid, int status)
{
  ev_child *w;
  int traced = WIFSTOPPED (status) || WIFCONTINUED (status);

  for (w = (ev_child *)childs [chain & (EV_PID_HASHSIZE - 1)]; w; w = (ev_child *)((WL)w)->next)
    {
      if ((w->pid == pid || !w->pid)
          && (!traced || (w->flags & 1)))
        {
          ev_set_priority (w, EV_MAXPRI); /* need to do it *now*, this *must* be the same prio as the signal watcher itself */
          w->rpid    = pid;
          w->rstatus = status;
          ev_feed_event (EV_A_ (W)w, EV_CHILD);
        }
    }
}

#ifndef WCONTINUED
# define WCONTINUED 0
#endif

static void
childcb (EV_P_ ev_signal *sw, int revents)
{
  int pid, status;

  /* some systems define WCONTINUED but then fail to support it (linux 2.4) */
  if (0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED | WCONTINUED)))
    if (!WCONTINUED
        || errno != EINVAL
        || 0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED)))
      return;

  /* make sure we are called again until all children have been reaped */
  /* we need to do it this way so that the callback gets called before we continue */
  ev_feed_event (EV_A_ (W)sw, EV_SIGNAL);

  child_reap (EV_A_ pid, pid, status);
  if (EV_PID_HASHSIZE > 1)
    child_reap (EV_A_ 0, pid, status); /* this might trigger a watcher twice, but feed_event catches that */
}

#endif

/*****************************************************************************/

#if EV_USE_PORT
# include "ev_port.c"
#endif
#if EV_USE_KQUEUE
# include "ev_kqueue.c"
#endif
#if EV_USE_EPOLL
# include "ev_epoll.c"
#endif
#if EV_USE_POLL
# include "ev_poll.c"
#endif
#if EV_USE_SELECT
# include "ev_select.c"
#endif

int
ev_version_major (void)
{
  return EV_VERSION_MAJOR;
}

int
ev_version_minor (void)
{
  return EV_VERSION_MINOR;
}

/* return true if we are running with elevated privileges and should ignore env variables */
int inline_size
enable_secure (void)
{
#ifdef _WIN32
  return 0;
#else
  return getuid () != geteuid ()
      || getgid () != getegid ();
#endif
}

unsigned int
ev_supported_backends (void)
{
  unsigned int flags = 0;

  if (EV_USE_PORT  ) flags |= EVBACKEND_PORT;
  if (EV_USE_KQUEUE) flags |= EVBACKEND_KQUEUE;
  if (EV_USE_EPOLL ) flags |= EVBACKEND_EPOLL;
  if (EV_USE_POLL  ) flags |= EVBACKEND_POLL;
  if (EV_USE_SELECT) flags |= EVBACKEND_SELECT;
  
  return flags;
}

unsigned int
ev_recommended_backends (void)
{
  unsigned int flags = ev_supported_backends ();

#ifndef __NetBSD__
  /* kqueue is borked on everything but netbsd apparently */
  /* it usually doesn't work correctly on anything but sockets and pipes */
  flags &= ~EVBACKEND_KQUEUE;
#endif
#ifdef __APPLE__
  // flags &= ~EVBACKEND_KQUEUE; for documentation
  flags &= ~EVBACKEND_POLL;
#endif

  return flags;
}

unsigned int
ev_embeddable_backends (void)
{
  int flags = EVBACKEND_EPOLL | EVBACKEND_KQUEUE | EVBACKEND_PORT;

  /* epoll embeddability broken on all linux versions up to at least 2.6.23 */
  /* please fix it and tell me how to detect the fix */
  flags &= ~EVBACKEND_EPOLL;

  return flags;
}

unsigned int
ev_backend (EV_P)
{
  return backend;
}

unsigned int
ev_loop_count (EV_P)
{
  return loop_count;
}

void
ev_set_io_collect_interval (EV_P_ ev_tstamp interval)
{
  io_blocktime = interval;
}

void
ev_set_timeout_collect_interval (EV_P_ ev_tstamp interval)
{
  timeout_blocktime = interval;
}

static void noinline
loop_init (EV_P_ unsigned int flags)
{
  if (!backend)
    {
#if EV_USE_MONOTONIC
      {
        struct timespec ts;
        if (!clock_gettime (CLOCK_MONOTONIC, &ts))
          have_monotonic = 1;
      }
#endif

      ev_rt_now         = ev_time ();
      mn_now            = get_clock ();
      now_floor         = mn_now;
      rtmn_diff         = ev_rt_now - mn_now;

      io_blocktime      = 0.;
      timeout_blocktime = 0.;
      backend           = 0;
      backend_fd        = -1;
      gotasync          = 0;
#if EV_USE_INOTIFY
      fs_fd             = -2;
#endif

      /* pid check not overridable via env */
#ifndef _WIN32
      if (flags & EVFLAG_FORKCHECK)
        curpid = getpid ();
#endif

      if (!(flags & EVFLAG_NOENV)
          && !enable_secure ()
          && getenv ("LIBEV_FLAGS"))
        flags = atoi (getenv ("LIBEV_FLAGS"));

      if (!(flags & 0x0000ffffU))
        flags |= ev_recommended_backends ();

#if EV_USE_PORT
      if (!backend && (flags & EVBACKEND_PORT  )) backend = port_init   (EV_A_ flags);
#endif
#if EV_USE_KQUEUE
      if (!backend && (flags & EVBACKEND_KQUEUE)) backend = kqueue_init (EV_A_ flags);
#endif
#if EV_USE_EPOLL
      if (!backend && (flags & EVBACKEND_EPOLL )) backend = epoll_init  (EV_A_ flags);
#endif
#if EV_USE_POLL
      if (!backend && (flags & EVBACKEND_POLL  )) backend = poll_init   (EV_A_ flags);
#endif
#if EV_USE_SELECT
      if (!backend && (flags & EVBACKEND_SELECT)) backend = select_init (EV_A_ flags);
#endif

      ev_init (&pipeev, pipecb);
      ev_set_priority (&pipeev, EV_MAXPRI);
    }
}

static void noinline
loop_destroy (EV_P)
{
  int i;

  if (ev_is_active (&pipeev))
    {
      ev_ref (EV_A); /* signal watcher */
      ev_io_stop (EV_A_ &pipeev);

#if EV_USE_EVENTFD
      if (evfd >= 0)
        close (evfd);
#endif

      if (evpipe [0] >= 0)
        {
          close (evpipe [0]);
          close (evpipe [1]);
        }
    }

#if EV_USE_INOTIFY
  if (fs_fd >= 0)
    close (fs_fd);
#endif

  if (backend_fd >= 0)
    close (backend_fd);

#if EV_USE_PORT
  if (backend == EVBACKEND_PORT  ) port_destroy   (EV_A);
#endif
#if EV_USE_KQUEUE
  if (backend == EVBACKEND_KQUEUE) kqueue_destroy (EV_A);
#endif
#if EV_USE_EPOLL
  if (backend == EVBACKEND_EPOLL ) epoll_destroy  (EV_A);
#endif
#if EV_USE_POLL
  if (backend == EVBACKEND_POLL  ) poll_destroy   (EV_A);
#endif
#if EV_USE_SELECT
  if (backend == EVBACKEND_SELECT) select_destroy (EV_A);
#endif

  for (i = NUMPRI; i--; )
    {
      array_free (pending, [i]);
#if EV_IDLE_ENABLE
      array_free (idle, [i]);
#endif
    }

  ev_free (anfds); anfdmax = 0;

  /* have to use the microsoft-never-gets-it-right macro */
  array_free (fdchange, EMPTY);
  array_free (timer, EMPTY);
#if EV_PERIODIC_ENABLE
  array_free (periodic, EMPTY);
#endif
#if EV_FORK_ENABLE
  array_free (fork, EMPTY);
#endif
  array_free (prepare, EMPTY);
  array_free (check, EMPTY);
#if EV_ASYNC_ENABLE
  array_free (async, EMPTY);
#endif

  backend = 0;
}

#if EV_USE_INOTIFY
void inline_size infy_fork (EV_P);
#endif

void inline_size
loop_fork (EV_P)
{
#if EV_USE_PORT
  if (backend == EVBACKEND_PORT  ) port_fork   (EV_A);
#endif
#if EV_USE_KQUEUE
  if (backend == EVBACKEND_KQUEUE) kqueue_fork (EV_A);
#endif
#if EV_USE_EPOLL
  if (backend == EVBACKEND_EPOLL ) epoll_fork  (EV_A);
#endif
#if EV_USE_INOTIFY
  infy_fork (EV_A);
#endif

  if (ev_is_active (&pipeev))
    {
      /* this "locks" the handlers against writing to the pipe */
      /* while we modify the fd vars */
      gotsig = 1;
#if EV_ASYNC_ENABLE
      gotasync = 1;
#endif

      ev_ref (EV_A);
      ev_io_stop (EV_A_ &pipeev);

#if EV_USE_EVENTFD
      if (evfd >= 0)
        close (evfd);
#endif

      if (evpipe [0] >= 0)
        {
          close (evpipe [0]);
          close (evpipe [1]);
        }

      evpipe_init (EV_A);
      /* now iterate over everything, in case we missed something */
      pipecb (EV_A_ &pipeev, EV_READ);
    }

  postfork = 0;
}

#if EV_MULTIPLICITY

struct ev_loop *
ev_loop_new (unsigned int flags)
{
  struct ev_loop *loop = (struct ev_loop *)ev_malloc (sizeof (struct ev_loop));

  memset (loop, 0, sizeof (struct ev_loop));

  loop_init (EV_A_ flags);

  if (ev_backend (EV_A))
    return loop;

  return 0;
}

void
ev_loop_destroy (EV_P)
{
  loop_destroy (EV_A);
  ev_free (loop);
}

void
ev_loop_fork (EV_P)
{
  postfork = 1; /* must be in line with ev_default_fork */
}

#if EV_VERIFY
void noinline
verify_watcher (EV_P_ W w)
{
  assert (("watcher has invalid priority", ABSPRI (w) >= 0 && ABSPRI (w) < NUMPRI));

  if (w->pending)
    assert (("pending watcher not on pending queue", pendings [ABSPRI (w)][w->pending - 1].w == w));
}

static void noinline
verify_heap (EV_P_ ANHE *heap, int N)
{
  int i;

  for (i = HEAP0; i < N + HEAP0; ++i)
    {
      assert (("active index mismatch in heap", ev_active (ANHE_w (heap [i])) == i));
      assert (("heap condition violated", i == HEAP0 || ANHE_at (heap [HPARENT (i)]) <= ANHE_at (heap [i])));
      assert (("heap at cache mismatch", ANHE_at (heap [i]) == ev_at (ANHE_w (heap [i]))));

      verify_watcher (EV_A_ (W)ANHE_w (heap [i]));
    }
}

static void noinline
array_verify (EV_P_ W *ws, int cnt)
{
  while (cnt--)
    {
      assert (("active index mismatch", ev_active (ws [cnt]) == cnt + 1));
      verify_watcher (EV_A_ ws [cnt]);
    }
}
#endif

void
ev_loop_verify (EV_P)
{
#if EV_VERIFY
  int i;
  WL w;

  assert (activecnt >= -1);

  assert (fdchangemax >= fdchangecnt);
  for (i = 0; i < fdchangecnt; ++i)
    assert (("negative fd in fdchanges", fdchanges [i] >= 0));

  assert (anfdmax >= 0);
  for (i = 0; i < anfdmax; ++i)
    for (w = anfds [i].head; w; w = w->next)
      {
        verify_watcher (EV_A_ (W)w);
        assert (("inactive fd watcher on anfd list", ev_active (w) == 1));
        assert (("fd mismatch between watcher and anfd", ((ev_io *)w)->fd == i));
      }

  assert (timermax >= timercnt);
  verify_heap (EV_A_ timers, timercnt);

#if EV_PERIODIC_ENABLE
  assert (periodicmax >= periodiccnt);
  verify_heap (EV_A_ periodics, periodiccnt);
#endif

  for (i = NUMPRI; i--; )
    {
      assert (pendingmax [i] >= pendingcnt [i]);
#if EV_IDLE_ENABLE
      assert (idleall >= 0);
      assert (idlemax [i] >= idlecnt [i]);
      array_verify (EV_A_ (W *)idles [i], idlecnt [i]);
#endif
    }

#if EV_FORK_ENABLE
  assert (forkmax >= forkcnt);
  array_verify (EV_A_ (W *)forks, forkcnt);
#endif

#if EV_ASYNC_ENABLE
  assert (asyncmax >= asynccnt);
  array_verify (EV_A_ (W *)asyncs, asynccnt);
#endif

  assert (preparemax >= preparecnt);
  array_verify (EV_A_ (W *)prepares, preparecnt);

  assert (checkmax >= checkcnt);
  array_verify (EV_A_ (W *)checks, checkcnt);

# if 0
  for (w = (ev_child *)childs [chain & (EV_PID_HASHSIZE - 1)]; w; w = (ev_child *)((WL)w)->next)
  for (signum = signalmax; signum--; ) if (signals [signum].gotsig)
# endif
#endif
}

#endif /* multiplicity */

#if EV_MULTIPLICITY
struct ev_loop *
ev_default_loop_init (unsigned int flags)
#else
int
ev_default_loop (unsigned int flags)
#endif
{
  if (!ev_default_loop_ptr)
    {
#if EV_MULTIPLICITY
      struct ev_loop *loop = ev_default_loop_ptr = &default_loop_struct;
#else
      ev_default_loop_ptr = 1;
#endif

      loop_init (EV_A_ flags);

      if (ev_backend (EV_A))
        {
#ifndef _WIN32
          ev_signal_init (&childev, childcb, SIGCHLD);
          ev_set_priority (&childev, EV_MAXPRI);
          ev_signal_start (EV_A_ &childev);
          ev_unref (EV_A); /* child watcher should not keep loop alive */
#endif
        }
      else
        ev_default_loop_ptr = 0;
    }

  return ev_default_loop_ptr;
}

void
ev_default_destroy (void)
{
#if EV_MULTIPLICITY
  struct ev_loop *loop = ev_default_loop_ptr;
#endif

#ifndef _WIN32
  ev_ref (EV_A); /* child watcher */
  ev_signal_stop (EV_A_ &childev);
#endif

  loop_destroy (EV_A);
}

void
ev_default_fork (void)
{
#if EV_MULTIPLICITY
  struct ev_loop *loop = ev_default_loop_ptr;
#endif

  if (backend)
    postfork = 1; /* must be in line with ev_loop_fork */
}

/*****************************************************************************/

void
ev_invoke (EV_P_ void *w, int revents)
{
  EV_CB_INVOKE ((W)w, revents);
}

void inline_speed
call_pending (EV_P)
{
  int pri;

  for (pri = NUMPRI; pri--; )
    while (pendingcnt [pri])
      {
        ANPENDING *p = pendings [pri] + --pendingcnt [pri];

        if (expect_true (p->w))
          {
            /*assert (("non-pending watcher on pending list", p->w->pending));*/

            p->w->pending = 0;
            EV_CB_INVOKE (p->w, p->events);
            EV_FREQUENT_CHECK;
          }
      }
}

#if EV_IDLE_ENABLE
void inline_size
idle_reify (EV_P)
{
  if (expect_false (idleall))
    {
      int pri;

      for (pri = NUMPRI; pri--; )
        {
          if (pendingcnt [pri])
            break;

          if (idlecnt [pri])
            {
              queue_events (EV_A_ (W *)idles [pri], idlecnt [pri], EV_IDLE);
              break;
            }
        }
    }
}
#endif

void inline_size
timers_reify (EV_P)
{
  EV_FREQUENT_CHECK;

  while (timercnt && ANHE_at (timers [HEAP0]) < mn_now)
    {
      ev_timer *w = (ev_timer *)ANHE_w (timers [HEAP0]);

      /*assert (("inactive timer on timer heap detected", ev_is_active (w)));*/

      /* first reschedule or stop timer */
      if (w->repeat)
        {
          ev_at (w) += w->repeat;
          if (ev_at (w) < mn_now)
            ev_at (w) = mn_now;

          assert (("negative ev_timer repeat value found while processing timers", w->repeat > 0.));

          ANHE_at_cache (timers [HEAP0]);
          downheap (timers, timercnt, HEAP0);
        }
      else
        ev_timer_stop (EV_A_ w); /* nonrepeating: stop timer */

      EV_FREQUENT_CHECK;
      ev_feed_event (EV_A_ (W)w, EV_TIMEOUT);
    }
}

#if EV_PERIODIC_ENABLE
void inline_size
periodics_reify (EV_P)
{
  EV_FREQUENT_CHECK;

  while (periodiccnt && ANHE_at (periodics [HEAP0]) < ev_rt_now)
    {
      ev_periodic *w = (ev_periodic *)ANHE_w (periodics [HEAP0]);

      /*assert (("inactive timer on periodic heap detected", ev_is_active (w)));*/

      /* first reschedule or stop timer */
      if (w->reschedule_cb)
        {
          ev_at (w) = w->reschedule_cb (w, ev_rt_now);

          assert (("ev_periodic reschedule callback returned time in the past", ev_at (w) >= ev_rt_now));

          ANHE_at_cache (periodics [HEAP0]);
          downheap (periodics, periodiccnt, HEAP0);
        }
      else if (w->interval)
        {
          ev_at (w) = w->offset + ceil ((ev_rt_now - w->offset) / w->interval) * w->interval;
          /* if next trigger time is not sufficiently in the future, put it there */
          /* this might happen because of floating point inexactness */
          if (ev_at (w) - ev_rt_now < TIME_EPSILON)
            {
              ev_at (w) += w->interval;

              /* if interval is unreasonably low we might still have a time in the past */
              /* so correct this. this will make the periodic very inexact, but the user */
              /* has effectively asked to get triggered more often than possible */
              if (ev_at (w) < ev_rt_now)
                ev_at (w) = ev_rt_now;
            }

          ANHE_at_cache (periodics [HEAP0]);
          downheap (periodics, periodiccnt, HEAP0);
        }
      else
        ev_periodic_stop (EV_A_ w); /* nonrepeating: stop timer */

      EV_FREQUENT_CHECK;
      ev_feed_event (EV_A_ (W)w, EV_PERIODIC);
    }
}

static void noinline
periodics_reschedule (EV_P)
{
  int i;

  /* adjust periodics after time jump */
  for (i = HEAP0; i < periodiccnt + HEAP0; ++i)
    {
      ev_periodic *w = (ev_periodic *)ANHE_w (periodics [i]);

      if (w->reschedule_cb)
        ev_at (w) = w->reschedule_cb (w, ev_rt_now);
      else if (w->interval)
        ev_at (w) = w->offset + ceil ((ev_rt_now - w->offset) / w->interval) * w->interval;

      ANHE_at_cache (periodics [i]);
    }

  reheap (periodics, periodiccnt);
}
#endif

void inline_speed
time_update (EV_P_ ev_tstamp max_block)
{
  int i;

#if EV_USE_MONOTONIC
  if (expect_true (have_monotonic))
    {
      ev_tstamp odiff = rtmn_diff;

      mn_now = get_clock ();

      /* only fetch the realtime clock every 0.5*MIN_TIMEJUMP seconds */
      /* interpolate in the meantime */
      if (expect_true (mn_now - now_floor < MIN_TIMEJUMP * .5))
        {
          ev_rt_now = rtmn_diff + mn_now;
          return;
        }

      now_floor = mn_now;
      ev_rt_now = ev_time ();

      /* loop a few times, before making important decisions.
       * on the choice of "4": one iteration isn't enough,
       * in case we get preempted during the calls to
       * ev_time and get_clock. a second call is almost guaranteed
       * to succeed in that case, though. and looping a few more times
       * doesn't hurt either as we only do this on time-jumps or
       * in the unlikely event of having been preempted here.
       */
      for (i = 4; --i; )
        {
          rtmn_diff = ev_rt_now - mn_now;

          if (expect_true (fabs (odiff - rtmn_diff) < MIN_TIMEJUMP))
            return; /* all is well */

          ev_rt_now = ev_time ();
          mn_now    = get_clock ();
          now_floor = mn_now;
        }

# if EV_PERIODIC_ENABLE
      periodics_reschedule (EV_A);
# endif
      /* no timer adjustment, as the monotonic clock doesn't jump */
      /* timers_reschedule (EV_A_ rtmn_diff - odiff) */
    }
  else
#endif
    {
      ev_rt_now = ev_time ();

      if (expect_false (mn_now > ev_rt_now || ev_rt_now > mn_now + max_block + MIN_TIMEJUMP))
        {
#if EV_PERIODIC_ENABLE
          periodics_reschedule (EV_A);
#endif
          /* adjust timers. this is easy, as the offset is the same for all of them */
          for (i = 0; i < timercnt; ++i)
            {
              ANHE *he = timers + i + HEAP0;
              ANHE_w (*he)->at += ev_rt_now - mn_now;
              ANHE_at_cache (*he);
            }
        }

      mn_now = ev_rt_now;
    }
}

void
ev_ref (EV_P)
{
  ++activecnt;
}

void
ev_unref (EV_P)
{
  --activecnt;
}

static int loop_done;

void
ev_loop (EV_P_ int flags)
{
  loop_done = EVUNLOOP_CANCEL;

  call_pending (EV_A); /* in case we recurse, ensure ordering stays nice and clean */

  do
    {
#if EV_VERIFY >= 2
      ev_loop_verify (EV_A);
#endif

#ifndef _WIN32
      if (expect_false (curpid)) /* penalise the forking check even more */
        if (expect_false (getpid () != curpid))
          {
            curpid = getpid ();
            postfork = 1;
          }
#endif

#if EV_FORK_ENABLE
      /* we might have forked, so queue fork handlers */
      if (expect_false (postfork))
        if (forkcnt)
          {
            queue_events (EV_A_ (W *)forks, forkcnt, EV_FORK);
            call_pending (EV_A);
          }
#endif

      /* queue prepare watchers (and execute them) */
      if (expect_false (preparecnt))
        {
          queue_events (EV_A_ (W *)prepares, preparecnt, EV_PREPARE);
          call_pending (EV_A);
        }

      if (expect_false (!activecnt))
        break;

      /* we might have forked, so reify kernel state if necessary */
      if (expect_false (postfork))
        loop_fork (EV_A);

      /* update fd-related kernel structures */
      fd_reify (EV_A);

      /* calculate blocking time */
      {
        ev_tstamp waittime  = 0.;
        ev_tstamp sleeptime = 0.;

        if (expect_true (!(flags & EVLOOP_NONBLOCK || idleall || !activecnt)))
          {
            /* update time to cancel out callback processing overhead */
            time_update (EV_A_ 1e100);

            waittime = MAX_BLOCKTIME;

            if (timercnt)
              {
                ev_tstamp to = ANHE_at (timers [HEAP0]) - mn_now + backend_fudge;
                if (waittime > to) waittime = to;
              }

#if EV_PERIODIC_ENABLE
            if (periodiccnt)
              {
                ev_tstamp to = ANHE_at (periodics [HEAP0]) - ev_rt_now + backend_fudge;
                if (waittime > to) waittime = to;
              }
#endif

            if (expect_false (waittime < timeout_blocktime))
              waittime = timeout_blocktime;

            sleeptime = waittime - backend_fudge;

            if (expect_true (sleeptime > io_blocktime))
              sleeptime = io_blocktime;

            if (sleeptime)
              {
                ev_sleep (sleeptime);
                waittime -= sleeptime;
              }
          }

        ++loop_count;
        backend_poll (EV_A_ waittime);

        /* update ev_rt_now, do magic */
        time_update (EV_A_ waittime + sleeptime);
      }

      /* queue pending timers and reschedule them */
      timers_reify (EV_A); /* relative timers called last */
#if EV_PERIODIC_ENABLE
      periodics_reify (EV_A); /* absolute timers called first */
#endif

#if EV_IDLE_ENABLE
      /* queue idle watchers unless other events are pending */
      idle_reify (EV_A);
#endif

      /* queue check watchers, to be executed first */
      if (expect_false (checkcnt))
        queue_events (EV_A_ (W *)checks, checkcnt, EV_CHECK);

      call_pending (EV_A);
    }
  while (expect_true (
    activecnt
    && !loop_done
    && !(flags & (EVLOOP_ONESHOT | EVLOOP_NONBLOCK))
  ));

  if (loop_done == EVUNLOOP_ONE)
    loop_done = EVUNLOOP_CANCEL;
}

void
ev_unloop (EV_P_ int how)
{
  loop_done = how;
}

/*****************************************************************************/

void inline_size
wlist_add (WL *head, WL elem)
{
  elem->next = *head;
  *head = elem;
}

void inline_size
wlist_del (WL *head, WL elem)
{
  while (*head)
    {
      if (*head == elem)
        {
          *head = elem->next;
          return;
        }

      head = &(*head)->next;
    }
}

void inline_speed
clear_pending (EV_P_ W w)
{
  if (w->pending)
    {
      pendings [ABSPRI (w)][w->pending - 1].w = 0;
      w->pending = 0;
    }
}

int
ev_clear_pending (EV_P_ void *w)
{
  W w_ = (W)w;
  int pending = w_->pending;

  if (expect_true (pending))
    {
      ANPENDING *p = pendings [ABSPRI (w_)] + pending - 1;
      w_->pending = 0;
      p->w = 0;
      return p->events;
    }
  else
    return 0;
}

void inline_size
pri_adjust (EV_P_ W w)
{
  int pri = w->priority;
  pri = pri < EV_MINPRI ? EV_MINPRI : pri;
  pri = pri > EV_MAXPRI ? EV_MAXPRI : pri;
  w->priority = pri;
}

void inline_speed
ev_start (EV_P_ W w, int active)
{
  pri_adjust (EV_A_ w);
  w->active = active;
  ev_ref (EV_A);
}

void inline_size
ev_stop (EV_P_ W w)
{
  ev_unref (EV_A);
  w->active = 0;
}

/*****************************************************************************/

void noinline
ev_io_start (EV_P_ ev_io *w)
{
  int fd = w->fd;

  if (expect_false (ev_is_active (w)))
    return;

  assert (("ev_io_start called with negative fd", fd >= 0));

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, 1);
  array_needsize (ANFD, anfds, anfdmax, fd + 1, anfds_init);
  wlist_add (&anfds[fd].head, (WL)w);

  fd_change (EV_A_ fd, w->events & EV_IOFDSET | 1);
  w->events &= ~EV_IOFDSET;

  EV_FREQUENT_CHECK;
}

void noinline
ev_io_stop (EV_P_ ev_io *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  assert (("ev_io_stop called with illegal fd (must stay constant after start!)", w->fd >= 0 && w->fd < anfdmax));

  EV_FREQUENT_CHECK;

  wlist_del (&anfds[w->fd].head, (WL)w);
  ev_stop (EV_A_ (W)w);

  fd_change (EV_A_ w->fd, 1);

  EV_FREQUENT_CHECK;
}

void noinline
ev_timer_start (EV_P_ ev_timer *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  ev_at (w) += mn_now;

  assert (("ev_timer_start called with negative timer repeat value", w->repeat >= 0.));

  EV_FREQUENT_CHECK;

  ++timercnt;
  ev_start (EV_A_ (W)w, timercnt + HEAP0 - 1);
  array_needsize (ANHE, timers, timermax, ev_active (w) + 1, EMPTY2);
  ANHE_w (timers [ev_active (w)]) = (WT)w;
  ANHE_at_cache (timers [ev_active (w)]);
  upheap (timers, ev_active (w));

  EV_FREQUENT_CHECK;

  /*assert (("internal timer heap corruption", timers [ev_active (w)] == (WT)w));*/
}

void noinline
ev_timer_stop (EV_P_ ev_timer *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    assert (("internal timer heap corruption", ANHE_w (timers [active]) == (WT)w));

    --timercnt;

    if (expect_true (active < timercnt + HEAP0))
      {
        timers [active] = timers [timercnt + HEAP0];
        adjustheap (timers, timercnt, active);
      }
  }

  EV_FREQUENT_CHECK;

  ev_at (w) -= mn_now;

  ev_stop (EV_A_ (W)w);
}

void noinline
ev_timer_again (EV_P_ ev_timer *w)
{
  EV_FREQUENT_CHECK;

  if (ev_is_active (w))
    {
      if (w->repeat)
        {
          ev_at (w) = mn_now + w->repeat;
          ANHE_at_cache (timers [ev_active (w)]);
          adjustheap (timers, timercnt, ev_active (w));
        }
      else
        ev_timer_stop (EV_A_ w);
    }
  else if (w->repeat)
    {
      ev_at (w) = w->repeat;
      ev_timer_start (EV_A_ w);
    }

  EV_FREQUENT_CHECK;
}

#if EV_PERIODIC_ENABLE
void noinline
ev_periodic_start (EV_P_ ev_periodic *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  if (w->reschedule_cb)
    ev_at (w) = w->reschedule_cb (w, ev_rt_now);
  else if (w->interval)
    {
      assert (("ev_periodic_start called with negative interval value", w->interval >= 0.));
      /* this formula differs from the one in periodic_reify because we do not always round up */
      ev_at (w) = w->offset + ceil ((ev_rt_now - w->offset) / w->interval) * w->interval;
    }
  else
    ev_at (w) = w->offset;

  EV_FREQUENT_CHECK;

  ++periodiccnt;
  ev_start (EV_A_ (W)w, periodiccnt + HEAP0 - 1);
  array_needsize (ANHE, periodics, periodicmax, ev_active (w) + 1, EMPTY2);
  ANHE_w (periodics [ev_active (w)]) = (WT)w;
  ANHE_at_cache (periodics [ev_active (w)]);
  upheap (periodics, ev_active (w));

  EV_FREQUENT_CHECK;

  /*assert (("internal periodic heap corruption", ANHE_w (periodics [ev_active (w)]) == (WT)w));*/
}

void noinline
ev_periodic_stop (EV_P_ ev_periodic *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    assert (("internal periodic heap corruption", ANHE_w (periodics [active]) == (WT)w));

    --periodiccnt;

    if (expect_true (active < periodiccnt + HEAP0))
      {
        periodics [active] = periodics [periodiccnt + HEAP0];
        adjustheap (periodics, periodiccnt, active);
      }
  }

  EV_FREQUENT_CHECK;

  ev_stop (EV_A_ (W)w);
}

void noinline
ev_periodic_again (EV_P_ ev_periodic *w)
{
  /* TODO: use adjustheap and recalculation */
  ev_periodic_stop (EV_A_ w);
  ev_periodic_start (EV_A_ w);
}
#endif

#ifndef SA_RESTART
# define SA_RESTART 0
#endif

void noinline
ev_signal_start (EV_P_ ev_signal *w)
{
#if EV_MULTIPLICITY
  assert (("signal watchers are only supported in the default loop", loop == ev_default_loop_ptr));
#endif
  if (expect_false (ev_is_active (w)))
    return;

  assert (("ev_signal_start called with illegal signal number", w->signum > 0));

  evpipe_init (EV_A);

  EV_FREQUENT_CHECK;

  {
#ifndef _WIN32
    sigset_t full, prev;
    sigfillset (&full);
    sigprocmask (SIG_SETMASK, &full, &prev);
#endif

    array_needsize (ANSIG, signals, signalmax, w->signum, signals_init);

#ifndef _WIN32
    sigprocmask (SIG_SETMASK, &prev, 0);
#endif
  }

  ev_start (EV_A_ (W)w, 1);
  wlist_add (&signals [w->signum - 1].head, (WL)w);

  if (!((WL)w)->next)
    {
#if _WIN32
      signal (w->signum, ev_sighandler);
#else
      struct sigaction sa;
      sa.sa_handler = ev_sighandler;
      sigfillset (&sa.sa_mask);
      sa.sa_flags = SA_RESTART; /* if restarting works we save one iteration */
      sigaction (w->signum, &sa, 0);
#endif
    }

  EV_FREQUENT_CHECK;
}

void noinline
ev_signal_stop (EV_P_ ev_signal *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  wlist_del (&signals [w->signum - 1].head, (WL)w);
  ev_stop (EV_A_ (W)w);

  if (!signals [w->signum - 1].head)
    signal (w->signum, SIG_DFL);

  EV_FREQUENT_CHECK;
}

void
ev_child_start (EV_P_ ev_child *w)
{
#if EV_MULTIPLICITY
  assert (("child watchers are only supported in the default loop", loop == ev_default_loop_ptr));
#endif
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, 1);
  wlist_add (&childs [w->pid & (EV_PID_HASHSIZE - 1)], (WL)w);

  EV_FREQUENT_CHECK;
}

void
ev_child_stop (EV_P_ ev_child *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  wlist_del (&childs [w->pid & (EV_PID_HASHSIZE - 1)], (WL)w);
  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}

#if EV_STAT_ENABLE

# ifdef _WIN32
#  undef lstat
#  define lstat(a,b) _stati64 (a,b)
# endif

#define DEF_STAT_INTERVAL 5.0074891
#define MIN_STAT_INTERVAL 0.1074891

static void noinline stat_timer_cb (EV_P_ ev_timer *w_, int revents);

#if EV_USE_INOTIFY
# define EV_INOTIFY_BUFSIZE 8192

static void noinline
infy_add (EV_P_ ev_stat *w)
{
  w->wd = inotify_add_watch (fs_fd, w->path, IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY | IN_DONT_FOLLOW | IN_MASK_ADD);

  if (w->wd < 0)
    {
      ev_timer_start (EV_A_ &w->timer); /* this is not race-free, so we still need to recheck periodically */

      /* monitor some parent directory for speedup hints */
      /* note that exceeding the hardcoded limit is not a correctness issue, */
      /* but an efficiency issue only */
      if ((errno == ENOENT || errno == EACCES) && strlen (w->path) < 4096)
        {
          char path [4096];
          strcpy (path, w->path);

          do
            {
              int mask = IN_MASK_ADD | IN_DELETE_SELF | IN_MOVE_SELF
                       | (errno == EACCES ? IN_ATTRIB : IN_CREATE | IN_MOVED_TO);

              char *pend = strrchr (path, '/');

              if (!pend)
                break; /* whoops, no '/', complain to your admin */

              *pend = 0;
              w->wd = inotify_add_watch (fs_fd, path, mask);
            } 
          while (w->wd < 0 && (errno == ENOENT || errno == EACCES));
        }
    }
  else
    ev_timer_stop (EV_A_ &w->timer); /* we can watch this in a race-free way */

  if (w->wd >= 0)
    wlist_add (&fs_hash [w->wd & (EV_INOTIFY_HASHSIZE - 1)].head, (WL)w);
}

static void noinline
infy_del (EV_P_ ev_stat *w)
{
  int slot;
  int wd = w->wd;

  if (wd < 0)
    return;

  w->wd = -2;
  slot = wd & (EV_INOTIFY_HASHSIZE - 1);
  wlist_del (&fs_hash [slot].head, (WL)w);

  /* remove this watcher, if others are watching it, they will rearm */
  inotify_rm_watch (fs_fd, wd);
}

static void noinline
infy_wd (EV_P_ int slot, int wd, struct inotify_event *ev)
{
  if (slot < 0)
    /* overflow, need to check for all hahs slots */
    for (slot = 0; slot < EV_INOTIFY_HASHSIZE; ++slot)
      infy_wd (EV_A_ slot, wd, ev);
  else
    {
      WL w_;

      for (w_ = fs_hash [slot & (EV_INOTIFY_HASHSIZE - 1)].head; w_; )
        {
          ev_stat *w = (ev_stat *)w_;
          w_ = w_->next; /* lets us remove this watcher and all before it */

          if (w->wd == wd || wd == -1)
            {
              if (ev->mask & (IN_IGNORED | IN_UNMOUNT | IN_DELETE_SELF))
                {
                  w->wd = -1;
                  infy_add (EV_A_ w); /* re-add, no matter what */
                }

              stat_timer_cb (EV_A_ &w->timer, 0);
            }
        }
    }
}

static void
infy_cb (EV_P_ ev_io *w, int revents)
{
  char buf [EV_INOTIFY_BUFSIZE];
  struct inotify_event *ev = (struct inotify_event *)buf;
  int ofs;
  int len = read (fs_fd, buf, sizeof (buf));

  for (ofs = 0; ofs < len; ofs += sizeof (struct inotify_event) + ev->len)
    infy_wd (EV_A_ ev->wd, ev->wd, ev);
}

void inline_size
infy_init (EV_P)
{
  if (fs_fd != -2)
    return;

  fs_fd = inotify_init ();

  if (fs_fd >= 0)
    {
      ev_io_init (&fs_w, infy_cb, fs_fd, EV_READ);
      ev_set_priority (&fs_w, EV_MAXPRI);
      ev_io_start (EV_A_ &fs_w);
    }
}

void inline_size
infy_fork (EV_P)
{
  int slot;

  if (fs_fd < 0)
    return;

  close (fs_fd);
  fs_fd = inotify_init ();

  for (slot = 0; slot < EV_INOTIFY_HASHSIZE; ++slot)
    {
      WL w_ = fs_hash [slot].head;
      fs_hash [slot].head = 0;

      while (w_)
        {
          ev_stat *w = (ev_stat *)w_;
          w_ = w_->next; /* lets us add this watcher */

          w->wd = -1;

          if (fs_fd >= 0)
            infy_add (EV_A_ w); /* re-add, no matter what */
          else
            ev_timer_start (EV_A_ &w->timer);
        }

    }
}

#endif

void
ev_stat_stat (EV_P_ ev_stat *w)
{
  if (lstat (w->path, &w->attr) < 0)
    w->attr.st_nlink = 0;
  else if (!w->attr.st_nlink)
    w->attr.st_nlink = 1;
}

static void noinline
stat_timer_cb (EV_P_ ev_timer *w_, int revents)
{
  ev_stat *w = (ev_stat *)(((char *)w_) - offsetof (ev_stat, timer));

  /* we copy this here each the time so that */
  /* prev has the old value when the callback gets invoked */
  w->prev = w->attr;
  ev_stat_stat (EV_A_ w);

  /* memcmp doesn't work on netbsd, they.... do stuff to their struct stat */
  if (
    w->prev.st_dev      != w->attr.st_dev
    || w->prev.st_ino   != w->attr.st_ino
    || w->prev.st_mode  != w->attr.st_mode
    || w->prev.st_nlink != w->attr.st_nlink
    || w->prev.st_uid   != w->attr.st_uid
    || w->prev.st_gid   != w->attr.st_gid
    || w->prev.st_rdev  != w->attr.st_rdev
    || w->prev.st_size  != w->attr.st_size
    || w->prev.st_atime != w->attr.st_atime
    || w->prev.st_mtime != w->attr.st_mtime
    || w->prev.st_ctime != w->attr.st_ctime
  ) {
      #if EV_USE_INOTIFY
        infy_del (EV_A_ w);
        infy_add (EV_A_ w);
        ev_stat_stat (EV_A_ w); /* avoid race... */
      #endif

      ev_feed_event (EV_A_ w, EV_STAT);
    }
}

void
ev_stat_start (EV_P_ ev_stat *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  /* since we use memcmp, we need to clear any padding data etc. */
  memset (&w->prev, 0, sizeof (ev_statdata));
  memset (&w->attr, 0, sizeof (ev_statdata));

  ev_stat_stat (EV_A_ w);

  if (w->interval < MIN_STAT_INTERVAL)
    w->interval = w->interval ? MIN_STAT_INTERVAL : DEF_STAT_INTERVAL;

  ev_timer_init (&w->timer, stat_timer_cb, w->interval, w->interval);
  ev_set_priority (&w->timer, ev_priority (w));

#if EV_USE_INOTIFY
  infy_init (EV_A);

  if (fs_fd >= 0)
    infy_add (EV_A_ w);
  else
#endif
    ev_timer_start (EV_A_ &w->timer);

  ev_start (EV_A_ (W)w, 1);

  EV_FREQUENT_CHECK;
}

void
ev_stat_stop (EV_P_ ev_stat *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

#if EV_USE_INOTIFY
  infy_del (EV_A_ w);
#endif
  ev_timer_stop (EV_A_ &w->timer);

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

#if EV_IDLE_ENABLE
void
ev_idle_start (EV_P_ ev_idle *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  pri_adjust (EV_A_ (W)w);

  EV_FREQUENT_CHECK;

  {
    int active = ++idlecnt [ABSPRI (w)];

    ++idleall;
    ev_start (EV_A_ (W)w, active);

    array_needsize (ev_idle *, idles [ABSPRI (w)], idlemax [ABSPRI (w)], active, EMPTY2);
    idles [ABSPRI (w)][active - 1] = w;
  }

  EV_FREQUENT_CHECK;
}

void
ev_idle_stop (EV_P_ ev_idle *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    idles [ABSPRI (w)][active - 1] = idles [ABSPRI (w)][--idlecnt [ABSPRI (w)]];
    ev_active (idles [ABSPRI (w)][active - 1]) = active;

    ev_stop (EV_A_ (W)w);
    --idleall;
  }

  EV_FREQUENT_CHECK;
}
#endif

void
ev_prepare_start (EV_P_ ev_prepare *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++preparecnt);
  array_needsize (ev_prepare *, prepares, preparemax, preparecnt, EMPTY2);
  prepares [preparecnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_prepare_stop (EV_P_ ev_prepare *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    prepares [active - 1] = prepares [--preparecnt];
    ev_active (prepares [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}

void
ev_check_start (EV_P_ ev_check *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++checkcnt);
  array_needsize (ev_check *, checks, checkmax, checkcnt, EMPTY2);
  checks [checkcnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_check_stop (EV_P_ ev_check *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    checks [active - 1] = checks [--checkcnt];
    ev_active (checks [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}

#if EV_EMBED_ENABLE
void noinline
ev_embed_sweep (EV_P_ ev_embed *w)
{
  ev_loop (w->other, EVLOOP_NONBLOCK);
}

static void
embed_io_cb (EV_P_ ev_io *io, int revents)
{
  ev_embed *w = (ev_embed *)(((char *)io) - offsetof (ev_embed, io));

  if (ev_cb (w))
    ev_feed_event (EV_A_ (W)w, EV_EMBED);
  else
    ev_loop (w->other, EVLOOP_NONBLOCK);
}

static void
embed_prepare_cb (EV_P_ ev_prepare *prepare, int revents)
{
  ev_embed *w = (ev_embed *)(((char *)prepare) - offsetof (ev_embed, prepare));

  {
    struct ev_loop *loop = w->other;

    while (fdchangecnt)
      {
        fd_reify (EV_A);
        ev_loop (EV_A_ EVLOOP_NONBLOCK);
      }
  }
}

#if 0
static void
embed_idle_cb (EV_P_ ev_idle *idle, int revents)
{
  ev_idle_stop (EV_A_ idle);
}
#endif

void
ev_embed_start (EV_P_ ev_embed *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  {
    struct ev_loop *loop = w->other;
    assert (("loop to be embedded is not embeddable", backend & ev_embeddable_backends ()));
    ev_io_init (&w->io, embed_io_cb, backend_fd, EV_READ);
  }

  EV_FREQUENT_CHECK;

  ev_set_priority (&w->io, ev_priority (w));
  ev_io_start (EV_A_ &w->io);

  ev_prepare_init (&w->prepare, embed_prepare_cb);
  ev_set_priority (&w->prepare, EV_MINPRI);
  ev_prepare_start (EV_A_ &w->prepare);

  /*ev_idle_init (&w->idle, e,bed_idle_cb);*/

  ev_start (EV_A_ (W)w, 1);

  EV_FREQUENT_CHECK;
}

void
ev_embed_stop (EV_P_ ev_embed *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_io_stop (EV_A_ &w->io);
  ev_prepare_stop (EV_A_ &w->prepare);

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

#if EV_FORK_ENABLE
void
ev_fork_start (EV_P_ ev_fork *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++forkcnt);
  array_needsize (ev_fork *, forks, forkmax, forkcnt, EMPTY2);
  forks [forkcnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_fork_stop (EV_P_ ev_fork *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    forks [active - 1] = forks [--forkcnt];
    ev_active (forks [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

#if EV_ASYNC_ENABLE
void
ev_async_start (EV_P_ ev_async *w)
{
  if (expect_false (ev_is_active (w)))
    return;

  evpipe_init (EV_A);

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++asynccnt);
  array_needsize (ev_async *, asyncs, asyncmax, asynccnt, EMPTY2);
  asyncs [asynccnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_async_stop (EV_P_ ev_async *w)
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    asyncs [active - 1] = asyncs [--asynccnt];
    ev_active (asyncs [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}

void
ev_async_send (EV_P_ ev_async *w)
{
  w->sent = 1;
  evpipe_write (EV_A_ &gotasync);
}
#endif

/*****************************************************************************/

struct ev_once
{
  ev_io io;
  ev_timer to;
  void (*cb)(int revents, void *arg);
  void *arg;
};

static void
once_cb (EV_P_ struct ev_once *once, int revents)
{
  void (*cb)(int revents, void *arg) = once->cb;
  void *arg = once->arg;

  ev_io_stop (EV_A_ &once->io);
  ev_timer_stop (EV_A_ &once->to);
  ev_free (once);

  cb (revents, arg);
}

static void
once_cb_io (EV_P_ ev_io *w, int revents)
{
  once_cb (EV_A_ (struct ev_once *)(((char *)w) - offsetof (struct ev_once, io)), revents);
}

static void
once_cb_to (EV_P_ ev_timer *w, int revents)
{
  once_cb (EV_A_ (struct ev_once *)(((char *)w) - offsetof (struct ev_once, to)), revents);
}

void
ev_once (EV_P_ int fd, int events, ev_tstamp timeout, void (*cb)(int revents, void *arg), void *arg)
{
  struct ev_once *once = (struct ev_once *)ev_malloc (sizeof (struct ev_once));

  if (expect_false (!once))
    {
      cb (EV_ERROR | EV_READ | EV_WRITE | EV_TIMEOUT, arg);
      return;
    }

  once->cb  = cb;
  once->arg = arg;

  ev_init (&once->io, once_cb_io);
  if (fd >= 0)
    {
      ev_io_set (&once->io, fd, events);
      ev_io_start (EV_A_ &once->io);
    }

  ev_init (&once->to, once_cb_to);
  if (timeout >= 0.)
    {
      ev_timer_set (&once->to, timeout, 0.);
      ev_timer_start (EV_A_ &once->to);
    }
}

#if EV_MULTIPLICITY
  #include "ev_wrap.h"
#endif

#ifdef __cplusplus
}
#endif

