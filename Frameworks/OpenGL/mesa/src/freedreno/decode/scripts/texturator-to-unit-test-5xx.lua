-- Parse logs from https://github.com/freedreno/freedreno/
-- test-texturator.c to generate a src/freedreno/fdl/fd5_layout_test.c
-- block.  We figure out the offsets from blits, but there may be some
-- unrelated blits.  So just save all of them until we find the
-- texture state.  This gives us the base address, and the miplevel #0
-- width/height/depth.  Then work backwards from there finding the
-- blits to the same dst buffer and deducing the miplevel from the
-- minified dimensions

local posix = require "posix"

io.write("Analyzing Data...\n")

local r = rnn.init("a530")
local found_tex = 0

local allblits = {}
local nallblits = 0

function get_first_blit(base, width, height)
  local first_blit = nil

  for n = 0,nallblits-1 do
    local blit = allblits[n]
    if blit.base == base and blit.width == width and blit.height == height then
      if not first_blit or blit.addr < first_blit.addr then
        first_blit = blit
      end
    end
  end

  return first_blit
end

function minify(val, lvls)
  val = val >> lvls
  if val < 1 then
    return 1
  end
  return val
end

function printf(fmt, ...)
  return io.write(string.format(fmt, ...))
end

function start_cmdstream(name)
  io.write("Parsing " .. name .. "\n")
  allblits = {}
  nallblits = 0
end

-- Record texture upload blits done through CP_EVENT_WRITE
function CP_EVENT_WRITE(pkt, size)
  if tostring(pkt[0].EVENT) ~= "BLIT" then
    return
  end

  local blit = {}

  blit.width   = r.RB_RESOLVE_CNTL_2.X + 1
  blit.height  = r.RB_RESOLVE_CNTL_2.Y + 1
  blit.pitch   = r.RB_BLIT_DST_PITCH
  blit.addr    = r.RB_BLIT_DST_LO | (r.RB_BLIT_DST_HI << 32)
  blit.base    = bos.base(blit.addr)
  blit.ubwc_addr = r.RB_BLIT_FLAG_DST_LO | (r.RB_BLIT_FLAG_DST_HI << 32)
  blit.ubwc_base = bos.base(blit.ubwc_addr)
  blit.ubwc_pitch = r.RB_BLIT_FLAG_DST_PITCH
  blit.endaddr = 0  -- filled in later
  printf("Found event blit: 0x%x (0x%x) %dx%d UBWC 0x%x (0x%x) tiled %s\n", blit.addr, blit.base, blit.width, blit.height, blit.ubwc_addr, blit.ubwc_base, r.RB_RESOLVE_CNTL_3.TILED)

  allblits[nallblits] = blit
  nallblits = nallblits + 1
end

function CP_BLIT(pkt, size)
  -- Just in case, filter out anything that isn't starting
  -- at 0,0
  if pkt[1].SRC_X1 ~= 0 or pkt[1].SRC_Y1 ~= 0 then
    return
  end

  local blit = {}

  blit.width   = pkt[2].SRC_X2 + 1
  blit.height  = pkt[2].SRC_Y2 + 1
  blit.pitch   = r.RB_2D_DST_SIZE.PITCH
  blit.addr    = r.RB_2D_DST_LO | (r.RB_2D_DST_HI << 32)
  blit.base    = bos.base(blit.addr)
  blit.ubwc_addr = r.RB_2D_DST_FLAGS_LO | (r.RB_2D_DST_FLAGS_HI << 32)
  blit.ubwc_base = bos.base(blit.ubwc_addr)
  blit.ubwc_pitch = r.RB_2D_DST_FLAGS_PITCH
  blit.endaddr = 0  -- filled in later
  printf("Found cp blit: 0x%x (0x%x) %dx%d UBWC 0x%x (0x%x) %s\n", blit.addr, blit.base, blit.width, blit.height, blit.ubwc_addr, blit.ubwc_base, r.RB_2D_DST_INFO.TILE_MODE)

  allblits[nallblits] = blit
  nallblits = nallblits + 1
end

function A5XX_TEX_CONST(pkt, size)
  -- ignore any texture state w/ DEPTH=1, these aren't the 3d tex state we
  -- are looking for

  local base = pkt[4].BASE_LO | (pkt[5].BASE_HI << 32)
  -- UBWC base on a5xx seems to be at the start of each miplevel, followed by pixels
  -- somewhere past that.
  local ubwc_base = base
  local width0  = pkt[1].WIDTH
  local height0 = pkt[1].HEIGHT
  local depth0  = pkt[5].DEPTH

  if (found_tex ~= 0) then
    return
  end
  found_tex = 1

  printf("Found texture state:\n  %ux%ux%u (%s, %s, UBWC=%s)\n",
         width0, height0, depth0, pkt[0].FMT, pkt[0].TILE_MODE, tostring(pkt[3].FLAG))

  -- Note that in some case the texture has some extra page or so
  -- at the beginning:
  local basebase = bos.base(base)
  printf("base: 0x%x (0x%x)\n", base, basebase)
  printf("ubwcbase: 0x%x (0x%x)\n", ubwc_base, bos.base(ubwc_base))

  -- see if we can find the associated blits..  The blob always seems to
  -- start from the lower (larger) mipmap levels and layers, so we don't
  -- need to sort by dst address.  Also, while we are at it, fill in the
  -- end-addr (at least for everything but the last blit)
  local blits = {}
  local nblits = 0
  local lastblit = nil
  for n = 0,nallblits-1 do
    local blit = allblits[n]
    --printf("blit addr: 0x%x (0x%x)\n", blit.addr, blit.base)
    if blit.base == basebase and blit.addr >= base then
      blits[nblits] = blit
      nblits = nblits + 1
      if lastblit then
        lastblit.endaddr = blit.addr
      end
      lastblit = blit
    end
  end

  printf("	{\n")
  printf("		.format = %s,\n", pkt[0].FMT)
  if (tostring(pkt[2].TYPE) == "A5XX_TEX_3D") then
    printf("		.is_3d = true,\n")
  end

  printf("		.layout = {\n")
  printf("			.tile_mode = %s,\n", pkt[0].TILE_MODE)
  printf("			.ubwc = %s,\n", tostring(pkt[3].FLAG))

  if (tostring(pkt[2].TYPE) == "A5XX_TEX_3D") then
    printf("			.width0 = %d, .height0 = %d, .depth0 = %d,\n", width0, height0, depth0)
  else
    printf("			.width0 = %d, .height0 = %d,\n", width0, height0)
  end

  printf("			.slices = {\n")
  local w = 0
  local h = 0
  local level = 0
  repeat
    local w = minify(width0, level)
    local h = minify(height0, level)
    local blit = get_first_blit(basebase, w, h)
    if blit then
      printf("				{ .offset = %d, .pitch = %u },\n",
          blit.addr - base,
          blit.pitch);
    end
    level = level + 1
  until w == 1 and h == 1
  printf("			},\n")

  if pkt[3].FLAG then
    printf("			.ubwc_slices = {\n")
    level = 0
    repeat
      local w = minify(width0, level)
      local h = minify(height0, level)
      local blit = get_first_blit(basebase, w, h)
      if blit then
        printf("				{ .offset = %d, .pitch = %u },\n",
            blit.ubwc_addr - ubwc_base,
            blit.ubwc_pitch);
      end
      level = level + 1
    until w == 1 and h == 1
    printf("			},\n")
  end

  printf("		},\n")
  printf("	},\n")
  printf("\n\n")
end

