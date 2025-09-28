There are three test data directories:

- 'data-test': These files are valid META files that test *specific*
  conversions and are expected to have specific data in them during
  testing. Do not put new test data here unless you are sure it meets all
  requirements needed to pass.

- 'data-valid': These files are valid META files.  Some may be improved by
  the Converter (particularly upconverting from ancient specs).

- 'data-fixable': These files are bad META files that fail validation, but
  can be fixed via the Converter.

- 'data-fail': These files are bad META files that fail validation and
  can't be fixed.
