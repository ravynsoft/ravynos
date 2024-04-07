package HAS_ATTACH;

sub STORABLE_attach {
  ++$attached_count;
  return bless [], 'HAS_ATTACH';
}

++$loaded_count;

1;
