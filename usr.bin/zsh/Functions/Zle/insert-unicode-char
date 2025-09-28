# Make hex integers appear as 0x...
setopt localoptions cbases

integer -g _insert_unicode_ready

if [[ $LASTWIDGET = insert-unicode-char && $_insert_unicode_ready -eq 1 ]]
then
  # Second call; we should have a usable prefix.
  # If we don't, give up.
  (( ${+NUMERIC} )) || return 1
  # Convert it back to hex, padded with zeroes to 8 digits plus the 0x...
  local -i 16 -Z 10 arg=$NUMERIC
  # ...and use print to turn this into a Unicode character.
  LBUFFER+="$(print -n "\U${arg##0x}")"
  integer -g _insert_unicode_ready=0
else
  # Set the base to 16...
  zle argument-base 16
  # ...wait for user to type hex keys then call this widget again.
  zle universal-argument
  integer -g _insert_unicode_ready=1
fi
