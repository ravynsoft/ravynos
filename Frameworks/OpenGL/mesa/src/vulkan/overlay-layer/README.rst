A Vulkan layer to display information about the running application using an overlay.

Building
=======

The overlay layer will be built if :code:`overlay` is passed as a :code:`vulkan-layers` argument. For example:

.. code-block:: sh

  meson -Dvulkan-layers=device-select,overlay builddir/
  ninja -C builddir/
  sudo ninja -C builddir/ install

See `docs/install.rst <https://gitlab.freedesktop.org/mesa/mesa/-/blob/master/docs/install.rst>`__ for more information.

Basic Usage
=======

Turn on the layer:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay /path/to/my_vulkan_app


List the available statistics:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay VK_LAYER_MESA_OVERLAY_CONFIG=help /path/to/my_vulkan_app


Turn on some statistics:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay VK_LAYER_MESA_OVERLAY_CONFIG=submit,draw,pipeline_graphics /path/to/my_vulkan_app

Position the overlay:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay VK_LAYER_MESA_OVERLAY_CONFIG=submit,draw,pipeline_graphics,position=top-right /path/to/my_vulkan_app

Logging Statistics
=======

Log statistics to a file:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay VK_LAYER_MESA_OVERLAY_CONFIG=output_file=/tmp/output.txt /path/to/my_vulkan_app

Logging is enabled for the entire lifecycle of the process unless a control socket is specified (see below).

**Note:** some statistics (e.g. :code:`frame_timing` and :code:`gpu_timing`) log values for the entire sample interval instead of per-frame.
For these statistics, logging the :code:`frame` statistic allows one to compute per-frame statistics after capture.

Log statistics to a file, controlling when such statistics will start to be captured:

.. code-block:: sh

  VK_INSTANCE_LAYERS=VK_LAYER_MESA_overlay VK_LAYER_MESA_OVERLAY_CONFIG=output_file=/tmp/output.txt,control=mesa_overlay /path/to/my_vulkan_app

The command above will open a Unix socket with the abstract path :code:`mesa_overlay`. When a control socket is specified,
logging must be explicitly enabled through the control socket. :code:`mesa-overlay-control.py` provides a convenient CLI:

.. code-block:: sh

  mesa-overlay-control.py start-capture

.. code-block:: sh

  mesa-overlay-control.py stop-capture

Direct Socket Control
------

The Unix socket may be used directly if needed. Once a client connects to the socket, the overlay layer will immediately
send the following commands to the client:

.. code-block:: sh

  :MesaOverlayControlVersion=1;
  :DeviceName=<device name>;
  :MesaVersion=<mesa version>;

The client connected to the overlay layer can enable statistics capturing by sending the command:

.. code-block:: sh

  :capture=1;

And disable it by sending:

.. code-block:: sh

  :capture=0;

.. _docs/install.rst: ../../docs/install.rst
