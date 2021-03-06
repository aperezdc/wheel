/*
 * wev.c
 * Copyright (C) 2012 Adrian Perez <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "wheel.h"

/*
 * Number of events to get from the kernel in each
 * call to w_event_loop_backend_poll.
 */
#ifndef W_EVENT_LOOP_NEVENTS
#define W_EVENT_LOOP_NEVENTS 32
#endif /* !W_EVENT_LOOP_NEVENTS */


/*
 * Chain of fools that chooses the backend:
 *   - epoll() on Linux.
 *   - kqueue() on BSDs.
 */
#if !(defined(W_EVENT_BACKEND_EPOLL) || defined(W_EVENT_BACKEND_KQUEUE))
# if defined(linux)
#  define W_EVENT_BACKEND_EPOLL 1
# elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#  define W_EVENT_BACKEND_KQUEUE 1
# else
#  error There is not an event loop backend for your platform.
# endif
#endif /* !(W_EVENT_BACKEND_EPOLL || W_EVENT_BACKEND_KQUEUE) */

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <time.h>


static size_t w_event_loop_backend_size  (void);
static bool   w_event_loop_backend_init  (w_event_loop_t*);
static void   w_event_loop_backend_free  (w_event_loop_t*);
static bool   w_event_loop_backend_start (w_event_loop_t*);
static bool   w_event_loop_backend_stop  (w_event_loop_t*);
static bool   w_event_loop_backend_add   (w_event_loop_t*, w_event_t*);
static bool   w_event_loop_backend_del   (w_event_loop_t*, w_event_t*);
static bool   w_event_loop_backend_poll  (w_event_loop_t*, w_timestamp_t);


static inline bool
fd_set_nonblocking (int fd)
{
    int flags;
    return (flags = fcntl (fd, F_GETFL)) < 0
        || fcntl (fd, F_SETFL, flags | O_NONBLOCK) == -1;
}


w_timestamp_t
w_timestamp_now (void)
{
#if _POSIX_TIMERS
    static bool use_realtime = true;

    if (use_realtime) {
        struct timespec ts;
        if (clock_gettime (CLOCK_REALTIME, &ts) == 0) {
            return ts.tv_sec + ts.tv_nsec * 1e-9;
        }

        /*
         * If control reaches here, using the clock_gettime failed,
         * so mark it to not be used and fall-back to gettimeofday.
         */
        use_realtime = false;
    }
#endif /* _POSIX_TIMERS */
    {
        struct timeval tv;
        gettimeofday (&tv, 0);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }
}


static void
_w_event_destroy (void *obj)
{
    w_event_t *event = (w_event_t*) obj;
    w_assert (event);

    switch (event->type) {
        case W_EVENT_IO:
            w_obj_unref (event->io);
            break;
        case W_EVENT_FD:
        case W_EVENT_TIMER:
        case W_EVENT_SIGNAL:
        case W_EVENT_IDLE:
            /* Nothing to be done. */
            break;
    }
}


w_event_t*
w_event_new (w_event_type_t type, w_event_callback_t callback, ...)
{
    va_list args;
    w_event_t *event;

    w_assert (callback);
    va_start (args, callback);

    event = w_obj_new (w_event_t);
    event->callback = callback;
    event->flags    = 0;

    switch ((event->type = type)) {
        case W_EVENT_FD:
            event->fd    = va_arg (args, int);
            event->flags = va_arg (args, int);
            break;
        case W_EVENT_IO:
            event->io    = w_obj_ref (va_arg (args, w_io_unix_t*));
            event->flags = va_arg (args, w_event_flags_t);
            event->flags &= (W_EVENT_IN | W_EVENT_OUT);
            break;
        case W_EVENT_TIMER:
            event->time  = va_arg (args, w_timestamp_t);
            break;
        case W_EVENT_SIGNAL:
            event->signum = va_arg (args, int);
            break;
        case W_EVENT_IDLE:
            event->flags = va_arg (args, w_event_flags_t);
            event->flags &= (W_EVENT_ONESHOT | W_EVENT_REPEAT);
            break;
    }
    va_end (args);
    return w_obj_dtor (event, _w_event_destroy);
}


static void
_w_event_loop_destroy (void *obj)
{
    w_event_loop_t *loop = (w_event_loop_t*) obj;
    w_event_loop_backend_free (loop);
    w_obj_unref (loop->events);
}


w_event_loop_t*
w_event_loop_new (void)
{
    w_event_loop_t *loop = w_obj_new_with_priv_sized (w_event_loop_t,
                                                      w_event_loop_backend_size ());

    if (w_event_loop_backend_init (loop)) {
        w_obj_unref (loop);
        return NULL;
    }

    loop->running = false;
    loop->events  = w_list_new (true);
    loop->now     = w_timestamp_now ();
    return w_obj_dtor (loop, _w_event_loop_destroy);
}


bool
w_event_loop_run (w_event_loop_t *loop)
{
    w_assert (loop);

    if (w_event_loop_backend_start (loop))
        return true;

    loop->running = true;

    /* TODO Allow control of timeouts for polling! */
    /* XXX Does that _actually_ make sense?? */
    while (loop->running) {
        if (w_event_loop_backend_poll (loop, -1.0)) {
            loop->running = false;
        }
        else {
            w_list_foreach (i, loop->idle_events) {
                w_event_t *event = *i;
                w_assert (event->type == W_EVENT_IDLE);

                (*event->callback) (loop, event);

                if (event->flags & W_EVENT_ONESHOT) {
                    w_list_del (loop->idle_events, i);
                }
            }
        }
    }

    return w_event_loop_backend_stop (loop);
}


void
w_event_loop_stop (w_event_loop_t *loop)
{
    w_assert (loop);
    loop->running = false;
}


bool
w_event_loop_add (w_event_loop_t *loop, w_event_t *event)
{
    bool ret = false;

    w_assert (loop);
    w_assert (event);

    /* Adding the element to the list will w_obj_ref() it, too */
    if (event->type == W_EVENT_IDLE)
        w_list_push_tail (loop->idle_events, event);
    else if (!(ret = w_event_loop_backend_add (loop, event)))
        w_list_push_head (loop->events, event);

    return ret;
}


bool
w_event_loop_del (w_event_loop_t *loop, w_event_t *event)
{
    w_iterator_t i;

    w_assert (loop);
    w_assert (event);

    w_list_t* list = event->type == W_EVENT_IDLE
        ? loop->idle_events
        : loop->events;

    bool ret = false;
    for (i = w_list_first (list); i; i = w_list_next (list, i))
        if (*i == event)
            goto found;

    return true;

found:
    /* Removing from the list will also w_obj_unref() the event */
    if (event->type == W_EVENT_IDLE)
        w_list_del (list, i);
    else if (!(ret = w_event_loop_backend_del (loop, event)))
        w_list_del (list, i);

    return ret;
}


#if defined(W_EVENT_BACKEND_EPOLL)
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#define W_EPOLL_SIGNAL_MARK ((void*) 0xbabebabe)


struct w_epoll
{
    int       fd;
    int       signal_fd;
    sigset_t  signal_mask;
    w_list_t *signal_events;
};
typedef struct w_epoll w_epoll_t;


static size_t
w_event_loop_backend_size (void)
{
    return sizeof (w_epoll_t);
}


static bool
w_event_loop_backend_init (w_event_loop_t *loop)
{
    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);

    w_assert (loop);
    w_assert (ep);

    /*
     * The epoll and signal file descriptors will be created lazily
     * when an event is first added to the event loop.
     */
    ep->fd = ep->signal_fd = -1;
    sigemptyset (&ep->signal_mask);
    ep->signal_events = w_list_new (false);
    return false;
}


static void
w_event_loop_backend_free (w_event_loop_t *loop)
{
    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    w_assert (loop);
    w_assert (ep);

    w_obj_unref (ep->signal_events);
    if (ep->signal_fd >= 0)
        close (ep->signal_fd);
    if (ep->fd >= 0)
        close (ep->fd);
}


static bool
w_event_loop_backend_poll (w_event_loop_t *loop, w_timestamp_t timeout)
{
    bool stop_loop = false;
    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    struct epoll_event events[W_EVENT_LOOP_NEVENTS];
    int nevents, i;

    w_assert (loop);
    w_assert (ep);
    w_assert (ep->fd >= 0);

    nevents = epoll_wait (ep->fd,
                          events,
                          W_EVENT_LOOP_NEVENTS,
                          (int) (timeout > 0.0) ? timeout * 1000 : timeout);

    /* TODO Check for errors on (nevents < 0) */

    loop->now = w_timestamp_now ();

    for (i = 0; i < nevents && !stop_loop; i++) {
        w_event_t *event = events[i].data.ptr;
        if (event == W_EPOLL_SIGNAL_MARK) {
            w_assert (ep->signal_fd >= 0);

            struct signalfd_siginfo si;
            if (read (ep->signal_fd, &si, sizeof (struct signalfd_siginfo))
                                       != sizeof (struct signalfd_siginfo))
                /* XXX This may be too drastic... */
                abort ();

            w_list_foreach (i, ep->signal_events) {
                event = *i;
                if (si.ssi_signo == (uint32_t) event->signum)
                    if ((*event->callback) (loop, event))
                        stop_loop = true;
            }
        }
        else {
            if (event->type == W_EVENT_TIMER) {
                uint64_t expires = 0;
                /* The "flags" field is used to store a file descriptor, ugh */
                if (read (event->flags, &expires, sizeof (uint64_t)) != sizeof (uint64_t) &&
                    errno != EAGAIN)
                    /* XXX This may be too drastic... */
                    abort ();

                /*
                 * TODO use the "expirations" counter to do something
                 * sensible with it instead of just ignoring it.
                 */
            }

            if ((*event->callback) (loop, event))
                stop_loop = true;
        }
    }
    return stop_loop;
}


static bool
w_event_loop_backend_add (w_event_loop_t *loop, w_event_t *event)
{
    w_assert (event->type != W_EVENT_IDLE);

    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    struct epoll_event ep_ev;
    sigset_t old_sigmask;
    bool ret;
    int fd = -1;

    w_assert (loop);
    w_assert (event);
    w_assert (ep);

    if (ep->fd < 0 && (ep->fd = epoll_create1 (EPOLL_CLOEXEC)) == -1)
        return true;

    ep_ev.data.ptr = event;
    ep_ev.events   = EPOLLET; /* Always use edge-triggered events */

    switch (event->type) {
        case W_EVENT_IO:
        case W_EVENT_FD:
            if (W_HAS_FLAG (event->flags, W_EVENT_IN))
                ep_ev.events |= EPOLLIN;
            if (W_HAS_FLAG (event->flags, W_EVENT_OUT))
                ep_ev.events |= EPOLLOUT;

            fd = (event->type == W_EVENT_IO)
               ? w_io_get_fd (event->io)
               : event->fd;

            if (fd < 0 || fd_set_nonblocking (fd))
                return true;
            break;

        case W_EVENT_SIGNAL:
            if (sigismember (&ep->signal_mask, event->signum)) {
                /* This kind of signal is already being handled */
                w_assert (ep->signal_fd >= 0);
                w_list_push_tail (ep->signal_events, event);
                return false;
            }

            sigaddset (&ep->signal_mask, event->signum);
            if (sigprocmask (SIG_BLOCK, &ep->signal_mask, &old_sigmask) != 0)
                return true;

            if ((fd = signalfd (ep->signal_fd, &ep->signal_mask, SFD_CLOEXEC)) == -1) {
                /*
                 * If failed to create/modify signal_fd, try to restore the old
                 * signal mask, so the masking state is left untouched on
                 * failure (if possible).
                 */
                sigprocmask (SIG_SETMASK, &old_sigmask, NULL);
                return true;
            }

            ep_ev.data.ptr = W_EPOLL_SIGNAL_MARK;
            ep_ev.events   = EPOLLIN;
            ep->signal_fd  = fd;
            break;

        case W_EVENT_TIMER:
            if ((fd = event->flags = timerfd_create (CLOCK_MONOTONIC, TFD_CLOEXEC)) == -1)
                return true;
            if (fd_set_nonblocking (fd)) {
                close (fd);
                return true;
            }
            ep_ev.events = EPOLLIN;
            break;

        case W_EVENT_IDLE:
            W_BUG ("Called with event of type W_EVENT_IDLE.\n");
    }
    w_assert (fd >= 0);

    ret = epoll_ctl (ep->fd, EPOLL_CTL_ADD, fd, &ep_ev) != 0 && errno != EEXIST;

    if (!ret && event->type == W_EVENT_SIGNAL)
        w_list_push_tail (ep->signal_events, event);

    return ret;
}


static bool
w_event_loop_backend_del (w_event_loop_t *loop, w_event_t *event)
{
    w_assert (event->type != W_EVENT_IDLE);

    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    struct epoll_event ep_ev;
    w_iterator_t delpos = 0;
    sigset_t sigmask;
    int fd = -1;

    w_assert (loop);
    w_assert (event);
    w_assert (ep);
    w_assert (ep->fd >= 0);

    switch (event->type) {
        case W_EVENT_SIGNAL:
            /* It is only needed to modify the signal mask. */
            if (ep->signal_fd < 0)
                return true;
            if (!sigismember (&ep->signal_mask, event->signum))
                return true;

            fd = 0;
            w_list_foreach (i, ep->signal_events) {
                if (((w_event_t *) *i)->signum == event->signum) fd++;
                if (event == *i) delpos = i;
            }

            /* Last event for this signal, modify signalfd */
            if (fd == 1) {
                sigemptyset (&sigmask);
                sigaddset (&sigmask, event->signum);
                sigdelset (&ep->signal_mask, event->signum);
                if (sigprocmask (SIG_UNBLOCK, &sigmask, NULL) != 0)
                    return true;
                if (signalfd (ep->signal_fd, &ep->signal_mask, O_CLOEXEC) == -1)
                    return true;
            }
            w_list_del (ep->signal_events, delpos);
            return false;

        case W_EVENT_FD:
            fd = event->fd;
            break;

        case W_EVENT_IO:
            fd = w_io_get_fd (event->io);
            break;

        case W_EVENT_TIMER:
            /* The "flags" attribute is being used to store the fd. */
            /* TODO/FIXME close the timer fd */
            fd = (int) event->flags;
            break;

        case W_EVENT_IDLE:
            W_BUG ("Called with event of type W_EVENT_IDLE.\n");
    }
    w_assert (fd >= 0);

    /*
     * XXX Note that it is theoretically possible to pass NULL as last
     * argument, but kernels prior to 2.6.10 actually do need it to be
     * non-NULL, even when the argument is not used.
     */
    return epoll_ctl (ep->fd, EPOLL_CTL_DEL, fd, &ep_ev) != 0
        && errno != ENOENT;
}


static bool
w_event_loop_backend_start (w_event_loop_t *loop)
{
    w_assert (loop);

    /* Arm timers */
    w_list_foreach (i, loop->events) {
        w_event_t *event = *i;
        if (event->type == W_EVENT_TIMER) {
            struct itimerspec its;

            its.it_value.tv_sec = 0;
            its.it_value.tv_nsec = 1;
            its.it_interval.tv_sec = (time_t) floor (event->time);
            its.it_interval.tv_nsec = (long) ((event->time - its.it_interval.tv_sec) * 1e9);

            if (timerfd_settime ((int) event->flags, 0, &its, NULL) == -1)
                return true;
        }
    }
    return false;
}


static bool
w_event_loop_backend_stop (w_event_loop_t *loop)
{
    w_assert (loop);

    /* Setting the structure to all zeros disarms a timer */
    struct itimerspec its;
    memset (&its, 0x00, sizeof (struct itimerspec));

    /* Disarm timers */
    w_list_foreach (i, loop->events) {
        w_event_t *event = *i;
        if (event->type == W_EVENT_TIMER) {
            if (timerfd_settime ((int) event->flags, 0, &its, NULL) == -1)
                return true;
        }
    }
    return false;
}

#elif defined(W_EVENT_BACKEND_KQUEUE)
#include <sys/event.h>

struct w_kqueue
{
    int fd;
};
typedef struct w_kqueue w_kqueue_t;


static size_t
w_event_loop_backend_size (void)
{
    return sizeof (w_kqueue_t);
}


static bool
w_event_loop_backend_init (w_event_loop_t *loop)
{
    w_kqueue_t *kq = w_obj_priv (loop, w_event_loop_t);

    w_assert (loop);
    w_assert (kq);

    /*
     * The kqueue file descript will be created lazily when an event is first
     * added to the event loop.
     */
    kq->fd = -1;
    return false;
}


static void
w_event_loop_backend_free (w_event_loop_t *loop)
{
    w_kqueue_t *kq = w_obj_priv (loop, w_event_loop_t);
    w_assert (loop);
    w_assert (kq);

    if (kq->fd >= 0)
        close (kq->fd);
}


static bool
to_kevent (struct kevent *kev, w_event_t *event)
{
    w_assert (kev);
    w_assert (event);

    int fd = -1;
    short filter = 0;

    switch (event->type) {
        case W_EVENT_IO:
        case W_EVENT_FD:
            /* ident, filter, flags, fflags, data, udata */
            if (W_HAS_FLAG (event->flags, W_EVENT_IN))
                filter |= EVFILT_READ;
            if (W_HAS_FLAG (event->flags, W_EVENT_OUT))
                filter |= EVFILT_WRITE;

            fd = (event->type == W_EVENT_IO)
               ? w_io_get_fd (event->io)
               : event->fd;
            if (fd < 0 || fd_set_nonblocking (fd))
                return true;

            EV_SET (kev, fd, filter, EV_CLEAR, 0, 0, event);
            break;

        case W_EVENT_SIGNAL:
            /* EVFILT_SIGNAL implies EV_CLEAR */
            EV_SET (kev, event->signum, EVFILT_SIGNAL, 0, 0, 0, event);
            break;

        case W_EVENT_TIMER:
            return true; /* TODO */

        case W_EVENT_IDLE:
            W_BUG ("Called with event of type E_EVENT_IDLE\n");
    }

    return false;
}


static bool
w_event_loop_backend_add (w_event_loop_t *loop, w_event_t *event)
{
    w_assert (loop);
    w_assert (event);
    w_assert (event->type != W_EVENT_IDLE);

    w_kqueue_t *kq = w_obj_priv (loop, w_event_loop_t);
    w_assert (kq);

    if (kq->fd < 0 && (kq->fd = kqueue ()) == -1)
        return true;

    struct kevent kev;
    if (to_kevent (&kev, event))
        return true;

    kev.flags |= EV_ADD;

    return kevent (kq->fd, &kev, 1, NULL, 0, NULL) == -1;
}


static bool
w_event_loop_backend_del (w_event_loop_t *loop, w_event_t *event)
{
    w_assert (loop);
    w_assert (event);

    w_kqueue_t *kq = w_obj_priv (loop, w_event_loop_t);
    w_assert (kq);

    struct kevent kev;
    if (to_kevent (&kev, event))
        return true;

    kev.flags |= EV_DELETE;

    return kevent (kq->fd, &kev, 1, NULL, 0, NULL) == -1;
}


static bool
w_event_loop_backend_start (w_event_loop_t *loop)
{
    w_assert (loop);

    /* TODO: Arm timers? */
    return false;
}


static bool
w_event_loop_backend_stop (w_event_loop_t *loop)
{
    w_assert (loop);

    /* TODO: Disarm timers? */
    return false;
}


static inline struct timespec*
to_timespec (struct timespec *ts, w_timestamp_t timeout)
{
    w_assert (ts);

    /* Add half epsilon to round the input */
    timeout += 0.5e-9;

    ts->tv_sec = (time_t) timeout;
    ts->tv_nsec = (long) (timeout - ts->tv_sec) * 1000000000L;
    return ts;
}


static bool
w_event_loop_backend_poll (w_event_loop_t *loop, w_timestamp_t timeout)
{
    w_assert (loop);

    w_kqueue_t *kq = w_obj_priv (loop, w_event_loop_t);
    w_assert (kq);
    w_assert (kq->fd >= 0);

    struct timespec ts;
    struct kevent events[W_EVENT_LOOP_NEVENTS];
    int nevents = kevent (kq->fd,
                          NULL, 0,
                          events, W_EVENT_LOOP_NEVENTS,
                          (timeout > 0.0) ? to_timespec (&ts, timeout) : NULL);

    /* TODO: Check for errors on (nevents < 0) */

    loop->now = w_timestamp_now ();

    bool stop_loop = false;
    for (int i = 0; i < nevents && !stop_loop; i++) {
        w_event_t *event = events[i].udata;
        if ((*event->callback) (loop, event))
            stop_loop = true;
    }
    return stop_loop;
}

#else
# error No events support available
#endif /* W_EVENT_BACKEND_EPOLL */
