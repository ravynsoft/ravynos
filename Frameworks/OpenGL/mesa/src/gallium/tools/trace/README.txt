These directory contains tools for manipulating traces produced by the trace
pipe driver.


Most debug builds of gallium frontends already load the trace driver by default.
To produce a trace do

  export GALLIUM_TRACE=foo.gtrace

and run the application.  You can choose any name, but the .gtrace is
recommended to avoid confusion with the .trace produced by apitrace.


You can dump a trace by doing

  ./dump.py foo.gtrace | less


You can dump a JSON file describing the static state at any given draw call
(e.g., 12345) by
doing

  ./dump_state.py -v -c 12345 foo.gtrace > foo.json

or by specifying the n-th (e.g, 1st) draw call by doing

  ./dump_state.py -v -d 1 foo.gtrace > foo.json

The state is derived from the call sequence in the trace file, so no dynamic
(eg. rendered textures) is included.


You can compare two JSON files by doing

  ./diff_state.py foo.json boo.json | less

If you're investigating a regression in an gallium frontend, you can obtain a good
and bad trace, dump respective state in JSON, and then compare the states to
identify the problem.
