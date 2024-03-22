-- Parse logs from test-quad-textured-3d.c to exctract layer/level
-- offsets
--
-- We figure out the offsets from blits, but there may be some
-- unrelated blits.  So just save all of them until we find the
-- texture state for the 3d texture.  This gives us the base
-- address, and the miplevel #0 width/height/depth.  Then work
-- backwards from there finding the blits to the same dst buffer
-- and deducing the miplevel from the minified dimensions

local posix = require "posix"

io.write("Analyzing Data...\n")

local allblits = {}
local nallblits = 0
local r = rnn.init("a630")

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

function draw(primtype, nindx)
  if primtype ~= "BLIT_OP_SCALE" then
    return
  end

  -- Just in case, filter out anything that isn't starting
  -- at 0,0
  if r.GRAS_2D_DST_TL.X ~= 0 or r.GRAS_2D_DST_TL.Y ~= 0 then
    return
  end

  local blit = {}
  
  blit.width   = r.GRAS_2D_DST_BR.X + 1
  blit.height  = r.GRAS_2D_DST_BR.Y + 1
  blit.pitch   = r.RB_2D_DST_SIZE.PITCH
  blit.addr    = r.RB_2D_DST_LO | (r.RB_2D_DST_HI << 32)
  blit.base    = bos.base(blit.addr)
  blit.endaddr = 0  -- filled in later
  --printf("Found blit: 0x%x (0x%x)\n", blit.addr, blit.base)

  allblits[nallblits] = blit
  nallblits = nallblits + 1
end

function A6XX_TEX_CONST(pkt, size)
  -- ignore any texture state w/ DEPTH=1, these aren't the 3d tex state we
  -- are looking for
  if pkt[5].DEPTH <= 1 then
    return
  end

  local base = pkt[4].BASE_LO | (pkt[5].BASE_HI << 32)
  local width0  = pkt[1].WIDTH
  local height0 = pkt[1].HEIGHT
  local depth0  = pkt[5].DEPTH

  printf("Found texture state: %ux%ux%u (MIN_LAYERSZ=0x%x)\n",
         width0, height0, depth0, pkt[3].MIN_LAYERSZ)

  -- Note that in some case the texture has some extra page or so
  -- at the beginning:
  local basebase = bos.base(base)
  printf("base: 0x%x (0x%x)\n", base, basebase)

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

  -- now go thru the relevant blits and print out interesting details
  local level = 0
  local layer = 0
  local w = width0   -- track current width/height to detect changing
  local h = height0  -- mipmap level
  for n = 0,nblits-1 do
    local blit = blits[n]
    --printf("%u: %ux%u, addr=%x\n", n, blit.width, blit.height, blit.addr)
    if w ~= blit.width or h ~= blit.height then
      level = level + 1
      layer = 0

      if blit.width ~= minify(w, 1) or blit.height ~= minify(h, 1) then
        printf("I am confused! %ux%u vs %ux%u\n", blit.width, blit.height, minify(w, 1), minify(h, 1))
	printf("addr=%x\n", blit.addr)
        --return
      end

      w = blit.width
      h = blit.height
    end

    printf("level=%u, layer=%u, sz=%ux%u, pitch=%u, offset=0x%x, addr=%x",
           level, layer, w, h, blit.pitch, blit.addr - base, blit.addr)
    if blit.endaddr ~= 0 then
      local layersz = blit.endaddr - blit.addr
      local alignedheight = layersz / blit.pitch
      printf(", layersz=0x%x, alignedheight=%f", layersz, alignedheight)
    end
    printf("\n")

    layer = layer + 1
  end
  printf("\n\n")
end

