Gpuvis Tracing Annotations
==========================

`Gpuvis <https://github.com/mikesart/gpuvis>`__ is a tool to visualize ftrace
traces, with support for most GPU drivers to show GPU activity.

Mesa can emit trace markers to be displayed in Gpuvis, which is useful for
figuring out why e.g. stalls are happening.

Run on Linux
------------

Any traces can be made with trace-cmd. The Gpuvis repository contains
`scripts <https://github.com/mikesart/gpuvis/tree/master/sample>`__ for
configuring the markers needed for GPU events. To start tracing:

.. code-block:: console

   sh trace-cmd-setup.sh && sh trace-cmd-start-tracing.sh
   # Start your game etc. Then to capture a trace
   sh trace-cmd-capture.sh
   # and to finally stop
   sh trace-cmd-stop-tracing.sh

The resulting trace file can be opened with Gpuvis.

Run on Steamos
--------------

Steamos includes a script (`gpu-trace <https://github.com/lostgoat/gpu-trace>`__)
to capture traces.

.. code-block:: console

   sudo gpu-trace
   # Press Ctrl+C to stop capture and open report in gpuvis

Note that on Steamos gpuvis is actually not installed by default, but the
script does write a gpu-trace.zip file in the current working directory. To
open it you'll want to do the following on your development machine:

.. code-block:: console

   scp sd-host:gpu-trace.zip ./
   unzip gpu-trace.zip
   ./trace-cmd convert -i *.dat -o converted.dat --compression none --file-version 6
   gpuvis *.json converted.dat

The main advantage of the gpu-trace script is that it has an integration with
perf, so Gpuvis can also visualize perf samples in the timeline. Note that the
perf file is preprocessed before being put into the zip file, so the trace is
portable between machines.


Annotations in Steam games
--------------------------

Steam games are run in a container, and need the environment variable
PRESSURE_VESSEL_DEVEL=1 to set up the tracing filesystem so the trace marker
can be written. This can e.g. be done by going to the game properties in
Steam and setting the command line to

.. code-block:: console

   PRESSURE_VESSEL_DEVEL=1 %command%

Then you can run the game as usual, and the driver will write trace annotations.