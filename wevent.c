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
#if defined(linux)
# define W_EVENT_BACKEND_EPOLL 1
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# define W_EVENT_BACKEND_KQUEUE 1
#else
# error There is not an event loop backend for your platform.
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <time.h>


static size_t   w_event_loop_backend_size  (void);
static w_bool_t w_event_loop_backend_init  (w_event_loop_t*);
static void     w_event_loop_backend_free  (w_event_loop_t*);
static w_bool_t w_event_loop_backend_start (w_event_loop_t*);
static w_bool_t w_event_loop_backend_stop  (w_event_loop_t*);
static w_bool_t w_event_loop_backend_add   (w_event_loop_t*, w_event_t*);
static w_bool_t w_event_loop_backend_del   (w_event_loop_t*, w_event_t*);
static w_bool_t w_event_loop_backend_poll  (w_event_loop_t*, w_timestamp_t);


static inline w_bool_t
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
    static w_bool_t use_realtime = W_YES;

    if (use_realtime) {
        struct timespec ts;
        if (clock_gettime (CLOCK_REALTIME, &ts) == 0) {
            return ts.tv_sec + ts.tv_nsec * 1e-9;
        }

        /*
         * If control reaches here, using the clock_gettime failed,
         * so mark it to not be used and fall-back to gettimeofday.
         */
        use_realtime = W_NO;
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

    switch (w_event_type (event)) {
        case W_EVENT_IO:
            w_obj_unref (event->io);
            break;
        case W_EVENT_FD:
        case W_EVENT_TIMER:
        case W_EVENT_SIGNAL:
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
            event->flags = va_arg (args, int);
            break;
        case W_EVENT_TIMER:
            event->time  = va_arg (args, w_timestamp_t);
            break;
        case W_EVENT_SIGNAL:
            event->signum = va_arg (args, int);
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

    loop->running = W_NO;
    loop->events  = w_list_new (W_YES);
    loop->now     = w_timestamp_now ();
    return w_obj_dtor (loop, _w_event_loop_destroy);
}


w_bool_t
w_event_loop_run (w_event_loop_t *loop)
{
    w_assert (loop);

    if (w_event_loop_backend_start (loop))
        return W_YES;

    loop->running = W_YES;

    /* TODO Allow control of timeouts for polling! */
    /* XXX Does that _actually_ make sense?? */
    while (loop->running)
        if (w_event_loop_backend_poll (loop, -1.0))
            loop->running = W_NO;

    return w_event_loop_backend_stop (loop);
}


void
w_event_loop_stop (w_event_loop_t *loop)
{
    w_assert (loop);
    loop->running = W_NO;
}


w_bool_t
w_event_loop_add (w_event_loop_t *loop, w_event_t *event)
{
    w_bool_t ret;

    w_assert (loop);
    w_assert (event);

    /* Adding the element to the list will w_obj_ref() it, too */
    if (!(ret = w_event_loop_backend_add (loop, event)))
        w_list_push_head (loop->events, event);

    return ret;
}


w_bool_t
w_event_loop_del (w_event_loop_t *loop, w_event_t *event)
{
    w_iterator_t i;
    w_bool_t ret;

    w_assert (loop);
    w_assert (event);

    for (i = w_list_first (loop->events); i; i = w_list_next (loop->events, i))
        if (*i == event)
            goto found;

    return W_YES;

found:
    /* Removing from the list will also w_obj_unref() the event */
    if (!(ret = w_event_loop_backend_del (loop, event)))
        w_list_del (loop->events, i);

    return ret;
}


#ifdef W_EVENT_BACKEND_EPOLL
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


static w_bool_t
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
    ep->signal_events = w_list_new (W_NO);
    return 0;
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


static w_bool_t
w_event_loop_backend_poll (w_event_loop_t *loop, w_timestamp_t timeout)
{
    w_bool_t stop_loop = W_NO;
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
            struct signalfd_siginfo si;
            w_iterator_t i;

            w_assert (ep->signal_fd >= 0);
            if (read (ep->signal_fd, &si, sizeof (struct signalfd_siginfo))
                                       != sizeof (struct signalfd_siginfo))
                /* XXX This may be too drastic... */
                abort ();

            w_list_foreach (ep->signal_events, i) {
                event = *i;
                if (si.ssi_signo == (uint32_t) event->signum)
                    if ((*event->callback) (loop, event))
                        stop_loop = W_YES;
            }
        }
        else {
            if (w_event_type (event) == W_EVENT_TIMER) {
                uint64_t expires = 0;
                /* The "flags" field is used to store a file descriptor, ugh */
                w_assert (event->flags >= 0);
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
                stop_loop = W_YES;
        }
    }
    return stop_loop;
}


static w_bool_t
w_event_loop_backend_add (w_event_loop_t *loop, w_event_t *event)
{
    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    struct epoll_event ep_ev;
    sigset_t old_sigmask;
    w_bool_t ret;
    int fd = -1;

    w_assert (loop);
    w_assert (event);
    w_assert (ep);

    if (ep->fd < 0 && (ep->fd = epoll_create1 (EPOLL_CLOEXEC)) == -1)
        return W_YES;

    ep_ev.data.ptr = event;
    ep_ev.events   = EPOLLET; /* Always use edge-triggered events */

    switch (w_event_type (event)) {
        case W_EVENT_IO:
        case W_EVENT_FD:
            fd = (w_event_type (event) == W_EVENT_IO)
               ? W_IO_UNIX_FD (event->io)
               : event->fd;

            if (W_HAS_FLAG (event->flags, W_EVENT_IN))
                ep_ev.events |= EPOLLIN;
            if (W_HAS_FLAG (event->flags, W_EVENT_OUT))
                ep_ev.events |= EPOLLOUT;

            if (fd_set_nonblocking (fd))
                return W_YES;
            break;

        case W_EVENT_SIGNAL:
            if (sigismember (&ep->signal_mask, event->signum)) {
                /* This kind of signal is already being handled */
                w_assert (ep->signal_fd >= 0);
                w_list_push_tail (ep->signal_events, event);
                return W_NO;
            }

            sigaddset (&ep->signal_mask, event->signum);
            if (sigprocmask (SIG_BLOCK, &ep->signal_mask, &old_sigmask) != 0)
                return W_YES;

            if ((fd = signalfd (ep->signal_fd, &ep->signal_mask, SFD_CLOEXEC)) == -1) {
                /*
                 * If failed to create/modify signal_fd, try to restore the old
                 * signal mask, so the masking state is left untouched on
                 * failure (if possible).
                 */
                sigprocmask (SIG_SETMASK, &old_sigmask, NULL);
                return W_YES;
            }

            ep_ev.data.ptr = W_EPOLL_SIGNAL_MARK;
            ep_ev.events   = EPOLLIN;
            ep->signal_fd  = fd;
            break;

        case W_EVENT_TIMER:
            if ((fd = event->flags = timerfd_create (CLOCK_MONOTONIC, TFD_CLOEXEC)) == -1)
                return W_YES;
            if (fd_set_nonblocking (fd)) {
                close (fd);
                return W_YES;
            }
            ep_ev.events = EPOLLIN;
            break;
    }
    w_assert (fd >= 0);

    ret = epoll_ctl (ep->fd, EPOLL_CTL_ADD, fd, &ep_ev) != 0 && errno != EEXIST;

    if (!ret && w_event_type (event) == W_EVENT_SIGNAL)
        w_list_push_tail (ep->signal_events, event);

    return ret;
}


static w_bool_t
w_event_loop_backend_del (w_event_loop_t *loop, w_event_t *event)
{
    w_epoll_t *ep = w_obj_priv (loop, w_event_loop_t);
    struct epoll_event ep_ev;
    w_iterator_t delpos = 0, i;
    sigset_t sigmask;
    int fd = -1;

    w_assert (loop);
    w_assert (event);
    w_assert (ep);
    w_assert (ep->fd >= 0);

    switch (w_event_type (event)) {
        case W_EVENT_SIGNAL:
            /* It is only needed to modify the signal mask. */
            if (ep->signal_fd < 0)
                return W_YES;
            if (!sigismember (&ep->signal_mask, event->signum))
                return W_YES;

            fd = 0;
            w_list_foreach (ep->signal_events, i) {
                if (((w_event_t *) *i)->signum == event->signum) fd++;
                if (event == *i) delpos = i;
            }

            /* Last event for this signal, modify signalfd */
            if (fd == 1) {
                sigemptyset (&sigmask);
                sigaddset (&sigmask, event->signum);
                sigdelset (&ep->signal_mask, event->signum);
                if (sigprocmask (SIG_UNBLOCK, &sigmask, NULL) != 0)
                    return W_YES;
                if (signalfd (ep->signal_fd, &ep->signal_mask, O_CLOEXEC) == -1)
                    return W_YES;
            }
            w_list_del (ep->signal_events, delpos);
            return W_NO;

        case W_EVENT_FD:
            fd = event->fd;
            break;

        case W_EVENT_IO:
            fd = W_IO_UNIX_FD (event->io);
            break;

        case W_EVENT_TIMER:
            /* The "flags" attribute is being used to store the fd. */
            /* TODO/FIXME close the timer fd */
            w_assert (event->flags >= 0);
            fd = (int) event->flags;
            break;
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


static w_bool_t
w_event_loop_backend_start (w_event_loop_t *loop)
{
    w_iterator_t i;
    w_assert (loop);

    /* Arm timers */
    w_list_foreach (loop->events, i) {
        w_event_t *event = *i;
        if (w_event_type (event) == W_EVENT_TIMER) {
            struct itimerspec its;

            its.it_value.tv_sec = 0;
            its.it_value.tv_nsec = 1;
            its.it_interval.tv_sec = (time_t) floor (event->time);
            its.it_interval.tv_nsec = (long) ((event->time - its.it_interval.tv_sec) * 1e9);

            if (event->flags < 0 || timerfd_settime (event->flags, 0, &its, NULL) == -1)
                return W_YES;
        }
    }

    return W_NO;
}


static w_bool_t
w_event_loop_backend_stop (w_event_loop_t *loop)
{
    struct itimerspec its;
    w_iterator_t i;

    w_assert (loop);

    /* Setting the structure to all zeros disarms a timer */
    memset (&its, 0x00, sizeof (struct itimerspec));

    /* Disarm timers */
    w_list_foreach (loop->events, i) {
        w_event_t *event = *i;
        if (w_event_type (event) == W_EVENT_TIMER) {
            if (event->flags < 0 || timerfd_settime (event->flags, 0, &its, NULL) == -1)
                return W_YES;
        }
    }

    return W_NO;
}

#endif /* W_EVENT_BACKEND_EPOLL */
