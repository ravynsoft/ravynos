-- A script that compares a set of equivalent cmdstream captures from
-- various generations, looking for equivalencies between registers.
--
-- This would be run across a group of similar tests for various
-- generations, for example:
--
--   cffdump --script scripts/analyze.lua a320/quad-flat-*.rd a420/quad-flat-*.rd
--
-- This is done by comparing unique register values.  Ie. for each
-- generation, find the set of registers that have different values
-- between equivalent draw calls.

local posix = require "posix"

io.write("Analyzing Data...\n")

-- results - table structure:
-- * [gpuname] - gpu
--   * tests
--     * [testname] - current test
--       * draws
--         * [1..n] - the draws
--           * primtype - the primitive type
--           * regs - table of values for draw
--             * [regbase] - regval
--   * regvals - table of unique values across all draws
--     * [regbase]
--       * [regval] - list of test names
--         * [1..n] - testname "." didx
local results = {}

local test = nil
local gpuname = nil
local testname = nil


-- srsly, no sparse table size() op?
function tblsz(tbl)
  local n = 0;
  for k,v in pairs(tbl) do
    n = n + 1
  end
  return n
end


function start_cmdstream(name)
  testname = posix.basename(name)
  gpuname = posix.basename(posix.dirname(name))
  --io.write("START: gpuname=" .. gpuname .. ", testname=" .. testname .. "\n");
  local gpu = results[gpuname]
  if gpu == nil then
    gpu = {["tests"] = {}, ["regvals"] = {}}
    results[gpuname] = gpu
  end
  test = {["draws"] = {}}
  gpu["tests"][testname] = test
end

function draw(primtype, nindx)
  -- RECTLIST is only used internally.. we want to ignore it for
  -- now, although it could potentially be interesting to track
  -- these separately (separating clear/restore/resolve) just to
  -- figure out which registers are used for which..
  if primtype == "DI_PT_RECTLIST" then
    return
  end
  local regtbl = {}
  local draw = {["primtype"] = primtype, ["regs"] = regtbl}
  local didx = tblsz(test["draws"])

  test["draws"][didx] = draw

  -- populate current regs.  For now just consider ones that have
  -- been written.. maybe we need to make that configurable in
  -- case it filters out too many registers.
  for regbase=0,0xffff do
    if regs.written(regbase) ~= 0 then
      local regval = regs.val(regbase)

      -- track reg vals per draw:
      regtbl[regbase] = regval

      -- also track which reg vals appear in which tests:
      local uniq_regvals = results[gpuname]["regvals"][regbase]
      if uniq_regvals == nil then
        uniq_regvals = {}
        results[gpuname]["regvals"][regbase] = uniq_regvals;
      end
      local drawlist = uniq_regvals[regval]
      if drawlist == nil then
        drawlist = {}
        uniq_regvals[regval] = drawlist
      end
      table.insert(drawlist, testname .. "." .. didx)
    end
  end

  -- TODO maybe we want to whitelist a few well known regs, for the
  -- convenience of the code that runs at the end to analyze the data?
  -- TODO also would be useful to somehow capture CP_SET_BIN..

end

function end_cmdstream()
  test = nil
  gpuname = nil
  testname = nil
end

function print_draws(gpuname, gpu)
  io.write("  " .. gpuname .. "\n")
  for testname,test in pairs(gpu["tests"]) do
    io.write("    " .. testname .. ", draws=" .. #test["draws"] .. "\n")
    for didx,draw in pairs(test["draws"]) do
      io.write("      " .. didx .. ": " .. draw["primtype"] .. "\n")
    end
  end
end

-- sort and concat a list of draw names to form a key which can be
-- compared to other drawlists to check for equality
-- TODO maybe we instead want a scheme that allows for some fuzzyness
-- in the matching??
function drawlistname(drawlist)
  local name = nil
  for idx,draw in pairs(drawlist) do
    if name == nil then
      name = draw
    else
      name = name .. ":" .. draw
    end
  end
  return name
end

local rnntbl = {}

function dumpmatches(name)
  for gpuname,gpu in pairs(results) do
    local r = rnntbl[gpuname]
    if r == nil then
      io.write("loading rnn database: \n" .. gpuname)
      r = rnn.init(gpuname)
      rnntbl[gpuname] = r
    end
    for regbase,regvals in pairs(gpu["regvals"]) do
      for regval,drawlist in pairs(regvals) do
        local name2 = drawlistname(drawlist)
        if name == name2 then
          io.write(string.format("  %s:%s:\t%08x  %s\n",
                                 gpuname, rnn.regname(r, regbase),
                                 regval, rnn.regval(r, regbase, regval)))
        end
      end
    end
  end
end

function finish()
  -- drawlistnames that we've already dumped:
  local dumped = {}

  for gpuname,gpu in pairs(results) do
    -- print_draws(gpuname, gpu)
    for regbase,regvals in pairs(gpu["regvals"]) do
      for regval,drawlist in pairs(regvals) do
        local name = drawlistname(drawlist)
        if dumped[name] == nil then
          io.write("\n" .. name .. ":\n")
          dumpmatches(name)
          dumped[name] = 1
        end
      end
    end
  end
end

