#!/usr/bin/env bash
# shellcheck disable=SC2035
# shellcheck disable=SC2061
# shellcheck disable=SC2086 # we want word splitting

while true; do
  devcds=$(find /sys/devices/virtual/devcoredump/ -name data 2>/dev/null)
  for i in $devcds; do
    echo "Found a devcoredump at $i."
    if cp $i /results/first.devcore; then
      echo 1 > $i
      echo "Saved to the job artifacts at /first.devcore"
      exit 0
    fi
  done
  i915_error_states=$(find /sys/devices/ -path */drm/card*/error)
  for i in $i915_error_states; do
    tmpfile=$(mktemp)
    cp "$i" "$tmpfile"
    filesize=$(stat --printf="%s" "$tmpfile")
    # Does the file contain "No error state collected" ?
    if [ "$filesize" = 25 ]; then
        rm "$tmpfile"
    else
        echo "Found an i915 error state at $i size=$filesize."
        if cp "$tmpfile" /results/first.i915_error_state; then
            rm "$tmpfile"
            echo 1 > "$i"
            echo "Saved to the job artifacts at /first.i915_error_state"
            exit 0
        fi
    fi
  done
  sleep 10
done
