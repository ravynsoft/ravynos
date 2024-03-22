# Unofficial GCN/RDNA ISA reference errata

## `v_sad_u32`

The Vega ISA reference writes its behaviour as:

```
D.u = abs(S0.i - S1.i) + S2.u.
```

This is incorrect. The actual behaviour is what is written in the GCN3 reference
guide:

```
ABS_DIFF (A,B) = (A>B) ? (A-B) : (B-A)
D.u = ABS_DIFF (S0.u,S1.u) + S2.u
```

The instruction doesn't subtract the S0 and S1 and use the absolute value (the
_signed_ distance), it uses the _unsigned_ distance between the operands. So
`v_sad_u32(-5, 0, 0)` would return `4294967291` (`-5` interpreted as unsigned),
not `5`.

## `s_bfe_*`

Both the RDNA, Vega and GCN3 ISA references write that these instructions don't write
SCC. They do.

## `v_bcnt_u32_b32`

The Vega ISA reference writes its behaviour as:

```
D.u = 0;
for i in 0 ... 31 do
D.u += (S0.u[i] == 1 ? 1 : 0);
endfor.
```

This is incorrect. The actual behaviour (and number of operands) is what
is written in the GCN3 reference guide:

```
D.u = CountOneBits(S0.u) + S1.u.
```

## `v_alignbyte_b32`

All versions of the ISA document are vague about it, but after some trial and
error we discovered that only 2 bits of the 3rd operand are used.
Therefore, this instruction can't shift more than 24 bits.

The correct description of `v_alignbyte_b32` is probably the following:

```
D.u = ({S0, S1} >> (8 * S2.u[1:0])) & 0xffffffff
```

## SMEM stores

The Vega ISA references doesn't say this (or doesn't make it clear), but
the offset for SMEM stores must be in m0 if IMM == 0.

The RDNA ISA doesn't mention SMEM stores at all, but they seem to be supported
by the chip and are present in LLVM. AMD devs however highly recommend avoiding
these instructions.

## SMEM atomics

RDNA ISA: same as the SMEM stores, the ISA pretends they don't exist, but they
are there in LLVM.

## VMEM stores

All reference guides say (under "Vector Memory Instruction Data Dependencies"):

> When a VM instruction is issued, the address is immediately read out of VGPRs
> and sent to the texture cache. Any texture or buffer resources and samplers
> are also sent immediately. However, write-data is not immediately sent to the
> texture cache.

Reading that, one might think that waitcnts need to be added when writing to
the registers used for a VMEM store's data. Experimentation has shown that this
does not seem to be the case on GFX8 and GFX9 (GFX6 and GFX7 are untested). It
also seems unlikely, since NOPs are apparently needed in a subset of these
situations.

## MIMG opcodes on GFX8/GCN3

The `image_atomic_{swap,cmpswap,add,sub}` opcodes in the GCN3 ISA reference
guide are incorrect. The Vega ISA reference guide has the correct ones.

## VINTRP encoding

VEGA ISA doc says the encoding should be `110010` but `110101` works.

## VOP1 instructions encoded as VOP3

RDNA ISA doc says that `0x140` should be added to the opcode, but that doesn't
work. What works is adding `0x180`, which LLVM also does.

## FLAT, Scratch, Global instructions

The NV bit was removed in RDNA, but some parts of the doc still mention it.

RDNA ISA doc 13.8.1 says that SADDR should be set to 0x7f when ADDR is used, but
9.3.1 says it should be set to NULL. We assume 9.3.1 is correct and set it to
SGPR_NULL.

## Legacy instructions

Some instructions have a `_LEGACY` variant which implements "DX9 rules", in which
the zero "wins" in multiplications, ie. `0.0*x` is always `0.0`. The VEGA ISA
mentions `V_MAC_LEGACY_F32` but this instruction is not really there on VEGA.

## LDS size and allocation granule

GFX7-8 ISA manuals are mistaken about the available LDS size.

* GFX7+ workgroups can use 64KB LDS.
  There is 64KB LDS per CU.
* GFX6 workgroups can use 32KB LDS.
  There is 64KB LDS per CU, but a single workgroup can only use half of it.

 Regarding the LDS allocation granule, Mesa has the correct details and
 the ISA manuals are mistaken.

## `m0` with LDS instructions on Vega and newer

The Vega ISA doc (both the old one and the "7nm" one) claims that LDS instructions
use the `m0` register for address clamping like older GPUs, but this is not the case.

In reality, only the `_addtid` variants of LDS instructions use `m0` on Vega and
newer GPUs, so the relevant section of the RDNA ISA doc seems to apply.
LLVM also doesn't emit any initialization of `m0` for LDS instructions, and this
was also confirmed by AMD devs.

## RDNA L0, L1 cache and DLC, GLC bits

The old L1 cache was renamed to L0, and a new L1 cache was added to RDNA. The
L1 cache is 1 cache per shader array. Some instruction encodings have DLC and
GLC bits that interact with the cache.

* DLC ("device level coherent") bit: controls the L1 cache
* GLC ("globally coherent") bit: controls the L0 cache

The recommendation from AMD devs is to always set these two bits at the same time,
as it doesn't make too much sense to set them independently, aside from some
circumstances (eg. we needn't set DLC when only one shader array is used).

Stores and atomics always bypass the L1 cache, so they don't support the DLC bit,
and it shouldn't be set in these cases. Setting the DLC for these cases can result
in graphical glitches or hangs.

## RDNA `s_dcache_wb`

The `s_dcache_wb` is not mentioned in the RDNA ISA doc, but it is needed in order
to achieve correct behavior in some SSBO CTS tests.

## RDNA subvector mode

The documentation of `s_subvector_loop_begin` and `s_subvector_mode_end` is not clear
on what sort of addressing should be used, but it says that it
"is equivalent to an `S_CBRANCH` with extra math", so the subvector loop handling
in ACO is done according to the `s_cbranch` doc.

## RDNA early rasterization

The ISA documentation says about `s_endpgm`:

> The hardware implicitly executes S_WAITCNT 0 and S_WAITCNT_VSCNT 0
> before executing this instruction.

What the doc doesn't say is that in case of NGG (and legacy VS) when there
are no param exports, the driver sets `NO_PC_EXPORT=1` for optimal performance,
and when this is set, the hardware will start clipping and rasterization
as soon as it encounters a position export with `DONE=1`, without waiting
for the NGG (or VS) to finish.

It can even launch PS waves before NGG (or VS) ends.

When this happens, any store performed by a VS is not guaranteed
to be complete when PS tries to load it, so we need to manually
make sure to insert wait instructions before the position exports.

## A16 and G16

On GFX9, the A16 field enables both 16 bit addresses and derivatives.
Since GFX10+ these are fully independent of each other, A16 controls 16 bit addresses
and G16 opcodes 16 bit derivatives. A16 without G16 uses 32 bit derivatives.

## POPS collision wave ID argument (GFX9-10.3)

The 2020 RDNA and RDNA 2 ISA references contain incorrect offsets and widths of
the fields of the "POPS collision wave ID" SGPR argument.

According to the code generated for Rasterizer Ordered View usage in Direct3D,
the correct layout is:

* [31]: Whether overlap has occurred.
* [29:28] (GFX10+) / [28] (GFX9): ID of the packer the wave should be associated
  with.
* [25:16]: Newest overlapped wave ID.
* [9:0]: Current wave ID.

# Hardware Bugs

## SMEM corrupts VCCZ on SI/CI

[See this LLVM source.](https://github.com/llvm/llvm-project/blob/acb089e12ae48b82c0b05c42326196a030df9b82/llvm/lib/Target/AMDGPU/SIInsertWaits.cpp#L580-L616)

After issuing a SMEM instructions, we need to wait for the SMEM instructions to
finish and then write to vcc (for example, `s_mov_b64 vcc, vcc`) to correct vccz

Currently, we don't do this.

## SGPR offset on MUBUF prevents addr clamping on SI/CI

[See this LLVM source.](https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/AMDGPU/Utils/AMDGPUBaseInfo.cpp#L1917-L1922)

This leads to wrong bounds checking, using a VGPR offset fixes it.

## unused VMEM/DS destination lanes can't be used without waiting

On GFX11, we can't safely read/write unused lanes of VMEM/DS destination
VGPRs without waiting for the load to finish.

## GCN / GFX6 hazards

### VINTRP followed by a read with `v_readfirstlane` or `v_readlane`

It's required to insert 1 wait state if the dst VGPR of any  `v_interp_*` is
followed by a read with `v_readfirstlane` or `v_readlane` to fix GPU hangs on GFX6.
Note that `v_writelane_*` is apparently not affected. This hazard isn't
documented anywhere but AMD confirmed it.

## RDNA / GFX10 hazards

### SMEM store followed by a load with the same address

We found that an `s_buffer_load` will produce incorrect results if it is preceded
by an `s_buffer_store` with the same address. Inserting an `s_nop` between them
does not mitigate the issue, so an `s_waitcnt lgkmcnt(0)` must be inserted.
This is not mentioned by LLVM among the other GFX10 bugs, but LLVM doesn't use
SMEM stores, so it's not surprising that they didn't notice it.

### VMEMtoScalarWriteHazard

Triggered by:
VMEM/FLAT/GLOBAL/SCRATCH/DS instruction reads an SGPR (or EXEC, or M0).
Then, a SALU/SMEM instruction writes the same SGPR.

Mitigated by:
A VALU instruction or an `s_waitcnt` between the two instructions.

### SMEMtoVectorWriteHazard

Triggered by:
An SMEM instruction reads an SGPR. Then, a VALU instruction writes that same SGPR.

Mitigated by:
Any non-SOPP SALU instruction (except `s_setvskip`, `s_version`, and any non-lgkmcnt `s_waitcnt`).

### Offset3fBug

Any branch that is located at offset 0x3f will be buggy. Just insert some NOPs to make sure no branch
is located at this offset.

### InstFwdPrefetchBug

According to LLVM, the `s_inst_prefetch` instruction can cause a hang on GFX10.
Seems to be resolved on GFX10.3+. There are no further details.

### LdsMisalignedBug

When there is a misaligned multi-dword FLAT load/store instruction in WGP mode,
it needs to be split into multiple single-dword FLAT instructions.

ACO doesn't use FLAT load/store on GFX10, so is unaffected.

### FlatSegmentOffsetBug

The 12-bit immediate OFFSET field of FLAT instructions must always be 0.
GLOBAL and SCRATCH are unaffected.

ACO doesn't use FLAT load/store on GFX10, so is unaffected.

### VcmpxPermlaneHazard

Triggered by:
Any permlane instruction that follows any VOPC instruction which writes exec.

Mitigated by: any VALU instruction except `v_nop`.

### VcmpxExecWARHazard

Triggered by:
Any non-VALU instruction reads the EXEC mask. Then, any VALU instruction writes the EXEC mask.

Mitigated by:
A VALU instruction that writes an SGPR (or has a valid SDST operand), or `s_waitcnt_depctr 0xfffe`.
Note: `s_waitcnt_depctr` is an internal instruction, so there is no further information
about what it does or what its operand means.

### LdsBranchVmemWARHazard

Triggered by:
VMEM/GLOBAL/SCRATCH instruction, then a branch, then a DS instruction,
or vice versa: DS instruction, then a branch, then a VMEM/GLOBAL/SCRATCH instruction.

Mitigated by:
Only `s_waitcnt_vscnt null, 0`. Needed even if the first instruction is a load.

### NSAClauseBug

"MIMG-NSA in a hard clause has unpredictable results on GFX10.1"

### NSAMaxSize5

NSA MIMG instructions should be limited to 3 dwords before GFX10.3 to avoid
stability issues: https://reviews.llvm.org/D103348

## RDNA3 / GFX11 hazards

### VcmpxPermlaneHazard

Same as GFX10.

### LdsDirectVALUHazard

Triggered by:
LDSDIR instruction writing a VGPR soon after it's used by a VALU instruction.

Mitigated by:
A vdst wait, preferably using the LDSDIR's field.

### LdsDirectVMEMHazard

Triggered by:
LDSDIR instruction writing a VGPR after it's used by a VMEM/DS instruction.

Mitigated by:
Waiting for the VMEM/DS instruction to finish, a VALU or export instruction, or
`s_waitcnt_depctr 0xffe3`.

### VALUTransUseHazard

Triggered by:
A VALU instruction reading a VGPR written by a transcendental VALU instruction without 6+ VALU or 2+
transcendental instructions in-between.

Mitigated by:
A va_vdst=0 wait: `s_waitcnt_deptr 0x0fff`

### VALUPartialForwardingHazard

Triggered by:
A VALU instruction reading two VGPRs: one written before an exec write by SALU and one after. To
trigger, there must be less than 3 VALU between the first and second VGPR writes and less than 5
VALU between the second VGPR write and the current instruction.

Mitigated by:
A va_vdst=0 wait: `s_waitcnt_deptr 0x0fff`

### VALUMaskWriteHazard

Triggered by:
SALU writing then reading a SGPR that was previously used as a lane mask for a VALU.

Mitigated by:
A VALU instruction reading a SGPR or with literal, or a sa_sdst=0 wait: `s_waitcnt_depctr 0xfffe`
