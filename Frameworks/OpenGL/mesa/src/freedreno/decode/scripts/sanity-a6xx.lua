-- Parse cmdstream dump and check for common errors
--  1) Check for overflowing HLSQ_xS_CNTL.CONSTLEN
--  2) Check for constant uploades that overwrite each other.  The
--     range checking is reset on  each draw, since it is a valid
--     use-case to do partial constant upload.  But if we see two
--     CP_LOAD_STATE* that overwrite the same range of constants
--     within the same draw, that is almost certainly unintentional.
--
-- TODO add more checks
-- TODO maybe some parts could be shared across
--      different generations

--local posix = require "posix"

function printf(fmt, ...)
	return io.write(string.format(fmt, ...))
end

function dbg(fmt, ...)
	--printf(fmt, ...)
end

stages = {
	"SB6_VS_SHADER",
	"SB6_HS_SHADER",
	"SB6_DS_SHADER",
	"SB6_GS_SHADER",
	"SB6_FS_SHADER",
	"SB6_CS_SHADER",
}

-- maps shader stage to HLSQ_xS_CNTL register name:
cntl_regs = {
	["SB6_VS_SHADER"] = "HLSQ_VS_CNTL",
	["SB6_HS_SHADER"] = "HLSQ_HS_CNTL",
	["SB6_DS_SHADER"] = "HLSQ_DS_CNTL",
	["SB6_GS_SHADER"] = "HLSQ_GS_CNTL",
	["SB6_FS_SHADER"] = "HLSQ_FS_CNTL",
	["SB6_CS_SHADER"] = "HLSQ_CS_CNTL",
}

-- initialize constant updated ranges:
--   constranges[stagename] -> table of offsets that have been uploaded
constranges = {}
function reset_constranges()
	for i,stage in ipairs(stages) do
		constranges[stage] = {}
	end
end

reset_constranges()

printf("Checking cmdstream...\n")

local r = rnn.init("a630")

function draw(primtype, nindx)
	printf("draw!\n")
	-- reset ranges of uploaded consts on each draw:
	reset_constranges()
end

function CP_LOAD_STATE6(pkt, size)
	if tostring(pkt[0].STATE_TYPE) ~= "ST6_CONSTANTS" then
		return
	end
	dbg("got CP_LOAD_STATE6\n")
	stage = tostring(pkt[0].STATE_BLOCK)
	max = pkt[0].DST_OFF + pkt[0].NUM_UNIT
	cntl_reg = cntl_regs[stage]
	dbg("looking for %s.. max=%d vs %d\n", cntl_reg, max, r[cntl_reg].CONSTLEN)
	if max > r[cntl_reg].CONSTLEN then
		printf("ERROR: invalid max constant offset for stage %s: %d vs %d\n", stage, max, r[cntl_reg].CONSTLEN)
	end

end
