-- Benchmark name declarations

local benchmarks_all = {
	-- Single-threaded benchmarks.
	"churn",
	"list_allocate",
	"tree_allocate",
	"tree_churn",
	"fragment",
	"fragment_iterate",
	"medium",
	"big",

	-- Benchmarks based on browser recordings.
	"facebook",
	"reddit",
	"flickr",
	"theverge",
	"nimlang",

	-- Multi-threaded benchmark variants.
	"message_one",
	"message_many",
	"churn --parallel",
	"list_allocate --parallel",
	"tree_allocate --parallel",
	"tree_churn --parallel",
	"fragment --parallel",
	"fragment_iterate --parallel",

	-- These tests often crash TCMalloc: <rdar://problem/13657137>.
	"medium --parallel",
	"big --parallel",

	--[[ Enable these tests to test memory footprint. The way they run is not
		really compatible with throughput testing. ]]
	-- "reddit_memory_warning --runs 0",
	-- "flickr_memory_warning --runs 0",
	-- "theverge_memory_warning --runs 0",

	-- Enable this test to test shrinking back down from a large heap while a process remains active.
	-- The way it runs is not really compatible with throughput testing.
	-- "balloon"
	"facebook --parallel",
	"reddit --parallel",
	"flickr --parallel",
	"theverge --parallel",
	-- "nimlang --use-thread-id",
}

local benchmarks_memory = {
	"facebook",
	"reddit",
	"flickr",
	"theverge",
	"nimlang"
}

local benchmarks_memory_warning = {
	"reddit_memory_warning --runs 0",
	"flickr_memory_warning --runs 0",
	"theverge_memory_warning --runs 0",
}

-- Allocator names mapped to the required environemnt variable setting.
local allocator_env_vars = {
	-- Note that bmalloc requires MallocNanoZone=1
	["bmalloc"] = "MallocNanoZone=1",
	["SystemMalloc"] = "MallocNanoZone=0",
	["NanoMallocV1"] = "MallocNanoZone=V1",
	["NanoMallocV2"] = "MallocNanoZone=V2",
}

-- Constants

local executionTimeName = "executionTime"
local peakMemoryName = "peakMemory"
local memoryAtEndName = "memoryAtEnd"
local arithmeticMean = "a"
local geometricMean = "g"
local harmonicMean = "h"
local arithmeticMeanName = "<arithmetic mean>"
local geometricMeanName = "<geometric mean>"
local harmonicMeanName = "<harmonic mean>"

local displayMeanNames = {
	[arithmeticMean] = arithmeticMeanName,
	[geometricMean] = geometricMeanName,
	[harmonicMean] = harmonicMeanName,
}

-- Shared state

local benchmarks = nil
local heap = 0
local quiet = false
local oneRun = false
local mallocBench = nil
local jsonPath = nil
local fileSep = package.config:sub(1,1)
local dirPattern = "(.*)" .. fileSep .. "[^" ..fileSep .. "]*"
local usrLocalLibMbmalloc = "/usr/local/lib/libmbmalloc.dylib"

-- Argument parsing

local usage = [[run-malloc-benchmarks [options] /path/to/MallocBench Name:/path/to/libmbmalloc.dylib [ Name:/path/to/libmbmalloc.dylib ... ]

Runs a suite of memory allocation and access benchmarks.
	
<Name:/path/to/libmbmalloc.dylib> is a symbolic name followed by a path to libmbmalloc.dylib.

Specify \"SystemMalloc\" to test the built-in libc malloc using the system allocators (no NanoMalloc).
Specify \"NanoMalloc\" to test the built-in libc malloc using the default NanoMalloc allocator.
Specify \"NanoMallocV1\" to test the built-in libc malloc using the NanoMalloc V1 allocator.
Specify \"NanoMallocV2\" to test the built-in libc malloc using the NanoMalloc V2 allocator.

Example usage:

	run-malloc-benchmarks /BUILD/MallocBench SystemMalloc:/BUILD/libmbmalloc.dylib NanoMalloc:/BUILD/libmbmalloc.dylib
	run-malloc-benchmarks /BUILD/MallocBench FastMalloc:/BUILD/FastMalloc/libmbmalloc.dylib
	run-malloc-benchmarks --benchmark churn SystemMalloc:/BUILD/libmbmalloc.dylib FastMalloc:/BUILD/FastMalloc/libmbmalloc.dylib
	
Options:

	--one_run                 Run the test only once and without cache warmup.
	--json <filename>         Write the results to "filename" as JSON.
	--quiet                   Do not write the results to standard output.
	--benchmark <benchmark>   Select a single benchmark to run instead of the full suite.
	--heap <heap>             Set a baseline heap size for bmalloc.
	-h, --help                Show this help message and exit.
]]

local function dirname(path)
	local _, _, dir = path:find(dirPattern)
	return dir
end

local function parseArguments(cmdline)
	local function toHeapSize(valueStr)
		local result = tonumber(valueStr)
		return result and result >= 0 and result == math.floor(result) and result or nil
	end

	local function contains_value(table, value) 
		for _, v in pairs(table) do
			if v == value then return true end
		end
		return false
	end

	local argparse = require "argparse"
	local parser = argparse()
	parser:usage(usage)
	parser:help(usage)
	parser:option("--json"):args(1)
	parser:option("--quiet"):args(0)
	parser:option("--heap"):args(1):default(0):convert(toHeapSize)
	parser:option("--benchmark"):args(1)
	parser:option("--memory"):args(0)
	parser:option("--memory_warning"):args(0)
	parser:option("--one_run"):args(0)
	parser:argument("malloc_bench", "Path to MallocBench executable"):args(1)
	parser:argument("dylibs", "One or more mbmalloc dylibs"):args("1+")
	local options = parser:parse(cmdline)

	local lfs = require "lfs"
	local pwd = lfs.currentdir()
	local fileSep = package.config:sub(1,1)

	-- Get the JSON output path, if there is one.
	jsonPath = options["json"]
	if jsonPath then
		jsonPath = (jsonPath:sub(1, 1) == fileSep and jsonPath) or pwd .. fileSep .. jsonPath
	end

	-- Determine whether to run only once
	if options["one_run"] then oneRun = true end

	-- Suppress output to stdout, if requested.
	if options["quiet"] then quiet = true end

	-- Set the heap value, defaulted to 0 (so always present)
	heap = options["heap"]

	-- Set the benchmarks to be run, defaulting to all of them.
	local benchName = options["benchmark"]
	if benchName then
		assert(contains_value(benchmarks_all, benchName)
				or contains_value(benchmarks_memory, benchName),
				"Invalid benchmark name: " .. benchName)
		benchmarks = { benchName }
	end
	if options["memory"] then
		if benchmarks then error("Only one of --benchmark, --memory, --memory_warning can be used") end
		benchmarks = benchmarks_memory
	end
	if options["memory_warning"] then
		if benchmarks then error("Only one of --benchmark, --memory, --memory_warning can be used") end
		benchmarks = benchmarks_memory_warning
	end
	if not benchmarks then benchmarks = benchmarks_all end

	-- Ensure that the malloc_bench executable exists
	mallocBench = options["malloc_bench"]
	mallocBench = (mallocBench:sub(1, 1) == fileSep and mallocBench) or pwd .. fileSep .. mallocBench
	assert(lfs.attributes(mallocBench), string.format("Invalid malloc_bench reference: %s", mallocBench))

	-- Build the list of dylibs. The parser ensures that there is at least one.
	-- Each argument is of the form name:path where path refers to an mbmalloc dylib
	-- and name is the name of the allocator to use.
	local dylibs = {}
	local dylibPaths = {}
	local dylibArgs = options["dylibs"]
	for _, v in pairs(dylibArgs) do
		local s, _, v1, v2 = string.find(v, "^(%w+):(.*)")
		assert(s, string.format("Invalid dylib selector: %s", v))
		assert(allocator_env_vars[v1], string.format("Invalid allocator name: %s", v1))
		local full_path = (v2:sub(1, 1) == fileSep and v2) or pwd .. fileSep .. v2
		assert(lfs.attributes(full_path), string.format("Invalid dylib reference: %s", full_path))
		table.insert(dylibs, v1)
		table.insert(dylibPaths, full_path)
	end

	return dylibs, dylibPaths
end

-- Benchmark execution

local function addResult(results, benchmark, dylib, executionTime, peakMemory, memoryAtEnd)
	results[executionTimeName][dylib][benchmark] = executionTime
	results[peakMemoryName][dylib][benchmark] = peakMemory
	results[memoryAtEndName][dylib][benchmark] = memoryAtEnd
end

local function runBenchmarks(dylibs, dylibPaths)
	-- Initialize the results for each metric and dylib to empty.
	local results = {}
	results[executionTimeName] = {}
	results[peakMemoryName] = {}
	results[memoryAtEndName] = {}
	for _, dylib in pairs(dylibs) do
		results[executionTimeName][dylib] = {}
		results[peakMemoryName][dylib] = {}
		results[memoryAtEndName][dylib] = {}
	end

	-- Run each benchmark for each dylib and add the result to "results".
	local mallocBenchDir = dirname(mallocBench)
	for _, benchmark in pairs(benchmarks) do
		for i, dylib in ipairs(dylibs) do
			local dylibPath = dylibPaths[i]
			local dylibDir = dirname(dylibPath)
			local envVars = "DYLD_LIBRARY_PATH=" .. dylibDir ..
							" " .. allocator_env_vars[dylib]
			envVars = envVars .. " DYLD_PRINT_LIBRARIES=1 "
			local cmd = "cd '" .. mallocBenchDir .. "'; " .. envVars .. " '" .. mallocBench .. "' "
						.. "--benchmark " .. benchmark .. " --heap " .. heap
			if oneRun then cmd = cmd .. " --runs 1 --no-warm" end
io.write(string.format("\nCMD is %s\n", cmd));
			io.write(string.format("\rRUNNING %s: %s...                                ", benchmark, dylib))
			local f = assert(io.popen(cmd, "r"))
			local str = assert(f:read("*a"))
			f:close()

			--[[ Typical result in "str":
					Running churn [ not parallel ] [ don't use-thread-id ] [ heap: 0MB ] [ runs: 8 ]...
					Time:       	69.2164ms
					Peak Memory:	5444kB
					Memory at End:     	876kB
			   Capture the Time, Peak Memory and Memory at End values as the result.
			]]

			local resultLines = {}
			for line in str:gmatch("([^\r\n]+)") do table.insert(resultLines, line) end
			assert(#resultLines == 4, string.format("Unexpected benchmark result: %s\n", str))
			local time = tonumber(resultLines[2]:match("([%d]+)"))
			local peakMemory = tonumber(resultLines[3]:match("([%d]+)"))
			local memoryAtEnd = tonumber(resultLines[4]:match("([%d]+)"))

			addResult(results, benchmark, dylib, time, peakMemory, memoryAtEnd)
		end
	end
	io.write("\r                                                                                \n")

	return results
end

-- Results output

local function computeArithmeticMean(values)
	local sum = 0.0
	local count = 0
	for _, value in pairs(values) do
		sum = sum + value
		count = count + 1
	end
	return math.modf(sum/count)
end

local function computeGeometricMean(values)
	local product = 1.0
	local count = 0
	for _, value in pairs(values) do
		product = product * value
		count = count + 1
	end
	return math.modf(product ^ (1/count))
end

local function computeHarmonicMean(values)
	local sum = 0.0
	local count = 0
	for _, value in pairs(values) do
		sum = sum + 1/value
		count = count + 1
	end
	return math.modf(count/sum)
end

local function lowerIsBetter(a, b, better, worse)
    if a == 0 or b == 0 or b == a then
        return ""
    end

    if b < a then
		return string.format("^ %.2fx %s", a/b, better)
    end

	return string.format("! %.2fx %s", b/a, worse)
end

local function lowerIsFaster(a, b)
    return lowerIsBetter(a, b, "faster", "slower")
end

local function lowerIsSmaller(a, b)
    return lowerIsBetter(a, b, "smaller", "bigger")
end

local function prettify(number, suffix)
	local left, num, right = string.match(number,'^([^%d]*%d)(%d*)(.-)$')
    return left .. num:reverse():gsub('(%d%d%d)','%1,'):reverse() .. right .. suffix
end

local function ljustify(str, width)
	-- string.format does not understand "%*.*s".
	local fmt = string.format("%%%d.%ds", -width, width)
	return string.format(fmt, str)
end

local function rjustify(str, width)
	-- string.format does not understand "%*.*s".
	local fmt = string.format("%%%d.%ds", width, width)
	return string.format(fmt, str)
end

local function calculateMeans(dylibs, results)
	local means = {
		[executionTimeName] = {},
		[peakMemoryName] = {},
		[memoryAtEndName] = {}
	}

	for metric in pairs(means) do
		for _, dylib in ipairs(dylibs) do
			means[metric][dylib] = {}
			means[metric][dylib][geometricMean] = results[metric][dylib] and computeGeometricMean(results[metric][dylib]) or 0
			means[metric][dylib][arithmeticMean] = results[metric][dylib] and computeArithmeticMean(results[metric][dylib]) or 0
			means[metric][dylib][harmonicMean] = results[metric][dylib] and computeHarmonicMean(results[metric][dylib]) or 0
		end
	end

	return means
end

local function printResults(dylibs, results, means)
	local leadingPad = "    "
	local leadingPadLength = #leadingPad

	local width = #arithmeticMeanName
	for _, name in pairs(benchmarks) do
		local w = #name
		if w > width then width = w end
	end
	width = width + leadingPadLength

	local function printHeader(dylibs)
		headers = rjustify("", width) .. rjustify(dylibs[1], width)
		if #dylibs > 1 then
			for i = 2, #dylibs do
				headers = headers .. rjustify(dylibs[i], width) .. rjustify("Δ", width)
			end
		end
		print(headers)
	end

	local function printMean(mean, results, dylibs, means, compareFunction, units)
		-- Display the mean for the first dylib
		local meanName = displayMeanNames[mean]
		local baseDylib = dylibs[1]
		local str = leadingPad .. ljustify(meanName, width - leadingPadLength) ..
					rjustify(prettify(means[baseDylib][mean], units), width)

		-- For each subsequent dylib, show the mean and the ratio wrt the first dylib.
		for index = 2, #dylibs do
			local dylib = dylibs[index]
			str = str .. rjustify(prettify(means[dylib][mean], units), width)
						.. rjustify(compareFunction(means[baseDylib][mean], means[dylib][mean]), width)
		end
		print(str)
	end

	local function printMetric(title, results, means, metricName, compareFunction, units)
		print(title .. ":")

		-- Benchmark results. One row for each benchmark, one column for the first dylib
		-- followed by the result and comparison for the other dylibs.
		for _, benchmark in pairs(benchmarks) do
			local dylib = dylibs[1]
			local measurements = { results[dylib] and results[dylib][benchmark]
					and results[dylib][benchmark] or 0 } 
			local str = leadingPad .. ljustify(benchmark, width - leadingPadLength) ..
					rjustify(prettify(measurements[1], units), width)
			for index = 2, #dylibs do
				dylib = dylibs[index]
				measurements[index] = results[dylib] and results[dylib][benchmark] and results[dylib][benchmark] or 0
				str = str .. rjustify(prettify(measurements[index], units), width)
							.. rjustify(compareFunction(measurements[1], measurements[index]), width)
			end
			print(str)
		end

		-- Means
		print("")
		printMean(geometricMean, results, dylibs, means, compareFunction, units)
		printMean(arithmeticMean, results, dylibs, means, compareFunction, units)
		printMean(harmonicMean, results, dylibs, means, compareFunction, units)
		print("")
	end

	printHeader(dylibs)
	printMetric("Execution Time", results[executionTimeName], means[executionTimeName], executionTimeName, lowerIsFaster, "ms")
	printMetric("Peak Memory", results[peakMemoryName], means[peakMemoryName], peakMemoryName, lowerIsSmaller, "kB")
	printMetric("Memory at End", results[memoryAtEndName], means[memoryAtEndName], memoryAtEndName, lowerIsSmaller, "kB")
end

local function writeJSON(jsonPath, dylibs, results, means)
	local cjson = require 'cjson'
	local fullResults = {
		["results"] = results,
		["means"] = means
	}
	local jsonText = cjson.encode(fullResults)
	local f = assert(io.open(jsonPath, "w"), "Failed to open JSON file " .. jsonPath)
	f:write(jsonText)
	f:close()

	if not quiet then print("JSON results written to " .. jsonPath) end
end

-- Execution begins here

local dylibs, dylibPaths = parseArguments(arg)
local results = runBenchmarks(dylibs, dylibPaths)
local means = calculateMeans(dylibs, results)
if not quiet then printResults(dylibs, results, means) end
if jsonPath then writeJSON(jsonPath, dylibs, results, means) end
