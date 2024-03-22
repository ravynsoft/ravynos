ANV
===

Debugging
---------

Here are a few environment variable debug environment variables
specific to ANV:

:envvar:`ANV_ENABLE_PIPELINE_CACHE`
   If defined to ``0`` or ``false``, this will disable pipeline
   caching, forcing ANV to reparse and recompile any VkShaderModule
   (SPIRV) it is given.
:envvar:`ANV_DISABLE_SECONDARY_CMD_BUFFER_CALLS`
   If defined to ``1`` or ``true``, this will prevent usage of self
   modifying command buffers to implement ``vkCmdExecuteCommands``. As
   a result of this, it will also disable :ext:`VK_KHR_performance_query`.
:envvar:`ANV_ALWAYS_BINDLESS`
   If defined to ``1`` or ``true``, this forces all descriptor sets to
   use the internal `Bindless model`_.
:envvar:`ANV_QUEUE_THREAD_DISABLE`
   If defined to ``1`` or ``true``, this disables support for timeline
   semaphores.
:envvar:`ANV_USERSPACE_RELOCS`
   If defined to ``1`` or ``true``, this forces ANV to always do
   kernel relocations in command buffers. This should only have an
   effect on hardware that doesn't support soft-pinning (Ivybridge,
   Haswell, Cherryview).
:envvar:`ANV_PRIMITIVE_REPLICATION_MAX_VIEWS`
   Specifies up to how many view shaders can be lowered to handle
   :ext:`VK_KHR_multiview`. Beyond this number, multiview is implemented
   using instanced rendering. If unspecified, the value default to
   ``2``.


Experimental features
---------------------

.. _`Bindless model`:

Binding Model
-------------

Here is the ANV bindless binding model that was implemented for the
descriptor indexing feature of Vulkan 1.2 :

.. graphviz::

  digraph G {
    fontcolor="black";
    compound=true;

    subgraph cluster_1 {
      label = "Binding Table (HW)";

      bgcolor="cornflowerblue";

      node [ style=filled,shape="record",fillcolor="white",
             label="RT0"    ] n0;
      node [ label="RT1"    ] n1;
      node [ label="dynbuf0"] n2;
      node [ label="set0"   ] n3;
      node [ label="set1"   ] n4;
      node [ label="set2"   ] n5;

      n0 -> n1 -> n2 -> n3 -> n4 -> n5 [style=invis];
    }
    subgraph cluster_2 {
      label = "Descriptor Set 0";

      bgcolor="burlywood3";
      fixedsize = true;

      node [ style=filled,shape="record",fillcolor="white", fixedsize = true, width=4,
             label="binding 0 - STORAGE_IMAGE\n anv_storage_image_descriptor"          ] n8;
      node [ label="binding 1 - COMBINED_IMAGE_SAMPLER\n anv_sampled_image_descriptor" ] n9;
      node [ label="binding 2 - UNIFORM_BUFFER\n anv_address_range_descriptor"         ] n10;
      node [ label="binding 3 - UNIFORM_TEXEL_BUFFER\n anv_storage_image_descriptor"   ] n11;

      n8 -> n9 -> n10 -> n11 [style=invis];
    }
    subgraph cluster_5 {
      label = "Vulkan Objects"

      fontcolor="black";
      bgcolor="darkolivegreen4";

      subgraph cluster_6 {
        label = "VkImageView";

        bgcolor=darkolivegreen3;
        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="surface_state" ] n12;
      }
      subgraph cluster_7 {
        label = "VkSampler";

        bgcolor=darkolivegreen3;
        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="sample_state" ] n13;
      }
      subgraph cluster_8 {
        label = "VkImageView";
        bgcolor="darkolivegreen3";

        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="surface_state" ] n14;
      }
      subgraph cluster_9 {
        label = "VkBuffer";
        bgcolor=darkolivegreen3;

        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="address" ] n15;
      }
      subgraph cluster_10 {
        label = "VkBufferView";

        bgcolor=darkolivegreen3;
        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="surface_state" ] n16;
      }

      n12 -> n13 -> n14 -> n15 -> n16 [style=invis];
    }

    subgraph cluster_11 {
      subgraph cluster_12 {
        label = "CommandBuffer state stream";

        bgcolor="gold3";
        node [ style=filled,shape="box",fillcolor="white", fixedsize = true, width=2,
               label="surface_state" ] n17;
        node [ label="surface_state" ] n18;
        node [ label="surface_state" ] n19;

        n17 -> n18 -> n19 [style=invis];
      }
    }

    n3  -> n8 [lhead=cluster_2];

    n8  -> n12;
    n9  -> n13;
    n9  -> n14;
    n10 -> n15;
    n11 -> n16;

    n0 -> n17;
    n1 -> n18;
    n2 -> n19;
  }



The HW binding table is generated when the draw or dispatch commands
are emitted. Here are the types of entries one can find in the binding
table :

- The currently bound descriptor sets, one entry per descriptor set
  (our limit is 8).

- For dynamic buffers, one entry per dynamic buffer.

- For draw commands, render target entries if needed.

The entries of the HW binding table for descriptor sets are
RENDER_SURFACE_STATE similar to what you would have for a normal
uniform buffer. The shader will emit reads this buffer first to get
the information it needs to access a surface/sampler/etc... and then
emits the appropriate message using the information gathered from the
descriptor set buffer.

Each binding type entry gets an associated structure in memory
(``anv_storage_image_descriptor``, ``anv_sampled_image_descriptor``,
``anv_address_range_descriptor``, ``anv_storage_image_descriptor``).
This is the information read by the shader.


.. _`Binding tables`:

Binding Tables
--------------

Binding tables are arrays of 32bit offset entries referencing surface
states. This is how shaders can refer to binding table entry to read
or write a surface. For example fragment shaders will often refer to
entry 0 as the first render target.

The way binding tables are managed is fairly awkward.

Each shader stage must have its binding table programmed through
a corresponding instruction
``3DSTATE_BINDING_TABLE_POINTERS_*`` (each stage has its own).

.. graphviz::

  digraph structs {
    node [shape=record];
    struct3 [label="{ binding tables&#92;n area | { <bt4> BT4 | <bt3> BT3 | ... | <bt0> BT0 } }|{ surface state&#92;n area |{<ss0> ss0|<ss1> ss1|<ss2> ss2|...}}"];
    struct3:bt0 -> struct3:ss0;
    struct3:bt0 -> struct3:ss1;
  }


The value programmed in the ``3DSTATE_BINDING_TABLE_POINTERS_*``
instructions is not a 64bit pointer but an offset from the address
programmed in ``STATE_BASE_ADDRESS::Surface State Base Address`` or
``3DSTATE_BINDING_TABLE_POOL_ALLOC::Binding Table Pool Base Address``
(available on Gfx11+). The offset value in
``3DSTATE_BINDING_TABLE_POINTERS_*`` is also limited to a few bits
(not a full 32bit value), meaning that as we use more and more binding
tables we need to reposition ``STATE_BASE_ADDRESS::Surface State Base
Address`` to make space for new binding table arrays.

To make things even more awkward, the binding table entries are also
relative to ``STATE_BASE_ADDRESS::Surface State Base Address`` so as
we change ``STATE_BASE_ADDRESS::Surface State Base Address`` we need
add that offsets to the binding table entries.

The way with deal with this is that we allocate 4Gb of address space
(since the binding table entries can address 4Gb of surface state
elements). We reserve the first gigabyte exclusively to binding
tables, so that anywhere we position our binding table in that first
gigabyte, it can always refer to the surface states in the next 3Gb.


.. _`Descriptor Set Memory Layout`:

Descriptor Set Memory Layout
----------------------------

Here is a representation of how the descriptor set bindings, with each
elements in each binding is mapped to a the descriptor set memory :

.. graphviz::

  digraph structs {
    node [shape=record];
    rankdir=LR;

    struct1 [label="Descriptor Set | \
                    <b0> binding 0\n STORAGE_IMAGE \n (array_length=3) | \
                    <b1> binding 1\n COMBINED_IMAGE_SAMPLER \n (array_length=2) | \
                    <b2> binding 2\n UNIFORM_BUFFER \n (array_length=1) | \
                    <b3> binding 3\n UNIFORM_TEXEL_BUFFER \n (array_length=1)"];
    struct2 [label="Descriptor Set Memory | \
                    <b0e0> anv_storage_image_descriptor|\
                    <b0e1> anv_storage_image_descriptor|\
                    <b0e2> anv_storage_image_descriptor|\
                    <b1e0> anv_sampled_image_descriptor|\
                    <b1e1> anv_sampled_image_descriptor|\
                    <b2e0> anv_address_range_descriptor|\
                    <b3e0> anv_storage_image_descriptor"];

    struct1:b0 -> struct2:b0e0;
    struct1:b0 -> struct2:b0e1;
    struct1:b0 -> struct2:b0e2;
    struct1:b1 -> struct2:b1e0;
    struct1:b1 -> struct2:b1e1;
    struct1:b2 -> struct2:b2e0;
    struct1:b3 -> struct2:b3e0;
  }

Each Binding in the descriptor set is allocated an array of
``anv_*_descriptor`` data structure. The type of ``anv_*_descriptor``
used for a binding is selected based on the ``VkDescriptorType`` of
the bindings.

The value of ``anv_descriptor_set_binding_layout::descriptor_offset``
is a byte offset from the descriptor set memory to the associated
binding. ``anv_descriptor_set_binding_layout::array_size`` is the
number of ``anv_*_descriptor`` elements in the descriptor set memory
from that offset for the binding.


Pipeline state emission
-----------------------

Vulkan initially started by baking as much state as possible in
pipelines. But extension after extension, more and more state has
become potentially dynamic.

ANV tries to limit the amount of time an instruction has to be packed
to reprogram part of the 3D pipeline state. The packing is happening
in 2 places :

- ``genX_pipeline.c`` where the non dynamic state is emitted in the
  pipeline batch. Chunks of the batches are copied into the command
  buffer as a result of calling ``vkCmdBindPipeline()``, depending on
  what changes from the previously bound graphics pipeline

- ``genX_gfx_state.c`` where the dynamic state is added to already
  packed instructions from ``genX_pipeline.c``

The rule to know where to emit an instruction programming the 3D
pipeline is as follow :

- If any field of the instruction can be made dynamic, it should be
  emitted in ``genX_gfx_state.c``

- Otherwise, the instruction can be emitted in ``genX_pipeline.c``

When a piece of state programming is dynamic, it should have a
corresponding field in ``anv_gfx_dynamic_state`` and the
``genX(cmd_buffer_flush_gfx_runtime_state)`` function should be
updated to ensure we minimize the amount of time an instruction should
be emitted. Each instruction should have a associated
``ANV_GFX_STATE_*`` mask so that the dynamic emission code can tell
when to re-emit an instruction.


Generated indirect draws optimization
-------------------------------------

Indirect draws have traditionally been implemented on Intel HW by
loading the indirect parameters from memory into HW registers using
the command streamer's ``MI_LOAD_REGISTER_MEM`` instruction before
dispatching a draw call to the 3D pipeline.

On recent products, it was found that the command streamer is showing
as performance bottleneck, because it cannot dispatch draw calls fast
enough to keep the 3D pipeline busy.

The solution to this problem is to change the way we deal with
indirect draws. Instead of loading HW registers with values using the
command streamer, we generate entire set of ``3DPRIMITIVE``
instructions using a shader. The generated instructions contain the
entire draw call parameters. This way the command streamer executes
only ``3DPRIMITIVE`` instructions and doesn't do any data loading from
memory or touch HW registers, feeding the 3D pipeline as fast as it
can.

In ANV this implemented in 2 different ways :

By generating instructions directly into the command stream using a
side batch buffer. When ANV encounters the first indirect draws, it
generates a jump into the side batch, the side batch contains a draw
call using a generation shader for each indirect draw. We keep adding
on more generation draws into the batch until we have to stop due to
command buffer end, secondary command buffer calls or a barrier
containing the access flag ``VK_ACCESS_INDIRECT_COMMAND_READ_BIT``.
The side batch buffer jump back right after the instruction where it
was called. Here is a high level diagram showing how the generation
batch buffer writes in the main command buffer :

.. graphviz::

  digraph commands_mode {
    rankdir = "LR"
    "main-command-buffer" [
      label = "main command buffer|...|draw indirect0 start|<f0>jump to\ngeneration batch|<f1>|<f2>empty instruction0|<f3>empty instruction1|...|draw indirect0 end|...|draw indirect1 start|<f4>empty instruction0|<f5>empty instruction1|...|<f6>draw indirect1 end|..."
      shape = "record"
    ];
    "generation-command-buffer" [
      label = "generation command buffer|<f0>|<f1>write draw indirect0|<f2>write draw indirect1|...|<f3>exit jump"
      shape = "record"
    ];
    "main-command-buffer":f0 -> "generation-command-buffer":f0;
    "generation-command-buffer":f1 -> "main-command-buffer":f2 [color="#0000ff"];
    "generation-command-buffer":f1 -> "main-command-buffer":f3 [color="#0000ff"];
    "generation-command-buffer":f2 -> "main-command-buffer":f4 [color="#0000ff"];
    "generation-command-buffer":f2 -> "main-command-buffer":f5 [color="#0000ff"];
    "generation-command-buffer":f3 -> "main-command-buffer":f1;
  }

By generating instructions into a ring buffer of commands, when the
draw count number is high. This solution allows smaller batches to be
emitted. Here is a high level diagram showing how things are
executed :

.. graphviz::

  digraph ring_mode {
    rankdir=LR;
    "main-command-buffer" [
      label = "main command buffer|...| draw indirect |<f1>generation shader|<f2> jump to ring|<f3> increment\ndraw_base|<f4>..."
      shape = "record"
    ];
    "ring-buffer" [
      label = "ring buffer|<f0>generated draw0|<f1>generated draw1|<f2>generated draw2|...|<f3>exit jump"
      shape = "record"
    ];
    "main-command-buffer":f2 -> "ring-buffer":f0;
    "ring-buffer":f3 -> "main-command-buffer":f3;
    "ring-buffer":f3 -> "main-command-buffer":f4;
    "main-command-buffer":f3 -> "main-command-buffer":f1;
    "main-command-buffer":f1 -> "ring-buffer":f1 [color="#0000ff"];
    "main-command-buffer":f1 -> "ring-buffer":f2 [color="#0000ff"];
  }
