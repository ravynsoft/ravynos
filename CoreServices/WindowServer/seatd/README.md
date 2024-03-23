# seatd and libseat

A minimal seat management daemon, and a universal seat management library.

Currently supports Linux and FreeBSD, and has experimental NetBSD support.

## What is seat management?

Seat management takes care of mediating access to shared devices (graphics, input), without requiring the applications needing access to be root.

## What's in the box?

### seatd

A seat management daemon, that does everything it needs to do. Nothing more, nothing less. Depends only on libc.

### libseat

A seat management library allowing applications to use whatever seat management is available.

Supports:
- seatd
- (e)logind
- embedded seatd for standalone operation

Each backend can be compile-time included and is runtime auto-detected or manually selected with the `LIBSEAT_BACKEND` environment variable.

Which backend is in use is transparent to the application, providing a simple common interface.

## Why not (e)logind?

systemd-logind is not portable, and being part of the systemd project, it cannot be used in an environment not based on systemd. Furthermore, "simple" is definitely not within the set of adjectives that can be used to describe logind. For those in the dark, [take a glance at its API](https://www.freedesktop.org/wiki/Software/systemd/logind/). Plus, competition is healthy.

elogind tries to isolate systemd-logind form systemd through brute-force. This requires actively fighting against upstream design decisions for deep integration, and the efforts must be repeated every time one syncs with upstream. And even after all this work, one is left with nothing but a hackjob.

Why spend time isolating logind and keeping up with upstream when we could instead create something better with less work?

## Why does libseat support (e)logind?

[In order to not be part of the problem](https://xkcd.com/927/). We will not displace systemd-logind anytime soon, so for user shells like [sway](https://github.com/swaywm/sway), seatd joins the ranks of logind and direct session management for things they need to support.

Instead of giving user shell developers more work, libseat aims to make supporting seatd less work than what they're currently implementing. This is done by taking care of all the seat management needs with multiple backends, providing not only seatd support, but replacing the existing logind and direct seat management implementations.

## How to discuss

Go to [#kennylevinsen @ irc.libera.chat](ircs://irc.libera.chat/#kennylevinsen) to discuss, or use [~kennylevinsen/seatd-devel@lists.sr.ht](https://lists.sr.ht/~kennylevinsen/seatd-devel).
