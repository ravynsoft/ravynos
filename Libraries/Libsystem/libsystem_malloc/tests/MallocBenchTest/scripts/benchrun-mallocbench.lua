local benchrun = require "benchrun"
local cjson = require "cjson"
local inspect = require "inspect"
local lfs = require "lfs"
local perfdata = require "perfdata"

local usage = [[benchrun-mallocbench [options] args

Runs run-malloc-benchmarks using recon-benchrun and writes out the results in
perfdataV2 format. All of the arguments to this script, other than those listed
under "Options" below, are passed to run-malloc-benchmarks.

Example usage:

	benchrun-mallocbench /BUILD/MallocBench SystemMalloc:/BUILD/MallocBench/libmbmalloc.dylib NanoMallocV1:/BUILD/libmbmalloc.dylib
	benchrun-mallocbench /BUILD/MallocBench bmalloc:/BUILD/bmalloc/libmbmalloc.dylib
	benchrun-mallocbench --benchmark churn SystemMalloc:/BUILD/MallocBench/libmbmalloc.dylib bmalloc:/BUILD/bmalloc/libmbmalloc.dylib

Options:

	-h, --help                Show this help message and exit.
]]
local fileSep = package.config:sub(1,1)
local run_benchmark = "run-malloc-benchmarks.lua"
local benchJSONFileBase = "malloc_bench.json"
local mean_to_record = "g"
local metrics = { "executionTime", "memoryAtEnd", "peakMemory", }
local units = {
	["executionTime"] = perfdata.unit.ms,
	["memoryAtEnd"] = perfdata.unit.kilobytes,
	["peakMemory"] = perfdata.unit.kilobytes,
}

local function dirname(path)
	local dirPattern = "(.*)" .. fileSep .. "[^" ..fileSep .. "]*"
	local _, _, dir = path:find(dirPattern)
	return dir
end

-- Use recon-benchrun to run the MallocBench benchmark.

local function runBenchmark()
	-- Construct the path to benchrun-mallocbench.lua, which is assumed to be
	-- in the same directory as this script.
	local mypath = arg[0]
	mypath = (mypath:sub(1, 1) == fileSep and mypath) or lfs.currentdir() .. fileSep .. mypath
	local runbenchPath = dirname(mypath) .. fileSep .. run_benchmark
	local benchJSONFile = (os.getenv("TMPDIR") or "/tmp/") .. fileSep .. benchJSONFileBase

	-- Build the command line for benchrun-mallocbench.lua, based on the
	-- arguments passed to this script.
	local bench_arg = { "/usr/local/bin/recon", runbenchPath, "--json", benchJSONFile }
	for k, v in ipairs(arg) do
		bench_arg[k + 4] = v
	end

	-- Run the benchmark and write the results as perfdata
	print("Create benchrun instance at " .. os.date("%H:%M:%S"))
	local benchmark = benchrun.new{
		name = "libmalloc.MallocBench",
		--- SVN version of MallocBench sources
		version = 238853,
		energy = true,
		cpu_load = true,
		throttle_info = true,
		quiesce = 30,
		sleep = 30,
		iterations = 5,
		arg = { "--tmp", "--no-subdir" }
	}

	local run_index = 1
	local results = {}
	for result in benchmark:run(bench_arg) do
		print("Start iteration at " .. os.date("%H:%M:%S"))
		local f = assert(io.open(benchJSONFile, "r"), "Failed to open JSON file " .. benchJSONFile)
		local run_results = assert(cjson.decode(f:read("a")), "Failed to decode JSON results")
		--- Print all the results so that they are captured in the output.
		print(inspect(run_results))
		print("End iteration at " .. os.date("%H:%M:%S"))
		f:close()
		results[run_index] = run_results
		run_index = run_index + 1
	end

	print("RESULTS (time is " .. os.date("%H:%M:%S") .. ")")
	print(inspect(results))

	for _, result in pairs(results) do
		for _, metric in ipairs(metrics) do
			for allocator, values in pairs(result["means"][metric]) do
				benchmark.writer:add_value(metric, units[metric],
						values[mean_to_record], { ["allocator"] = allocator })
			end
		end
	end

	print("Completing benchrun (time is " .. os.date("%H:%M:%S") .. ")")
	benchmark:finish()
	print("Completed benchrun at " .. os.date("%H:%M:%S"))
end

-- Execution begins here

runBenchmark()
