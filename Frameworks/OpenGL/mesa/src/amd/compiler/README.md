# Welcome to ACO

ACO (short for *AMD compiler*) is a back-end compiler for AMD GCN / RDNA GPUs, based on the NIR compiler infrastructure.
Simply put, ACO translates shader programs from the NIR intermediate representation into a GCN / RDNA binary which the GPU can execute.

## Motivation

Why did we choose to develop a new compiler backend?

1. We'd like to give gamers a fluid, stutter-free experience, so we prioritize compilation speed.
2. Good divergence analysis allows us to better optimize runtime performance.
3. Issues can be fixed within mesa releases, independently of the schedule of other projects.

## Control flow

Modern GPUs are SIMD machines that execute the shader in parallel.
In case of GCN / RDNA the parallelism is achieved by executing the shader on several waves, and each wave has several lanes (32 or 64).
When every lane executes exactly the same instructions, and takes the same path, it's uniform control flow;
otherwise when some lanes take one path while other lanes take a different path, it's divergent.

Each hardware lane corresponds to a shader invocation from a software perspective.

The hardware doesn't directly support divergence,
so in case of divergent control flow, the GPU must execute both code paths, each with some lanes disabled.
This is why divergence is a performance concern in shader programming.

ACO deals with divergent control flow by maintaining two control flow graphs (CFG):

* logical CFG - directly translated from NIR and shows the intended control flow of the program.
* linear CFG - created according to Whole-Function Vectorization by Ralf Karrenberg and Sebastian Hack.
  The linear CFG represents how the program is physically executed on GPU and may contain additional blocks for control flow handling and to avoid critical edges.
  Note that all nodes of the logical CFG also participate in the linear CFG, but not vice versa.

## Compilation phases

#### Instruction Selection

The instruction selection is based around the divergence analysis and works in 3 passes on the NIR shader.

1. The divergence analysis pass calculates for each SSA definition if its value is guaranteed to be uniform across all threads of the workgroup.
2. We determine the register class for each SSA definition.
3. Actual instruction selection. The advanced divergence analysis allows for better usage of the scalar unit, scalar memory loads and the scalar register file.

We have two types of instructions:

* Hardware instructions as specified by the GCN / RDNA instruction set architecture manuals.
* Pseudo instructions which are helpers that encapsulate more complex functionality.
  They eventually get lowered to real hardware instructions.

Each instruction can have operands (temporaries that it reads), and definitions (temporaries that it writes).
Temporaries can be fixed to a specific register, or just specify a register class (either a single register, or a vector of several registers).

#### Value Numbering

The value numbering pass is necessary for two reasons: the lack of descriptor load representation in NIR,
and every NIR instruction that gets emitted as multiple ACO instructions also has potential for CSE.
This pass does dominator-tree value numbering.

#### Optimization

In this phase, simpler instructions are combined into more complex instructions (like the different versions of multiply-add as well as neg, abs, clamp, and output modifiers) and constants are inlined, moves are eliminated, etc.
Exactly which optimizations are performed depends on the hardware for which the shader is being compiled.

#### Setup of reduction temporaries

This pass is responsible for making sure that register allocation is correct for reductions, by adding pseudo instructions that utilize linear VGPRs.
When a temporary has a linear VGPR register class, this means that the variable is considered *live* in the linear control flow graph.

#### Insert exec mask

In the GCN/RDNA architecture, there is a special register called `exec` which is used for manually controlling which VALU threads (aka. *lanes*) are active. The value of `exec` has to change in divergent branches, loops, etc. and it needs to be restored after the branch or loop is complete. This pass ensures that the correct lanes are active in every branch.

#### Live-Variable Analysis

A live-variable analysis is used to calculate the register need of the shader.
This information is used for spilling and scheduling before register allocation.

#### Spilling

First, we lower the shader program to CSSA form.
Then, if the register demand exceeds the global limit, this pass lowers register usage by temporarily storing excess scalar values in free vector registers, or excess vector values in scratch memory, and reloading them when needed. It is based on the paper "Register Spilling and Live-Range Splitting for SSA-Form Programs".

#### Instruction Scheduling

Scheduling is another NP-complete problem where basically all known heuristics suffer from unpredictable change in register pressure. For that reason, the implemented scheduler does not completely re-schedule all instructions, but only aims to move up memory loads as far as possible without exceeding the maximum register limit for the pre-calculated wave count. The reason this works is that ILP is very limited on GCN. This approach looks promising so far.

#### Register Allocation

The register allocator works on SSA (as opposed to LLVM's which works on virtual registers). The SSA properties guarantee that there are always as many registers available as needed. The problem is that some instructions require a vector of neighboring registers to be available, but the free regs might be scattered. In this case, the register allocator inserts shuffle code (moving some temporaries to other registers) to make space for the variable. The assumption is that it is (almost) always better to have a few more moves than to sacrifice a wave. The RA does SSA-reconstruction on the fly, which makes its runtime linear.

#### SSA Elimination

The next step is a pass out of SSA by inserting parallelcopies at the end of blocks to match the phi nodes' semantics.

#### Lower to HW instructions

Most pseudo instructions are lowered to actual machine instructions.
These are mostly parallel copy instructions created by instruction selection or register allocation and spill/reload code.

#### ILP Scheduling

This second scheduler works on registers rather than SSA-values to determine dependencies. It implements a forward list scheduling algorithm using a partial dependency graph of few instructions at a time and aims to create larger memory clauses and improve ILP.

#### Insert wait states

GCN requires some wait states to be manually inserted in order to ensure correct behavior on memory instructions and some register dependencies.
This means that we need to insert `s_waitcnt` instructions (and its variants) so that the shader program waits until the eg. a memory operation is complete.

#### Resolve hazards and insert NOPs

Some instructions require wait states or other instructions to resolve hazards which are not handled by the hardware.
This pass makes sure that no known hazards occur.

#### Emit program - Assembler

The assembler emits the actual binary that will be sent to the hardware for execution. ACO's assembler is straight-forward because all instructions have their format, opcode, registers and potential fields already available, so it only needs to cater to the some differences between each hardware generation.

## Supported shader stages

Hardware stages (as executed on the chip) don't exactly match software stages (as defined in OpenGL / Vulkan).
Which software stage gets executed on which hardware stage depends on what kind of software stages are present in the current pipeline.

An important difference is that VS is always the first stage to run in SW models,
whereas HW VS refers to the last HW stage before fragment shading in GCN/RDNA terminology.
That's why, among other things, the HW VS is no longer used to execute the SW VS when tessellation or geometry shading are used.

#### Glossary of software stages

* VS = Vertex Shader
* TCS = Tessellation Control Shader, equivalent to D3D HS = Hull Shader
* TES = Tessellation Evaluation Shader, equivalent to D3D DS = Domain Shader
* GS = Geometry Shader
* FS = Fragment Shader, equivalent to D3D PS = Pixel Shader
* CS = Compute Shader
* TS = Task Shader
* MS = Mesh Shader

#### Glossary of hardware stages

* LS = Local Shader (merged into HS on GFX9+), only runs SW VS when tessellation is used
* HS = Hull Shader, the HW equivalent of a Tessellation Control Shader, runs before the fixed function hardware performs tessellation
* ES = Export Shader (merged into GS on GFX9+), if there is a GS in the SW pipeline, the preceding stage (ie. SW VS or SW TES) always has to run on this HW stage
* GS = Geometry Shader, also known as legacy GS
* VS = Vertex Shader, **not equivalent to SW VS**: when there is a GS in the SW pipeline this stage runs a "GS copy" shader, otherwise it always runs the SW stage before FS
* NGG = Next Generation Geometry, a new hardware stage that replaces legacy HW GS and HW VS on RDNA GPUs
* PS = Pixel Shader, the HW equivalent to SW FS
* CS = Compute Shader

##### Notes about HW VS and the "GS copy" shader

HW PS reads its inputs from a special ring buffer called Parameter Cache (PC) that only HW VS can write to, using export instructions.
However, legacy GS store their output in VRAM (before GFX10/NGG).
So in order for HW PS to be able to read the GS outputs, we must run something on the VS stage which reads the GS outputs
from VRAM and exports them to the PC. This is what we call a "GS copy" shader.
From a HW perspective the "GS copy" shader is in fact VS (it runs on the HW VS stage),
but from a SW perspective it's not part of the traditional pipeline,
it's just some "glue code" that we need for outputs to play nicely.

On GFX10/NGG this limitation no longer exists, because NGG can export directly to the PC.

##### Notes about merged shaders

The merged stages on GFX9 (and GFX10/legacy) are: LSHS and ESGS. On GFX10/NGG the ESGS is merged with HW VS into NGG.

This might be confusing due to a mismatch between the number of invocations of these shaders.
For example, ES is per-vertex, but GS is per-primitive.
This is why merged shaders get an argument called `merged_wave_info` which tells how many invocations each part needs,
and there is some code at the beginning of each part to ensure the correct number of invocations by disabling some threads.
So, think about these as two independent shader programs slapped together.

### Which software stage runs on which hardware stage?

#### Graphics Pipeline

##### GFX6-8:

* Each SW stage has its own HW stage
* LS and HS share the same LDS space, so LS can store its output to LDS, where HS can read it
* HS, ES, GS outputs are stored in VRAM, next stage reads these from VRAM
* GS outputs got to VRAM, so they have to be copied by a GS copy shader running on the HW VS stage

| GFX6-8 HW stages:       | LS  | HS  | ES  | GS  | VS     | PS | ACO terminology |
| -----------------------:|:----|:----|:----|:----|:-------|:---|:----------------|
| SW stages: only VS+PS:  |     |     |     |     | VS     | FS | `vertex_vs`, `fragment_fs` |
|            with tess:   | VS  | TCS |     |     | TES    | FS | `vertex_ls`, `tess_control_hs`, `tess_eval_vs`, `fragment_fs` |
|            with GS:     |     |     | VS  | GS  | GS copy| FS | `vertex_es`, `geometry_gs`, `gs_copy_vs`, `fragment_fs` |
|            with both:   | VS  | TCS | TES | GS  | GS copy| FS | `vertex_ls`, `tess_control_hs`, `tess_eval_es`, `geometry_gs`, `gs_copy_vs`, `fragment_fs` |

##### GFX9+ (including GFX10/legacy):

* HW LS and HS stages are merged, and the merged shader still uses LDS in the same way as before
* HW ES and GS stages are merged, so ES outputs can go to LDS instead of VRAM
* LSHS outputs and ESGS outputs are still stored in VRAM, so a GS copy shader is still necessary

| GFX9+ HW stages:        | LSHS      | ESGS      | VS     | PS | ACO terminology |
| -----------------------:|:----------|:----------|:-------|:---|:----------------|
| SW stages: only VS+PS:  |           |           | VS     | FS | `vertex_vs`, `fragment_fs` |
|            with tess:   | VS + TCS  |           | TES    | FS | `vertex_tess_control_hs`, `tess_eval_vs`, `fragment_fs` |
|            with GS:     |           | VS + GS   | GS copy| FS | `vertex_geometry_gs`, `gs_copy_vs`, `fragment_fs` |
|            with both:   | VS + TCS  | TES + GS  | GS copy| FS | `vertex_tess_control_hs`, `tess_eval_geometry_gs`, `gs_copy_vs`, `fragment_fs` |

##### NGG (GFX10+ only):

 * HW GS and VS stages are now merged, and NGG can export directly to PC
 * GS copy shaders are no longer needed

| GFX10/NGG HW stages:    | LSHS      | NGG                | PS | ACO terminology |
| -----------------------:|:----------|:-------------------|:---|:----------------|
| SW stages: only VS+PS:  |           | VS                 | FS | `vertex_ngg`, `fragment_fs` |
|            with tess:   | VS + TCS  | TES                | FS | `vertex_tess_control_hs`, `tess_eval_ngg`, `fragment_fs` |
|            with GS:     |           | VS + GS            | FS | `vertex_geometry_ngg`, `fragment_fs` |
|            with both:   | VS + TCS  | TES + GS           | FS | `vertex_tess_control_hs`, `tess_eval_geometry_ngg`, `fragment_fs` |

#### Mesh Shading Graphics Pipeline

GFX10.3+:

* TS will run as a CS and stores its output payload to VRAM
* MS runs on NGG, loads its inputs from VRAM and stores outputs to LDS, then PC
* Pixel Shaders work the same way as before

| GFX10.3+ HW stages      | CS    | NGG   | PS | ACO terminology |
| -----------------------:|:------|:------|:---|:----------------|
| SW stages: only MS+PS:  |       | MS    | FS | `mesh_ngg`, `fragment_fs` |
|            with task:   | TS    | MS    | FS | `task_cs`, `mesh_ngg`, `fragment_fs` |

#### Compute pipeline

GFX6-10:

* Note that the SW CS always runs on the HW CS stage on all HW generations.

| GFX6-10 HW stage        | CS   | ACO terminology |
| -----------------------:|:-----|:----------------|
| SW stage                | CS   | `compute_cs`    |


## How to debug

Handy `RADV_DEBUG` options that help with ACO debugging:

* `nocache` - you always want to use this when debugging, otherwise you risk using a broken shader from the cache.
* `shaders` - makes ACO print the IR after register allocation, as well as the disassembled shader binary.
* `metashaders` - does the same thing as `shaders` but for built-in RADV shaders.
* `preoptir` - makes ACO print the final NIR shader before instruction selection, as well as the ACO IR after instruction selection.
* `nongg` - disables NGG support

We also have `ACO_DEBUG` options:

* `validateir` - Validate the ACO IR between compilation stages. By default, enabled in debug builds and disabled in release builds.
* `validatera` - Perform a RA (register allocation) validation.
* `perfwarn` - Warn when sub-optimal instructions are found.
* `force-waitcnt` - Forces ACO to emit a wait state after each instruction when there is something to wait for. Harms performance.
* `novn` - Disables the ACO value numbering stage.
* `noopt` - Disables the ACO optimizer.
* `nosched` - Disables the ACO pre-RA and post-RA scheduler.
* `nosched-ilp` - Disables the ACO post-RA ILP scheduler.

Note that you need to **combine these options into a comma-separated list**, for example: `RADV_DEBUG=nocache,shaders` otherwise only the last one will take effect. (This is how all environment variables work, yet this is an often made mistake.) Example:

```
RADV_DEBUG=nocache,shaders ACO_DEBUG=validateir,validatera vkcube
```

### Using GCC sanitizers

GCC has several sanitizers which can help figure out hard to diagnose issues. To use these, you need to pass
the `-Dbsanitize` flag to `meson` when building mesa. For example `-Dbsanitize=undefined` will add support for
the undefined behavior sanitizer.

### Hardened builds and glibc++ assertions

Several Linux distributions use "hardened" builds meaning several special compiler flags are added by
downstream packaging which are not used in mesa builds by default. These may be responsible for
some bug reports of inexplicable crashes with assertion failures you can't reproduce.

Most notable are the glibc++ debug flags, which you can use by adding the `-D_GLIBCXX_ASSERTIONS=1` and
`-D_GLIBCXX_DEBUG=1` flags.

To see the full list of downstream compiler flags, you can use eg. `rpm --eval "%optflags"`
on Red Hat based distros like Fedora.

### Good practices

Here are some good practices we learned while debugging visual corruption and hangs.

1. Bisecting shaders:
    * Use renderdoc when examining shaders. This is deterministic while real games often use multi-threading or change the order in which shaders get compiled.
    * Edit `radv_shader.c` or `radv_pipeline.c` to change if they are compiled with LLVM or ACO.
2. Things to check early:
    * Disable value_numbering, optimizer and/or scheduler.
      Note that if any of these change the output, it does not necessarily mean that the error is there, as register assignment does also change.
3. Finding the instruction causing a hang:
    * The ability to directly manipulate the binaries gives us an easy way to find the exact instruction which causes the hang.
      Use NULL exports (for FS and VS) and `s_endpgm` to end the shader early to find the problematic instruction.
4. Other faulty instructions:
    * Use print_asm and check for illegal instructions.
    * Compare to the ACO IR to see if the assembly matches what we want (this can take a while).
      Typical issues might be a wrong instruction format leading to a wrong opcode or an sgpr used for vgpr field.
5. Comparing to the LLVM backend:
   * If everything else didn't help, we probably just do something wrong. The LLVM backend is quite mature, so its output might help find differences, but this can be a long road.
