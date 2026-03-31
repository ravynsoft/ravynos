#compdef invoke-rc.d

_arguments \
  '--quiet[quiet mode, no error messages are generated]' \
  '--force[run the initscript regardless of policy and subsystem]' \
  '--try-anyway[run the initscript even if a non-fatal error is found]' \
  '--disclose-deny[return 101 instead of 0 if action is denied]' \
  '--query[return one of status codes 100-106, does not run the script]' \
  '--no-fallback[ignore any fallback action requests by the policy layer]' \
  '1:service:_services' \
  '2:command:(start stop force-stop restart reload force-reload status)'
