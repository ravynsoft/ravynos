# epoll-shim

This is a small library that implements epoll on top of kqueue.
It has been successfully used to port libinput, libevdev, Wayland and more
software to FreeBSD: <https://www.freshports.org/devel/libepoll-shim/>

It may be useful for porting other software that uses epoll as well.

There are some tests inside `test/`. They should also compile under Linux and
can be used to verify proper epoll behavior.

Sadly, this library contains some very ugly hacks and workarounds. For example:

- When using `timerfd`, `signalfd` or `eventfd`, the system calls `read`,
  `write` and `close` are redefined as macros to internal helper functions.
  This is needed as there is some internal context that has to be free'd
  properly. This means that you shouldn't create a `timerfd`/`signalfd` in
  one part of a program and close it in a different part where
  `sys/timerfd.h` isn't included. The context would leak. Luckily, software
  such as libinput behaves very nicely and puts all `timerfd` related code in
  a single source file.

  Alternatively, a target/library `epoll-shim-interpose` is also provided.
  Instead of redefining those symbols as macros they are provided as "proper"
  symbols, making use of POSIX `dlsym` chaining with `RTLD_NEXT`.

  What approach is more suitable depends on the application: If the use of
  `epoll` is very localized the macro based approach is less overhead. If the
  use of those file descriptors is more pervasive, the interposition approach
  is more robust. It will be a bit less performant because all calls to
  `read`/`write`/`close` and so on will be routed through `epoll-shim`.

- There is limited support for file descriptors that lack support for
  kqueue but are supported by `poll(2)`. This includes graphics or sound
  devices under `/dev`. Those descriptors are handled in an outer `poll(2)`
  loop. Edge triggering using `EPOLLET` will not work.

- Shimmed file descriptors cannot be shared between processes. On `fork()`
  those fds are closed. When trying to pass a shimmed fd to another process the
  `sendmsg` call will return `EOPNOTSUPP`. In most cases sharing
  `epoll`/`timerfd`/`signalfd` is a bad idea anyway, but there are some
  legitimate use cases (for example sharing semaphore `eventfd`s, issue #23).
  When the OS natively supports `eventfd`s (as is the case for FreeBSD >= 13)
  this library won't provide `eventfd` shims or the `sys/eventfd.h` header.

- There is no proper notification mechanism for changes to the system
  `CLOCK_REALTIME` clock on BSD systems. Also, `kevent` `EVFILT_TIMER`s use the
  system monotonic clock as reference. Therefore, in order to implement
  absolute (`TFD_TIMER_ABSTIME`) `CLOCK_REALTIME` `timerfd`s or cancellation
  support (`TFD_TIMER_CANCEL_ON_SET`), a thread is spawned that periodically
  polls the system boot time for changes to the realtime clock.

The library is tested on the following operating systems:

- FreeBSD 12.3, 13.1
- NetBSD 9.1, -current 2022-03-06
- OpenBSD 7.1
- DragonFlyBSD 6.2.2
- macOS 12.5

Be aware of some subtle kqueue bugs that may affect the emulated
epoll behavior. I've marked tests that hit those behaviors as "skipped".
Have a look at `atf_tc_skip()` calls in the tests.

## Installation

Run the following commands to build libepoll-shim:

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build .

To run the tests:

    ctest --output-on-failure

To install (as root):

    cmake --build . --target install

## Changelog

### 2023-04-11

- Add support for FreeBSD's `POLLRDHUP`.
- Use `CMAKE_INSTALL_FULL_*` in `*.pc` files (thanks alyssais!).

### 2023-01-28

- Initial port to macOS.

### 2022-07-03

- Fix crashes when `libepoll-shim.so` is placed after `libc.so` in library
  search order.
- Test on OpenBSD 7.1.
- Fix memory leak in tests (thanks arichardson!).

### 2022-06-07

- Introduce `epoll-shim-interpose` library. This library provides proper
  wrapper symbols for `read`/`write`/`close`/`poll`/`ppoll`/`fcntl`. If for
  some reason the macro based approach of redefining those symbols is not
  appropriate, using this library instead of `epoll-shim` might be an
  alternative.
- More faithful simulation of file descriptor semantics, including reference
  counting.
- Faster file descriptor lookup, using an array instead of a tree data
  structure.
- Define wrapper macros as variadic, except when ANSI C is used.

### 2021-04-18

- Fix compiler warning when using shimmed `fcntl`.

### 2021-04-17

- Allow setting `O_NONBLOCK` flag with `fcntl` on created file descriptors.
- Implement `TFD_TIMER_CANCEL_ON_SET` for `timerfd`.
- Implement correction of absolute (`TFD_TIMER_ABSTIME`) `CLOCK_REALTIME`
  `timerfd`s when the system time is stepped.

### 2021-03-22

- Fix compilation on FreeBSD < 12 (#28).

### 2021-03-21

- Add `O_CLOEXEC` handling to created file descriptors (PR #26, thanks
  arichardson!). Note that the shimmed file descriptors still won't work
  correctly after `exec(3)`. Therefore, *not* using
  `EPOLL_CLOEXEC`/`TFD_CLOEXEC`/`SFD_CLOEXEC`/`EFD_CLOEXEC` is strongly
  discouraged.

### 2021-03-10

- Fix compilation on FreeBSD 12.1 (#25).

### 2021-02-13

- `signalfd` now hooks into the signal disposition mechanism, just like on
  Linux. Note: `poll` and `ppoll` are also shimmed with macros in case
  `sys/signalfd.h` is included to support some use cases seen in the wild. Many
  more `ssi_*` fields are now set on the resulting `struct signalfd_siginfo`.
- More accurate timeout calculations for `epoll_wait`/`poll`/`ppoll`.
- Fix integer overflow on timerfd timeout field on 32-bit machines.
- Fix re-arming of timerfd timeouts on BSDs where EV_ADD of a EVFILT_TIMER
  doesn't do it.

### 2020-12-29

- Add support for native `eventfd`s (provided by FreeBSD >= 13). The
  `sys/eventfd.h` header will not be installed in this case.

### 2020-11-06

- Add support for NetBSD 9.1.

### 2020-06-02

- On FreeBSD, add missing `sys/signal.h` include that resulted in `sigset_t`
  errors (#21).

### 2020-04-25

- Lift limit of 32 descriptors in `epoll_wait(2)`.
- Implement `EPOLLPRI` using `EVFILT_EXCEPT`, if available. If it is not
  available, add logic to `EVFILT_READ` handling that will work if
  `SO_OOBINLINE` is set on the socket.
- Implement `EPOLLONESHOT`.
- Implement edge triggering with `EPOLLET`.
- Add support for unlimited numbers of poll-only fds per epoll instance.
- Merge `EVFILT_READ`/`EVFILT_WRITE` events together to more closely match
  epoll semantics.
- Add support for NetBSD, OpenBSD and DragonFlyBSD.

### 2020-04-08

- Implement `epoll_pwait(2)`.
