# security-context-v1 engines

This document describes how some specific engine implementations populate the
metadata in security-context-v1 and provide further metadata with out of band
mechanisms.

## [Flatpak]

* `sandbox_engine` is always set to `org.flatpak`.
* `app_id` is the Flatpak application ID (in reverse-DNS style). It is always
  set.
* `instance_id` is the Flatpak instance ID of the running sandbox. It is always
  set.

More metadata is stored in `$XDG_RUNTIME_DIR/.flatpak/$instance_id/info`. This
file will be readable when `wp_security_context_v1.commit` is called.

[Flatpak]: https://flatpak.org/
