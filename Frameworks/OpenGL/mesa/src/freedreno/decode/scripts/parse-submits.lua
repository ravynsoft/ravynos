-- Parse cmdstream dump and analyse blits and batches

--local posix = require "posix"

function printf(fmt, ...)
	return io.write(string.format(fmt, ...))
end

function dbg(fmt, ...)
	--printf(fmt, ...)
end

printf("Analyzing Data...\n")

local r = rnn.init("a630")

-- Each submit, all draws will target the same N MRTs:
local mrts = {}
local allmrts = {}  -- includes historical render targets
function push_mrt(fmt, w, h, samples, base, flag, gmem)
	dbg("MRT: %s %ux%u 0x%x\n", fmt, w, h, base)

	local mrt = {}
	mrt.format = fmt
	mrt.w = w
	mrt.h = h
	mrt.samples = samples
	mrt.base = base
	mrt.flag = flag
	mrt.gmem = gmem

	mrts[base] = mrt
	allmrts[base] = mrt
end

-- And each each draw will read from M sources/textures:
local sources = {}
function push_source(fmt, w, h, samples, base, flag)
	dbg("SRC: %s %ux%u 0x%x\n", fmt, w, h, base)

	local source = {}
	source.format = fmt
	source.w = w
	source.h = h
	source.samples = samples
	source.base = base
	source.flag = flag

	sources[base] = source
end

local binw
local binh
local nbins
local blits = 0
local draws = 0
local drawmode
local cleared
local restored
local resolved
local nullbatch
local depthtest
local depthwrite
local stenciltest
local stencilwrite

function reset()
	dbg("reset\n")
	mrts = {}
	sources = {}
	draws = 0
	blits = 0
	cleared = {}
	restored = {}
	resolved = {}
	depthtest = false
	depthwrite = false
	stenciltest = false
	stencilwrite = false
	drawmode = Nil
end

function start_submit()
	dbg("start_submit\n")
	reset()
	nullbatch = true
end

function finish()
	dbg("finish\n")

	printf("\n")

	-- TODO we get false-positives for 'NULL BATCH!' because we don't have
	-- a really good way to differentiate between submits and cmds.  Ie.
	-- with growable cmdstream, and a large # of tiles, IB1 can get split
	-- across multiple buffers.  Since we ignore GMEM draws for window-
	-- offset != 0,0, the later cmds will appear as null batches
	if draws == 0 and blits == 0 then
		if nullbatch then
			printf("NULL BATCH!\n");
		end
		return
	end

	if draws > 0 then
		printf("Batch:\n")
		printf("-------\n")
		printf("  # of draws: %u\n", draws)
		printf("  mode: %s\n", drawmode)
		if drawmode == "RM6_GMEM" then
			printf("  bin size: %ux%u (%u bins)\n", binw, binh, nbins)
		end
		if depthtest or depthwrite then
			printf("  ")
			if depthtest then
				printf("DEPTHTEST ")
			end
			if depthwrite then
				printf("DEPTHWRITE")
			end
			printf("\n")
		end
		if stenciltest or stencilwrite then
			printf("  ")
			if stenciltest then
				printf("STENCILTEST ")
			end
			if stencilwrite then
				printf("STENCILWRITE")
			end
			printf("\n")
		end
	else
		printf("Blit:\n")
		printf("-----\n")
	end

	for base,mrt in pairs(mrts) do
		printf("  MRT[0x%x:0x%x]:\t%ux%u\t\t%s (%s)", base, mrt.flag, mrt.w, mrt.h, mrt.format, mrt.samples)
		if drawmode == "RM6_GMEM" then
			if cleared[mrt.gmem] then
				printf("\tCLEARED")
			end
			if restored[mrt.gmem] then
				printf("\tRESTORED")
			end
			if resolved[mrt.gmem] then
				printf("\tRESOLVED")
			end
		else
			if cleared[mrt.base] then
				printf("\tCLEARED")
			end
		end
		printf("\n")
	end

	function print_source(source)
		printf("  SRC[0x%x:0x%x]:\t%ux%u\t\t%s (%s)\n", source.base, source.flag, source.w, source.h, source.format, source.samples)
	end

	for base,source in pairs(sources) do
		-- only show sources that have been previously rendered to, other
		-- textures are less interesting.  Possibly this should be an
		-- option somehow
		if draws < 10 then
			print_source(source)
		elseif allmrts[base] or draws == 0 then
			print_source(source)
		elseif source.flag and allmrts[source.flag] then
			print_source(source)
		end
	end
	reset()
end

function end_submit()
	dbg("end_submit\n")
	finish()
end

-- Track the current mode:
local mode = ""
function CP_SET_MARKER(pkt, size)
	mode = pkt[0].MARKER
	dbg("mode: %s\n", mode)
end

function CP_EVENT_WRITE(pkt, size)
	if tostring(pkt[0].EVENT) ~= "BLIT" then
		return
	end
	nullbatch = false
	local m = tostring(mode)
	if m == "RM6_GMEM" then
		-- either clear or restore:
		if r.RB_BLIT_INFO.CLEAR_MASK == 0 then
			restored[r.RB_BLIT_BASE_GMEM] = 1
		else
			cleared[r.RB_BLIT_BASE_GMEM] = 1
		end
		-- push_mrt() because we could have GMEM
		-- passes with only a clear and no draws:
		local flag = 0
		local sysmem = 0;
		-- try to match up the GMEM addr with the MRT/DEPTH state,
		-- to avoid relying on RB_BLIT_DST also getting written:
		for n = 0,r.RB_FS_OUTPUT_CNTL1.MRT-1 do
			if r.RB_MRT[n].BASE_GMEM == r.RB_BLIT_BASE_GMEM then
				sysmem = r.RB_MRT[n].BASE
				flag = r.RB_MRT_FLAG_BUFFER[n].ADDR
				break
			end
		end
		if sysmem == 0 and r.RB_BLIT_BASE_GMEM == r.RB_DEPTH_BUFFER_BASE_GMEM then
			sysmem = r.RB_DEPTH_BUFFER_BASE
			flag = r.RB_DEPTH_FLAG_BUFFER_BASE

		end
		--NOTE this can get confused by previous blits:
		--if sysmem == 0 then
		--	-- fallback:
		--	sysmem = r.RB_BLIT_DST
		--	flag = r.RB_BLIT_FLAG_DST
		--end
		if not r.RB_BLIT_DST_INFO.FLAGS then
			flag = 0
		end
		-- TODO maybe just emit RB_BLIT_DST/HI for clears.. otherwise
		-- we get confused by stale values in registers.. not sure
		-- if this is a problem w/ blob
		push_mrt(r.RB_BLIT_DST_INFO.COLOR_FORMAT,
			r.RB_BLIT_SCISSOR_BR.X + 1,
			r.RB_BLIT_SCISSOR_BR.Y + 1,
			r.RB_BLIT_DST_INFO.SAMPLES,
			sysmem,
			flag,
			r.RB_BLIT_BASE_GMEM)
	elseif m == "RM6_RESOLVE" then
		resolved[r.RB_BLIT_BASE_GMEM] = 1
	else
		printf("I am confused!!!\n")
	end
end

function A6XX_TEX_CONST(pkt, size)
	push_source(pkt[0].FMT,
		pkt[1].WIDTH, pkt[1].HEIGHT,
		pkt[0].SAMPLES,
		pkt[4].BASE_LO | (pkt[5].BASE_HI << 32),
		pkt[7].FLAG_LO | (pkt[8].FLAG_HI << 32))
end

function handle_blit()
	-- blob sometimes uses CP_BLIT for resolves, so filter those out:
	-- TODO it would be nice to not hard-code GMEM addr:
	-- TODO I guess the src can be an offset from GMEM addr..
	if r.SP_PS_2D_SRC == 0x100000 and not r.RB_2D_BLIT_CNTL.SOLID_COLOR then
		resolved[0] = 1
		return
	end
	if draws > 0 then
		finish()
	end
	reset()
	drawmode = "BLIT"
	-- This kinda assumes that we are doing full img blits, which is maybe
	-- Not completely legit.  We could perhaps instead just track pitch and
	-- size/pitch??  Or maybe the size doesn't matter much
	push_mrt(r.RB_2D_DST_INFO.COLOR_FORMAT,
		r.GRAS_2D_DST_BR.X + 1,
		r.GRAS_2D_DST_BR.Y + 1,
		"MSAA_ONE",
		r.RB_2D_DST,
		r.RB_2D_DST_FLAGS,
		-1)
	if r.RB_2D_BLIT_CNTL.SOLID_COLOR then
		dbg("CLEAR=%x\n", r.RB_2D_DST)
		cleared[r.RB_2D_DST] = 1
	else
		push_source(r.SP_2D_SRC_FORMAT.COLOR_FORMAT,
			r.GRAS_2D_SRC_BR_X.X + 1,
			r.GRAS_2D_SRC_BR_Y.Y + 1,
			"MSAA_ONE",
			r.SP_PS_2D_SRC,
			r.SP_PS_2D_SRC_FLAGS)
	end
	blits = blits + 1
	finish()
end

function valid_transition(curmode, newmode)
	if curmode == "RM6_BINNING" and newmode == "RM6_GMEM" then
		return true
	end
	if curmode == "RM6_GMEM" and newmode == "RM6_RESOLVE" then
		return true
	end
	return false
end

function draw(primtype, nindx)
	dbg("draw: %s (%s)\n", primtype, mode)
	nullbatch = false
	if primtype == "BLIT_OP_SCALE" then
		handle_blit()
		return
	elseif primtype == "EVENT:BLIT" then
		return
	end

	local m = tostring(mode)

	-- detect changes in drawmode which indicate a different
	-- pass..  BINNING->GMEM means same pass, but other
	-- transitions mean different pass:
	if drawmode and m ~= drawmode then
		dbg("%s -> %s transition\n", drawmode, m)
		if not valid_transition(drawmode, m) then
			dbg("invalid transition, new render pass!\n")
			finish()
			reset()
		end
	end

	if m ~= "RM6_GMEM" and m ~= "RM6_BYPASS" then
		if m == "RM6_BINNING" then
			drawmode = m
			return
		end
		if m == "RM6_RESOLVE" and primtype == "EVENT:BLIT" then
			return
		end
		printf("unknown MODE %s for primtype %s\n", m, primtype)
		return
	end

	-- Only count the first tile for GMEM mode to avoid counting
	-- each draw for each tile
	if m == "RM6_GMEM" then
		if r.RB_WINDOW_OFFSET.X ~= 0 or r.RB_WINDOW_OFFSET.Y ~= 0 then
			return
		end
	end

	drawmode = m
	local render_components = {}
	render_components[0] = r.RB_RENDER_COMPONENTS.RT0;
	render_components[1] = r.RB_RENDER_COMPONENTS.RT1;
	render_components[2] = r.RB_RENDER_COMPONENTS.RT2;
	render_components[3] = r.RB_RENDER_COMPONENTS.RT3;
	render_components[4] = r.RB_RENDER_COMPONENTS.RT4;
	render_components[5] = r.RB_RENDER_COMPONENTS.RT5;
	render_components[6] = r.RB_RENDER_COMPONENTS.RT6;
	render_components[7] = r.RB_RENDER_COMPONENTS.RT7;
	for n = 0,r.RB_FS_OUTPUT_CNTL1.MRT-1 do
		if render_components[n] ~= 0 then
			push_mrt(r.RB_MRT[n].BUF_INFO.COLOR_FORMAT,
				r.GRAS_SC_SCREEN_SCISSOR[0].BR.X + 1,
				r.GRAS_SC_SCREEN_SCISSOR[0].BR.Y + 1,
				r.RB_BLIT_GMEM_MSAA_CNTL.SAMPLES,
				r.RB_MRT[n].BASE,
				r.RB_MRT_FLAG_BUFFER[n].ADDR,
				r.RB_MRT[n].BASE_GMEM)
		end
	end

	local depthbase = r.RB_DEPTH_BUFFER_BASE

	if depthbase ~= 0 then
		push_mrt(r.RB_DEPTH_BUFFER_INFO.DEPTH_FORMAT,
			r.GRAS_SC_SCREEN_SCISSOR[0].BR.X + 1,
			r.GRAS_SC_SCREEN_SCISSOR[0].BR.Y + 1,
			r.RB_BLIT_GMEM_MSAA_CNTL.SAMPLES,
			depthbase,
			r.RB_DEPTH_FLAG_BUFFER_BASE,
			r.RB_DEPTH_BUFFER_BASE_GMEM)
	end

	if r.RB_DEPTH_CNTL.Z_WRITE_ENABLE then
		depthwrite = true
	end

	if r.RB_DEPTH_CNTL.Z_TEST_ENABLE then
		depthtest = true
	end

	-- clearly 0 != false.. :-/
	if r.RB_STENCILWRMASK.WRMASK ~= 0 then
		stencilwrite = true
	end

	if r.RB_STENCIL_CONTROL.STENCIL_ENABLE then
		stenciltest = true
	end

	-- TODO should also check for stencil buffer for z32+s8 case

	if m == "RM6_GMEM" then
		binw = r.VSC_BIN_SIZE.WIDTH
		binh = r.VSC_BIN_SIZE.HEIGHT
		nbins = r.VSC_BIN_COUNT.NX * r.VSC_BIN_COUNT.NY
	end

	draws = draws + 1
end

