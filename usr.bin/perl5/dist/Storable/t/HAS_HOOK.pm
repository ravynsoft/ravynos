package HAS_HOOK;

sub STORABLE_thaw {
  ++$thawed_count;
}

++$loaded_count;

1;
