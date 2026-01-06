//
//  vtool.c
//  cctools
//
//  Created by Michael Trent on 12/29/18.
//  Copyright © 2018 apple. All rights reserved.
//

#include <architecture/byte_order.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <mach-o/arch.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stuff/port.h" /* cctools-port: reallocf() */

#ifndef PLATFORM_DRIVERKIT
#define PLATFORM_DRIVERKIT 10
#endif /* PLATFORM_DRIVERKIT */

enum command {
    kCommandUnset = 0,
    kCommandShow,
    kCommandSet,
    kCommandRemove,
    kCommandHelp,
};

enum show_command {
    kShowAll = 0,
    kShowBuild,
    kShowSource,
    kShowSpace,
};

enum set_command {
    kSetBuild,
    kSetSource,
    kAddTool,
};

enum remove_command {
    kRemoveBuild,
    kRemoveSource,
    kRemoveTool,
};

struct set_item {
    enum set_command type;
    uint32_t platform;
    uint32_t tool;
    uint32_t version;
    uint32_t sdk_vers;
    uint64_t src_vers;
    bool versmin;
};

struct remove_item {
    enum remove_command type;
    uint32_t platform;
    uint32_t tool;
};

struct lcmds {
    struct load_command** items;
    uint32_t count;
};

struct file {
    unsigned char* buf;
    off_t len;
    mode_t mode;
    uint32_t nfat_arch;
    struct fat_arch* fat_archs;
    uint32_t fat_arch_idx;
    struct mach_header_64 mh;
    size_t mh_size;
    unsigned char* lcs;  // a pointer into the raw load commands
    struct lcmds* lcmds; // a c-array of pointers to swapped load commands
    bool swap;
};

static struct options {
    enum command command;
    const char* inPath;
    const char* outPath;
    const NXArchInfo **archs;
    uint32_t narch;
    enum show_command show;
    bool replace;
    struct set_item* setItems;
    uint32_t numSetItems;
    struct remove_item* removeItems;
    uint32_t numRemoveItems;
} gOptions;

static const char* gProgramName;
static enum NXByteOrder gByteOrder;

static int process(void);
static int command_remove(struct file* fb);
static int command_set(struct file* fb);
static int command_show(struct file* fb);

static int file_read(const char* path, struct file* fb);
static int file_select_macho(const char* path, struct file* fb, uint32_t index);
static int file_write(const char* path, struct file* fb);

static struct lcmds* lcmds_alloc(struct file* fb);
static void lcmds_free(struct lcmds* lcmds);

static int parse_version_abcde(const char* rostr, uint64_t *version);
static int parse_version_xyz(const char* rostr, uint32_t *version);

static uint32_t platform_id_for_name(const char* name);
static uint32_t platform_id_for_vmlc(uint32_t vmlc);
static const char* platform_name_for_id(uint32_t platform_id);
static uint32_t platform_vmlc_for_id(uint32_t platform_id);

static void print_version_xyz(const char* label, uint32_t version);
static void print_version_min_command(struct version_min_command *vd);
static void print_build_version_command(struct build_version_command *bv);
static void print_build_tool_version(uint32_t tool, uint32_t version);
static void print_source_version_command(struct source_version_command *sv);

static uint32_t tool_id_for_name(const char* name);
static const char* tool_name_for_id(uint32_t tool_id);

static void usage(const char * __restrict format, ...);

int main(int argc, const char * argv[])
{
    bool read_options = true;
    
    gByteOrder = NXHostByteOrder();
    gProgramName = *argv++;
    argc--;
    
    if (argc == 0)
	usage(NULL);
    
    while (argc > 0)
    {
	if (read_options && *argv && '-' == **argv)
	{
	    if (0 == strcmp("-arch", *argv))
	    {
		argv++; argc--;
		
		if (!*argv) {
		    usage("missing arch");
		}
		
		const NXArchInfo *archInfo = NXGetArchInfoFromName(*argv);
		if (!archInfo) {
		    usage("unknown arch: %s", *argv);
		}
		
		gOptions.archs = reallocf(gOptions.archs,
					  sizeof(*gOptions.archs) *
					  (gOptions.narch + 1));
		gOptions.archs[gOptions.narch++] = archInfo;
	    }
	    else if (0 == strcmp("-h", *argv) ||
		     0 == strcmp("-help", *argv))
	    {
		gOptions.command = kCommandHelp;
		usage(NULL);
	    }
	    else if (0 == strcmp("-o", *argv) ||
		     0 == strcmp("-output", *argv))
	    {
		argv++; argc--;
		if (gOptions.outPath) {
		    usage("only one output file must be specified");
		}
		if (!*argv) {
		    usage("one output file must be specified");
		}
		gOptions.outPath = *argv;
	    }
	    else if (0 == strcmp("-r", *argv) ||
		     0 == strcmp("-replace", *argv))
	    {
		gOptions.replace = true;
	    }
	    else if (0 == strcmp("-remove-build-tool", *argv) ||
		     0 == strcmp("-remove-tool", *argv))
	    {
		const char* option = *argv;
		argv++; argc--;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandRemove != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -set "
			  "commands", option);
		}
		
		// make a remove-tool item
		gOptions.command = kCommandRemove;
		gOptions.removeItems = reallocf(gOptions.removeItems,
						sizeof(*gOptions.removeItems) *
						(gOptions.numRemoveItems + 1));
		struct remove_item* item =
		    &gOptions.removeItems[gOptions.numRemoveItems++];
		memset(item, 0, sizeof(*item));
		item->type = kRemoveTool;

		// get the platform id by name, or by literal number.
		if (!*argv) {
		    usage("missing %s platform", option);
		}
		
		item->platform = platform_id_for_name(*argv);
		if (0 == item->platform) {
		    item->platform = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->platform) {
		    usage("unknown platform: %s", *argv);
		}
		argv++; argc--;
		
		// get the tool
		if (!*argv) {
		    usage("missing %s tool", option);
		}
		
		item->tool = tool_id_for_name(*argv);
		if (0 == item->tool) {
		    item->tool = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->tool) {
		    usage("unknown tool: %s", *argv);
		}
	    }
	    else if (0 == strcmp("-remove-build-version", *argv))
	    {
		const char* option = *argv;
		argv++; argc--;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandRemove != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -set "
			  "commands", option);
		}
		
		gOptions.command = kCommandRemove;
		gOptions.removeItems = reallocf(gOptions.removeItems,
						sizeof(*gOptions.removeItems) *
						(gOptions.numRemoveItems + 1));
		struct remove_item* item =
		    &gOptions.removeItems[gOptions.numRemoveItems++];
		memset(item, 0, sizeof(*item));
		item->type = kRemoveBuild;

		// get the platform id by name, or by literal number.
		if (!*argv) {
		    usage("missing %s platform", option);
		}
		
		item->platform = platform_id_for_name(*argv);
		if (0 == item->platform) {
		    item->platform = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->platform) {
		    usage("unknown platform: %s", *argv);
		}
	    }
	    else if (0 == strcmp("-remove-source-version", *argv))
	    {
		const char* option = *argv;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandRemove != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -set "
			  "commands", option);
		}
		
		gOptions.command = kCommandRemove;
		gOptions.removeItems = reallocf(gOptions.removeItems,
						sizeof(*gOptions.removeItems) *
						(gOptions.numRemoveItems + 1));
		struct remove_item* item =
		    &gOptions.removeItems[gOptions.numRemoveItems++];
		memset(item, 0, sizeof(*item));
		item->type = kRemoveSource;
	    }
	    else if (0 == strcmp("-set-tool", *argv) ||
		     0 == strcmp("-set-build-tool", *argv))
	    {
		const char* option = *argv;
		argv++; argc--;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandSet != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -remove "
			  "commands", option);
		}
		
		// make an add-tool item
		gOptions.command = kCommandSet;
		gOptions.setItems = reallocf(gOptions.setItems,
					     sizeof(*gOptions.setItems) *
					     (gOptions.numSetItems + 1));
		struct set_item* item =
		&gOptions.setItems[gOptions.numSetItems++];
		memset(item, 0, sizeof(*item));
		item->type = kAddTool;
		
		// get the platform id by name, or by literal number.
		if (!*argv) {
		    usage("missing %s platform", option);
		}
		
		item->platform = platform_id_for_name(*argv);
		if (0 == item->platform) {
		    item->platform = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->platform) {
		    usage("unknown platform: %s", *argv);
		}
		argv++; argc--;
		
		// get the tool
		if (!*argv) {
		    usage("missing %s tool", option);
		}
		
		item->tool = tool_id_for_name(*argv);
		if (0 == item->tool) {
		    item->tool = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->tool) {
		    usage("unknown tool: %s", *argv);
		}
		argv++; argc--;
		
		// get the version
		if (!*argv) {
		    usage("missing %s version", option);
		}
		
		if (parse_version_xyz(*argv, &item->version)) {
		    usage("bad version: %s", gProgramName, *argv);
		}
	    }

	    else if (0 == strcmp("-set-build", *argv) ||
		     0 == strcmp("-set-build-version", *argv) ||
		     0 == strcmp("-set-version-min", *argv))
	    {
		const char* option = *argv;
		argv++; argc--;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandSet != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -remove "
			  "commands", option);
		}
		
		gOptions.command = kCommandSet;
		gOptions.setItems = reallocf(gOptions.setItems,
					     sizeof(*gOptions.setItems) *
					     (gOptions.numSetItems + 1));
		struct set_item* item =
		    &gOptions.setItems[gOptions.numSetItems++];
		memset(item, 0, sizeof(*item));
		item->type = kSetBuild;
		item->versmin = (0 == strcmp("-set-version-min", option));
		
		// get the platform id by name, or by literal number.
		if (!*argv) {
		    usage("missing %s platform", option);
		}
		
		item->platform = platform_id_for_name(*argv);
		if (0 == item->platform) {
		    item->platform = (uint32_t)strtol(*argv, NULL, 0);
		}
		if (0 == item->platform) {
		    usage("unknown platform: %s", *argv);
		}
		
		if (item->versmin && 0 == platform_vmlc_for_id(item->platform)){
		    usage("version min unsupported for platform: %s", *argv);
		}
		argv++; argc--;
		
		// get the minos version
		if (!*argv) {
		    usage("missing %s min_version", option);
		}
		
		if (parse_version_xyz(*argv, &item->version)) {
		    usage("bad min_version: %s", gProgramName, *argv);
		}
		argv++; argc--;
		
		// get the sdk version
		if (!*argv) {
		    usage("missing %s sdk_version", option);
		}
		
		if (parse_version_xyz(*argv, &item->sdk_vers)) {
		    usage("bad sdk_version: %s", gProgramName, *argv);
		}
		
		while (*(argv + 1) && 0 == strcmp("-tool", *(argv + 1)))
		{
		    uint32_t platform = item->platform;
		    
		    argv++; argc--;
		    argv++; argc--;
		    
		    if (item->versmin) {
			usage("-tool cannot be used with %s", option);
		    }
		    
		    // make an add-tool item
		    gOptions.setItems = reallocf(gOptions.setItems,
						 sizeof(*gOptions.setItems) *
						 (gOptions.numSetItems + 1));
		    item = &gOptions.setItems[gOptions.numSetItems++];
		    memset(item, 0, sizeof(*item));
		    item->type = kAddTool;
		    item->platform = platform;
		    
		    // get the tool
		    if (!*argv) {
			usage("missing %s -tool tool", option);
		    }
		    
		    item->tool = tool_id_for_name(*argv);
		    if (0 == item->tool) {
			item->tool = (uint32_t)strtol(*argv, NULL, 0);
		    }
		    if (0 == item->tool) {
			usage("unknown tool: %s", *argv);
		    }
		    argv++; argc--;
		    
		    // get the version
		    if (!*argv) {
			usage("missing %s -tool version", option);
		    }
		    
		    if (parse_version_xyz(*argv, &item->version)) {
			usage("bad version: %s", gProgramName, *argv);
		    }
		} // 0 == strcmp("tool", *(argv + 1))
	    } // if -set-build-version || -set-version-min
	    else if (0 == strcmp("-set-source", *argv) ||
		     0 == strcmp("-set-source-version", *argv))
	    {
		const char* option = *argv;
		argv++; argc--;
		
		if (kCommandUnset != gOptions.command &&
		    kCommandSet != gOptions.command)
		{
		    usage("option %s cannot be used with -show or -remove "
			  "commands", *argv);
		}
		
		gOptions.command = kCommandSet;
		gOptions.setItems = reallocf(gOptions.setItems,
					     sizeof(*gOptions.setItems) *
					     (gOptions.numSetItems + 1));
		struct set_item* item =
		    &gOptions.setItems[gOptions.numSetItems++];
		memset(item, 0, sizeof(*item));
		item->type = kSetSource;
		
		// get the source version
		if (!*argv) {
		    usage("missing %s version", option);
		}
		
		if (parse_version_abcde(*argv, &item->src_vers)) {
		    usage("bad version: %s", gProgramName, *argv);
		}
	    } // -set-source-version
	    else if (0 == strcmp("-show", *argv) ||
		     0 == strcmp("-show-all", *argv))
	    {
		if (gOptions.command)
		    usage("option %s cannot be used with other commands",
			  *argv);
		gOptions.command = kCommandShow;
		gOptions.show = kShowAll;
	    }
	    else if (0 == strcmp("-show-build", *argv) ||
		     0 == strcmp("-show-build-version", *argv))
	    {
		if (gOptions.command)
		    usage("option %s cannot be used with other commands",
			  *argv);
		gOptions.command = kCommandShow;
		gOptions.show = kShowBuild;
	    }
	    else if (0 == strcmp("-show-source", *argv) ||
		     0 == strcmp("-show-source-version", *argv))
	    {
		if (gOptions.command)
		    usage("option %s cannot be used with other commands",
			  *argv);
		gOptions.command = kCommandShow;
		gOptions.show = kShowSource;
	    }
	    else if (0 == strcmp("-show-space", *argv))
	    {
		if (gOptions.command)
		    usage("option %s cannot be used with other commands",
			  *argv);
		gOptions.command = kCommandShow;
		gOptions.show = kShowSpace;
	    }
	    else if (0 == strcmp("-", *argv))
	    {
		read_options = false;
	    }
	    else
	    {
		usage("unknown option: %s", *argv);
	    }
	} // if ('-' == **argv)
	else
	{
	    if (gOptions.inPath)
		usage("only one input file must be specified");
	    gOptions.inPath = *argv;
	}
	
	argv++; argc--;
    } // while (argc > 0)
    
    if (!gOptions.inPath)
	usage("one input file must be specified");
    
    if (gOptions.command == kCommandUnset)
	usage("a -show, -set, or -remove command must be specified");
    
    if ((gOptions.command==kCommandSet || gOptions.command==kCommandRemove) &&
	!gOptions.outPath)
	usage("one output file must be specified");
    
    if (gOptions.command==kCommandShow && gOptions.outPath)
	usage("-show commands do not write output");

    // make sure set options are not specified more than once per platform
    for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    {
	if (kSetBuild != gOptions.setItems[iitem].type)
	    continue;
	for (uint32_t jitem = 0; jitem < iitem; ++jitem)
	{
	    if (kSetBuild != gOptions.setItems[jitem].type)
		continue;
	    if (gOptions.setItems[iitem].platform ==
		gOptions.setItems[jitem].platform)
	    {
		uint32_t plid = gOptions.setItems[iitem].platform;
		const char* plname = platform_name_for_id(plid);
		if (plname)
		    usage("more than one build version specified for "
			  "platform %s", plname);
		else
		    usage("more than one build version specified for "
			  "platform #%u", plid);
	    }
	}
    }
    
    bool foundsrc = false;
    for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    {
	if (kSetSource != gOptions.setItems[iitem].type)
	    continue;
	if (foundsrc) {
	    usage("more than one source version specified");
	}
	foundsrc = true;
    }

    for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    {
	if (kAddTool != gOptions.setItems[iitem].type)
	    continue;
	for (uint32_t jitem = 0; jitem < iitem; ++jitem)
	{
	    if (kAddTool != gOptions.setItems[jitem].type)
		continue;
	    if (gOptions.setItems[iitem].platform ==
		gOptions.setItems[jitem].platform &&
		gOptions.setItems[iitem].tool ==
		gOptions.setItems[jitem].tool)
	    {
		uint32_t plid = gOptions.setItems[iitem].platform;
		const char* plname = platform_name_for_id(plid);
		uint32_t tlid = gOptions.setItems[iitem].tool;
		const char* tlname = tool_name_for_id(tlid);
		if (plname && tlname)
		    usage("more than one tool version specified for "
			  "platform %s, tool %s", plname, tlname);
		else if (plname)
		    usage("more than one tool version specified for "
			  "platform %s, tool #%u", plname, tlid);
		if (tlname)
		    usage("more than one tool version specified for "
			  "platform #%u, tool %s", plid, tlname);
		else
		    usage("more than one tool version specified for "
			  "platform #%u, tool #%u", plid, tlid);
	    }
	}
    }

    // do work!
    int res = process();
    
    return res ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*
 * process opens the input file for reading, calls the requested command
 * handler, and if necessary, writes the resulting data back to disk. process
 * manages all of the details of processing fat files, filtering on
 * architectures, and so on.
 */
int process(void)
{
    struct file file;
    int res = 0;
    
    // read the input file
    if (0 == res)
	res = file_read(gOptions.inPath, &file);
    
    // verify the specified archs exist in the file
    if (0 == res && gOptions.narch)
    {
	for (uint32_t iopt = 0; iopt < gOptions.narch; ++iopt)
	{
	    const NXArchInfo* a = gOptions.archs[iopt];
	    bool found = false;
	    
	    for (uint32_t iarch = 0; false == found && iarch < file.nfat_arch;
		 ++iarch)
	    {
		struct fat_arch* b = &file.fat_archs[iarch];
		if (a->cputype == b->cputype &&
		    (a->cpusubtype & ~CPU_SUBTYPE_MASK) ==
		    (b->cpusubtype & ~CPU_SUBTYPE_MASK))
		    found = true;
	    }
	    if (false == found) {
		fprintf(stderr, "%s error: %s file does not contain "
			"architecture: %s\n", gProgramName, gOptions.inPath,
			a->name);
		res = -1;
	    }
	}
    }
    
    // process architectures in the order they appear in the file
    for (uint32_t iarch = 0; 0 == res && iarch < file.nfat_arch; ++iarch)
    {
	// prepare the mach-o for reading
	res = file_select_macho(gOptions.inPath, &file, iarch);
	
	// filter on architecture if necessary
	if (0 == res && gOptions.narch)
	{
	    bool found = false;
	    for (uint32_t j = 0; false == found && j < gOptions.narch; ++j)
	    {
		if (gOptions.archs[j]->cputype == file.mh.cputype &&
		    (gOptions.archs[j]->cpusubtype & ~CPU_SUBTYPE_MASK) ==
		    (file.mh.cpusubtype & ~CPU_SUBTYPE_MASK))
		{
		    found = true;
		}
	    }
	    if (!found)
		continue;
	}
	
	// dispatch the requested commands once per Mach-O
	if (0 == res) {
	    if (kCommandShow == gOptions.command) {
		res = command_show(&file);
	    } else if (kCommandSet == gOptions.command) {
		res = command_set(&file);
	    } else if (kCommandRemove == gOptions.command) {
		res = command_remove(&file);
	    }
	}
    } // for (uint32_t iarch = 0; 0 == res && iarch < file.nfat_arch; ++iarch)
    
    // write out the final file
    if (0 == res &&
	(gOptions.command == kCommandSet || gOptions.command == kCommandRemove))
    {
	res = file_write(gOptions.outPath, &file);
    }
    
    return res;
}

/*
 * command_remove removes the requested version commands in a Mach-O file or
 * Mach-O files in a fat file.
 */
int command_remove(struct file* fb)
{
    // see command_set for detailed notes on how the Mach-O data is updated.
    // command_remove is a simplified set of those same instructions, although
    // the basic framework remains the same:
    //
    //   1. build a new array of "set instructions" for modifying build versions
    //      using a "remove build tool" instruction. That is, removing a
    //      build tool is the same as modifying a build version.
    //
    //   2. measure the space between the mach_header and the first segment
    //      section as well as the space required by the new load commands.
    //
    //   3. verify the new load commands will fit in the existing structure.
    //
    //   4. copy the unmodified load commands into a new buffer, and write new
    //      load commands (see step 1) at the end of that same buffer.
    //
    //   5. write the load commands and an adjusted mach_header back into
    //      the file buffer.
    //
    // Like with command_set, command_remove stops short of writing the new
    // data to disk; that will be the calling code's job.
    
    // cache the swapped load command table in a convenient form
    struct lcmds* lcmds = fb->lcmds;
    
    // build a new setItems array for build version commands whose tools are
    // being changed. If a tool is being removed from a build version that
    // is not present in the file, return an error.
    struct set_item* items = NULL;
    uint32_t nitem = 0;
    for (uint32_t iitem = 0; iitem < gOptions.numRemoveItems; ++iitem)
    {
	if (kRemoveTool != gOptions.removeItems[iitem].type)
	    continue;
	
	uint32_t platform = gOptions.removeItems[iitem].platform;
	struct build_version_command* bv = NULL;
	bool found = false;
	
	// look for this tool's corresponding build version in the program
	// arguments. if found, the entire build version is going away and
	// there's no need to do more.
	if (!found)
	{
	    for (uint32_t jitem = 0; !found &&
		 jitem < gOptions.numRemoveItems; ++jitem)
	    {
		if (kRemoveBuild == gOptions.removeItems[jitem].type &&
		    platform == gOptions.removeItems[jitem].platform) {
		    found = true;
		}
	    }
	}
	
	// look for this tool's corresponding build version in the input
	// file's load commands.
	if (!found)
	{
	    for (uint32_t icmd = 0; !found && icmd < lcmds->count; ++icmd)
	    {
		struct load_command* lc = lcmds->items[icmd];
		if (lc->cmd == LC_BUILD_VERSION)
		{
		    bv = (struct build_version_command*)lc;
		    found = (platform == bv->platform);
		}
		else if (lc->cmd == LC_VERSION_MIN_MACOSX ||
			 lc->cmd == LC_VERSION_MIN_IPHONEOS ||
			 lc->cmd == LC_VERSION_MIN_WATCHOS ||
			 lc->cmd == LC_VERSION_MIN_TVOS)
		{
		    if (platform_id_for_vmlc(lc->cmd) == platform) {
			fprintf(stderr, "%s error: %s version min load "
				"commands do not support tool versions\n",
				gProgramName, gOptions.inPath);
			return -1;
		    }
		}
	    }
	}
	
	// we looked and we looked but we didn't find a build version for
	// this tool. return an error.
	if (!found)
	{
	    const char* plname = platform_name_for_id(platform);
	    if (plname)
		fprintf(stderr, "%s error: no build verion load command "
			"found for platform %s\n", gProgramName, plname);
	    else
		fprintf(stderr, "%s error: no build verion load command "
			"found for platform #%u\n", gProgramName, platform);
	    return -1;
	}
	
	// if this tool is modifying a build version already in the input
	// file, and it's not being completely removed, add it to the list.
	// Also add this build version's tools to the list if they aren't
	// themselves being replaced.
	if (bv)
	{
	    items = reallocf(items, sizeof(*items) * (nitem+1));
	    struct set_item* item = &items[nitem++];
	    memset(item, 0, sizeof(*item));
	    item->type = kSetBuild;
	    item->platform = platform;
	    item->version = bv->minos;
	    item->sdk_vers = bv->sdk;
	    
	    for (uint32_t itool = 0; itool < bv->ntools; ++itool)
	    {
		struct build_tool_version* bt =
		(struct build_tool_version*)
		(((unsigned char*)(bv+1)) +
		 (itool * sizeof(struct build_tool_version)));
		found = false;
		for (uint32_t jitem = 0; !found &&
		     jitem < gOptions.numRemoveItems; ++jitem)
		{
		    if (platform == gOptions.removeItems[jitem].platform &&
			bt->tool == gOptions.removeItems[jitem].tool) {
			found = true;
		    }
		}
		
		if (!found) {
		    items = reallocf(items, sizeof(*items) * (nitem+1));
		    item = &items[nitem++];
		    memset(item, 0, sizeof(*item));
		    item->type = kAddTool;
		    item->platform = platform;
		    item->tool = bt->tool;
		    item->version = bt->version;
		}
	    }
	} // if (bv)
    } // for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    
    // measure the offset to the first section and the size of load commands
    // that will be modified. (Theoretically we're only making the load
    // commands smaller here; but it doesn't hurt much to be careful.)
    off_t sectoffset = fb->fat_archs[fb->fat_arch_idx].size;
    uint32_t modvlcsize = 0;
    
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    {
	struct load_command* lc = lcmds->items[icmd];
	if (lc->cmd == LC_SEGMENT)
	{
	    // walk the sections
	    struct segment_command* sg = (struct segment_command*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section* sc = (struct section*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	else if (lc->cmd == LC_SEGMENT_64)
	{
	    // walk the sections
	    struct segment_command_64* sg = (struct segment_command_64*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section_64* sc = (struct section_64*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	else if (lc->cmd == LC_VERSION_MIN_MACOSX ||
		 lc->cmd == LC_VERSION_MIN_IPHONEOS ||
		 lc->cmd == LC_VERSION_MIN_WATCHOS ||
		 lc->cmd == LC_VERSION_MIN_TVOS ||
		 lc->cmd == LC_BUILD_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    bool modify = false;
	    
	    uint32_t platform;
	    if (lc->cmd == LC_BUILD_VERSION) {
		struct build_version_command* bv =
		(struct build_version_command*)lc;
		platform = bv->platform;
	    }
	    else {
		platform = platform_id_for_vmlc(lc->cmd);
	    }
	    
	    for (uint32_t iitem=0; !modify &&
		 iitem < gOptions.numRemoveItems; ++iitem)
	    {
		if (gOptions.removeItems[iitem].type == kRemoveBuild &&
		    gOptions.removeItems[iitem].platform == platform)
		    modify = true;
	    }
	    for (uint32_t iitem=0; !modify && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetBuild &&
		    items[iitem].platform == platform)
		    modify = true;
	    }
	    
	    if (modify)
		modvlcsize += lc->cmdsize;
	}
	else if (lc->cmd == LC_SOURCE_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    bool modify = false;
	    
	    for (uint32_t iitem=0; !modify &&
		 iitem < gOptions.numRemoveItems; ++iitem)
	    {
		if (gOptions.removeItems[iitem].type == kRemoveSource)
		    modify = true;
	    }
	    
	    if (modify)
		modvlcsize += lc->cmdsize;
	}
    } // for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    
    // compute the size requirements for our new load commands by
    // subtracting away the existing commands and adding in the
    // user-requested ones.
    uint32_t sizeofnewcmds = fb->mh.sizeofcmds;
    sizeofnewcmds -= modvlcsize;
    
    for (uint32_t iitem=0; iitem < nitem; ++iitem)
    {
	if (items[iitem].type == kAddTool)
	    sizeofnewcmds += sizeof(struct build_tool_version);
	if (items[iitem].type == kSetBuild && !items[iitem].versmin)
	    sizeofnewcmds += sizeof(struct build_version_command);
    }
    
    // verify the load commands still fit below the start of section data.
    uint32_t totalcmdspace = (uint32_t)(sectoffset - fb->mh_size);
    if (totalcmdspace < sizeofnewcmds)
    {
	if (fb->nfat_arch > 1 || gOptions.narch) {
	    const NXArchInfo* archInfo = NULL;
	    archInfo = NXGetArchInfoFromCpuType(fb->mh.cputype,
						fb->mh.cpusubtype);
	    if (archInfo)
		fprintf(stderr, "%s error: %s (%s) not enough space to "
			"hold load commands\n", gProgramName,
			gOptions.inPath, archInfo->name);
	    else
		fprintf(stderr, "%s error: %s (%u, %u) not enough space to "
			"hold load commands\n", gProgramName,
			gOptions.inPath, fb->mh.cputype, fb->mh.cpusubtype);
	}
	else {
	    fprintf(stderr, "%s error: %s not enough space to hold load "
		    "commands\n", gProgramName, gOptions.inPath);
	}
	
	return -1;
    }
    
    // copy all of the non-version load commands from the input file into a
    // new buffer. keep count of the number of commands written.
    //
    // note that the newcmds array is zero-filled and is exactly the size of
    // available space. the entire buffer will be copied back into the file,
    // erasing any previous 
    unsigned char* newcmds = calloc(1, totalcmdspace);
    unsigned char* p = fb->lcs;
    unsigned char* q = newcmds;
    uint32_t ncmds = 0;
    
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    {
	struct load_command* lc = lcmds->items[icmd];
	bool copy = true;
	
	if (lc->cmd == LC_VERSION_MIN_MACOSX ||
	    lc->cmd == LC_VERSION_MIN_IPHONEOS ||
	    lc->cmd == LC_VERSION_MIN_WATCHOS ||
	    lc->cmd == LC_VERSION_MIN_TVOS ||
	    lc->cmd == LC_BUILD_VERSION)
	{
	    uint32_t platform;
	    if (lc->cmd == LC_BUILD_VERSION) {
		struct build_version_command* bv =
		(struct build_version_command*)lc;
		platform = bv->platform;
	    }
	    else {
		platform = platform_id_for_vmlc(lc->cmd);
	    }
	    
	    for (uint32_t iitem=0; copy &&
		 iitem < gOptions.numRemoveItems; ++iitem)
	    {
		if (gOptions.removeItems[iitem].type == kRemoveBuild &&
		    gOptions.removeItems[iitem].platform == platform)
		    copy = false;
	    }
	    for (uint32_t iitem=0; copy && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetBuild &&
		    items[iitem].platform == platform)
		    copy = false;
	    }
	}
	else if (lc->cmd == LC_SOURCE_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    for (uint32_t iitem=0; copy &&
		 iitem < gOptions.numRemoveItems; ++iitem)
	    {
		if (gOptions.removeItems[iitem].type == kRemoveSource)
		    copy = false;
	    }
	}
	
	// copy unswapped load commands from p to q, skipping version cmds.
	if (copy) {
	    memcpy(q, p, lc->cmdsize);
	    q += lc->cmdsize;
	    ncmds += 1;
	}
	
	// advance the unswapped load command pointer
	p += lc->cmdsize;
    } // for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    
    // write the new version commands to the output buffer
    for (uint32_t iitem=0; iitem < nitem; ++iitem)
    {
	if (items[iitem].type == kSetBuild) {
	    struct build_version_command* bv =
	    (struct build_version_command*)q;
	    
	    bv->cmd = LC_BUILD_VERSION;
	    bv->cmdsize = sizeof(*bv); // for now
	    bv->platform = items[iitem].platform;
	    bv->minos = items[iitem].version;
	    bv->sdk = items[iitem].sdk_vers;
	    bv->ntools = 0;
	    q += bv->cmdsize;
	    
	    struct build_tool_version* tools =
	    (struct build_tool_version*)q;
	    
	    // copy the tools for this platform
	    for (uint32_t itool=iitem + 1; itool < nitem; ++itool) {
		if (items[itool].type != kAddTool)
		    continue;
		if (items[itool].platform != bv->platform)
		    continue;
		struct build_tool_version* bt =
		(struct build_tool_version*)q;
		bt->tool = items[itool].tool;
		bt->version = items[itool].version;
		q += sizeof(*bt);
		bv->ntools += 1;
		bv->cmdsize += sizeof(*bt);
	    }
	    
	    if (fb->swap) {
		swap_build_version_command(bv, gByteOrder);
		swap_build_tool_version(tools, bv->ntools, gByteOrder);
	    }
	    
	    ncmds += 1;
	}
    }
    
    // update the mach_header and load commands in memory
    struct mach_header_64 mh;
    memcpy(&mh, &fb->mh, sizeof(mh));
    mh.sizeofcmds = sizeofnewcmds;
    mh.ncmds = ncmds;
    if (fb->swap)
	swap_mach_header_64(&mh, gByteOrder);
    
    q = fb->buf + fb->fat_archs[fb->fat_arch_idx].offset;
    memcpy(q, &mh, fb->mh_size);
    q += fb->mh_size;
    memcpy(q, newcmds, totalcmdspace);
    
    free(newcmds);
    free(items);
    
    return 0;
}

/*
 * command_set() adds or replaces the requested version commands in a Mach-O
 * file or Mach-O files in a fat file.
 */
int command_set(struct file* fb)
{
    // the write operation for a Mach-O takes place in several phases:
    //
    //   1. create a new list of set instructions with the "set build version"
    //      instructions sorted towards the front and the "set build tool"
    //      instructions sorted towards the end. For each "set build tool"
    //      instruction verify a "set build version" exists for that platform,
    //      creating a new set instruction from the source file if needed.
    //      support for adding new build tools makes this tricksy.
    //
    //   2. loop over the load commands to find the start of section contents,
    //      and measure the size of the existing version load commands.
    //
    //   3. compute the space required to hold the new load commands, and make
    //      sure they fit in the available space.
    //
    //   4. copy all of the load commands that aren't being changed or replaced
    //      into a new temporary buffer, then add the inserted and modified load
    //      commands at the end of said buffer.
    //
    //   5. replace the load commands and rewrite the mach header for the
    //      Mach-O stored in the fb buffer, so that later on someone can write
    //      that buffer back out to disk again.
    //
    // command_set does not change the file's size in any way. instead, it will
    // rewrite the contents of the file in memory, leaving the broader structure
    // of the Mach-O completely unchanged. if the new load commands do not fit
    // in the existing space between the mach_header and the first segment,
    // command_set will return an error.
    //
    // the input file on disk is not modified in this process, although the
    // copy in memory totally is.
    
    // cache the swapped load command table in a convenient form
    struct lcmds* lcmds = fb->lcmds;

    // build a new setItems array, verifying a build version exists for
    // every tool. If a build version was not specified on the command line
    // look for the build version in the source file.
    struct set_item* items = NULL;
    uint32_t nitem = 0;
    for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    {
	if (kAddTool == gOptions.setItems[iitem].type)
	    continue;
	// add this build version or source version to the list
	items = reallocf(items, sizeof(*items) * (nitem+1));
	memcpy(&items[nitem++], &gOptions.setItems[iitem], sizeof(*items));
    }
    for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    {
	if (kAddTool != gOptions.setItems[iitem].type)
	    continue;
	
	uint32_t platform = gOptions.setItems[iitem].platform;
	struct build_version_command* bv = NULL;
	bool found = false;
	
	// look for this tool's corresponding build version in the program
	// arguments.
	if (!found)
	{
	    for (uint32_t jitem = 0; !found && jitem < nitem; ++jitem)
	    {
		if (platform == items[jitem].platform &&
		    kAddTool != items[jitem].type)
		{
		    if (items[jitem].versmin) {
			fprintf(stderr, "%s error: version min load commands "
				"do not support tool versions\n", gProgramName);
			return -1;
		    }
		    found = true;
		}
	    }
	}
	
	// look for this tool's corresponding build version in the input
	// file's load commands.
	if (!found)
	{
	    for (uint32_t icmd = 0; !found && icmd < lcmds->count; ++icmd)
	    {
		struct load_command* lc = lcmds->items[icmd];
		if (lc->cmd == LC_BUILD_VERSION)
		{
		    bv = (struct build_version_command*)lc;
		    found = (platform == bv->platform);
		}
		else if (lc->cmd == LC_VERSION_MIN_MACOSX ||
			 lc->cmd == LC_VERSION_MIN_IPHONEOS ||
			 lc->cmd == LC_VERSION_MIN_WATCHOS ||
			 lc->cmd == LC_VERSION_MIN_TVOS)
		{
		    if (platform_id_for_vmlc(lc->cmd) == platform) {
			fprintf(stderr, "%s error: %s version min load "
				"commands do not support tool versions\n",
				gProgramName, gOptions.inPath);
			return -1;
		    }
		}
	    }
	}
	
	// we looked and we looked but we didn't find a build version for
	// this tool. return an error.
	if (!found)
	{
	    const char* plname = platform_name_for_id(platform);
	    if (plname)
		fprintf(stderr, "%s error: no build verion load command found "
			"for platform %s\n", gProgramName, plname);
	    else
		fprintf(stderr, "%s error: no build verion load command found "
			"for platform #%u\n", gProgramName, platform);
	    return -1;
	}
	
	// if this tool is modifying a build version already in the input
	// file, add it to the list. Also add this build version's tools
	// to the list if they aren't otherwise being replaced.
	if (bv)
	{
	    // we found a build version load command for this tool, so
	    // add it to the list. Also, add all the tools for this build
	    // version except those we have options for.
	    items = reallocf(items, sizeof(*items) * (nitem+1));
	    struct set_item* item = &items[nitem++];
	    memset(item, 0, sizeof(*item));
	    item->type = kSetBuild;
	    item->platform = platform;
	    item->version = bv->minos;
	    item->sdk_vers = bv->sdk;
	    
	    if (!gOptions.replace) {
		for (uint32_t itool = 0; itool < bv->ntools; ++itool)
		{
		    struct build_tool_version* bt =
		    (struct build_tool_version*)
		    (((unsigned char*)(bv+1)) +
		     (itool * sizeof(struct build_tool_version)));
		    found = false;
		    for (uint32_t jitem = 0; !found &&
			 jitem < gOptions.numSetItems; ++jitem)
		    {
			if (platform == gOptions.setItems[jitem].platform &&
			    bt->tool == gOptions.setItems[jitem].tool) {
			    found = true;
			}
		    }
		    
		    if (!found) {
			items = reallocf(items, sizeof(*items) * (nitem+1));
			item = &items[nitem++];
			memset(item, 0, sizeof(*item));
			item->type = kAddTool;
			item->platform = platform;
			item->tool = bt->tool;
			item->version = bt->version;
		    }
		}
	    }
	} // if (bv)
	
	// finally, add this tool to the list.
	items = reallocf(items, sizeof(*items) * (nitem+1));
	memcpy(&items[nitem++], &gOptions.setItems[iitem], sizeof(*items));
    } // for (uint32_t iitem = 0; iitem < gOptions.numSetItems; ++iitem)
    
    // measure the offset to the first section and the size of load commands
    // that will be modified
    off_t sectoffset = fb->fat_archs[fb->fat_arch_idx].size;
    uint32_t modvlcsize = 0;
    
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    {
	struct load_command* lc = lcmds->items[icmd];
	if (lc->cmd == LC_SEGMENT)
	{
	    // walk the sections
	    struct segment_command* sg = (struct segment_command*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section* sc = (struct section*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	else if (lc->cmd == LC_SEGMENT_64)
	{
	    // walk the sections
	    struct segment_command_64* sg = (struct segment_command_64*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section_64* sc = (struct section_64*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	else if (lc->cmd == LC_VERSION_MIN_MACOSX ||
		 lc->cmd == LC_VERSION_MIN_IPHONEOS ||
		 lc->cmd == LC_VERSION_MIN_WATCHOS ||
		 lc->cmd == LC_VERSION_MIN_TVOS ||
		 lc->cmd == LC_BUILD_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    bool modify = gOptions.replace;
	    
	    uint32_t platform;
	    if (lc->cmd == LC_BUILD_VERSION) {
		struct build_version_command* bv =
		(struct build_version_command*)lc;
		platform = bv->platform;
	    }
	    else {
		platform = platform_id_for_vmlc(lc->cmd);
	    }
	    
	    for (uint32_t iitem=0; !modify && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetBuild &&
		    items[iitem].platform == platform)
		    modify = true;
	    }
	    
	    if (modify)
		modvlcsize += lc->cmdsize;
	}
	else if (lc->cmd == LC_SOURCE_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    bool modify = false;
	    
	    for (uint32_t iitem=0; !modify && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetSource)
		    modify = true;
	    }
	    
	    if (modify)
		modvlcsize += lc->cmdsize;
	}
    } // for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    
    // compute the size requirements for our new load commands by
    // subtracting away the existing commands and adding in the
    // user-requested ones.
    uint32_t sizeofnewcmds = fb->mh.sizeofcmds;
    sizeofnewcmds -= modvlcsize;
    
    for (uint32_t iitem=0; iitem < nitem; ++iitem)
    {
	if (items[iitem].type == kSetSource)
	    sizeofnewcmds += sizeof(struct source_version_command);
	if (items[iitem].type == kAddTool)
	    sizeofnewcmds += sizeof(struct build_tool_version);
	if (items[iitem].type == kSetBuild && items[iitem].versmin)
	    sizeofnewcmds += sizeof(struct version_min_command);
	if (items[iitem].type == kSetBuild && !items[iitem].versmin)
	    sizeofnewcmds += sizeof(struct build_version_command);
    }
    
    // verify the load commands still fit below the start of section data.
    uint32_t totalcmdspace = (uint32_t)(sectoffset - fb->mh_size);
    if (totalcmdspace < sizeofnewcmds)
    {
	if (fb->nfat_arch > 1 || gOptions.narch) {
	    const NXArchInfo* archInfo = NULL;
	    archInfo = NXGetArchInfoFromCpuType(fb->mh.cputype,
						fb->mh.cpusubtype);
	    if (archInfo)
		fprintf(stderr, "%s error: %s (%s) not enough space to "
			"hold load commands\n", gProgramName,
			gOptions.inPath, archInfo->name);
	    else
		fprintf(stderr, "%s error: %s (%u, %u) not enough space to "
			"hold load commands\n", gProgramName,
			gOptions.inPath, fb->mh.cputype, fb->mh.cpusubtype);
	}
	else {
	    fprintf(stderr, "%s error: %s not enough space to hold load "
		    "commands\n", gProgramName, gOptions.inPath);
	}
	
	return -1;
    }

    // copy all of the non-version load commands from the input file into a
    // new buffer. keep count of the number of commands written.
    //
    // note that the newcmds array is zero-filled and is exactly the size of
    // available space. the entire buffer will be copied back into the file,
    // erasing any previous
    unsigned char* newcmds = calloc(1, totalcmdspace);
    unsigned char* p = fb->lcs;
    unsigned char* q = newcmds;
    uint32_t ncmds = 0;
    
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    {
	struct load_command* lc = lcmds->items[icmd];
	bool copy = true;
	
	if (lc->cmd == LC_VERSION_MIN_MACOSX ||
	    lc->cmd == LC_VERSION_MIN_IPHONEOS ||
	    lc->cmd == LC_VERSION_MIN_WATCHOS ||
	    lc->cmd == LC_VERSION_MIN_TVOS ||
	    lc->cmd == LC_BUILD_VERSION)
	{
	    copy = !gOptions.replace;
	    
	    uint32_t platform;
	    if (lc->cmd == LC_BUILD_VERSION) {
		struct build_version_command* bv =
		(struct build_version_command*)lc;
		platform = bv->platform;
	    }
	    else {
		platform = platform_id_for_vmlc(lc->cmd);
	    }
	    
	    for (uint32_t iitem=0; copy && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetBuild &&
		    items[iitem].platform == platform)
		    copy = false;
	    }
	}
	else if (lc->cmd == LC_SOURCE_VERSION)
	{
	    // check if we are modifying this load command. if yes, add the
	    // concrete load command size to our running total.
	    copy = true;
	    
	    for (uint32_t iitem=0; copy && iitem < nitem; ++iitem)
	    {
		if (items[iitem].type == kSetSource)
		    copy = false;
	    }
	}
	
	// copy unswapped load commands from p to q, skipping version cmds.
	if (copy) {
	    memcpy(q, p, lc->cmdsize);
	    q += lc->cmdsize;
	    ncmds += 1;
	}
	
	// advance the unswapped load command pointer
	p += lc->cmdsize;
    } // for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    
    // write the new version commands to the output buffer
    for (uint32_t iitem=0; iitem < nitem; ++iitem)
    {
	if (items[iitem].type == kSetBuild) {
	    if (items[iitem].versmin) {
		struct version_min_command* vmc =
		(struct version_min_command*)q;
		vmc->cmd = platform_vmlc_for_id(items[iitem].platform);
		vmc->cmdsize = sizeof(*vmc);
		vmc->version = items[iitem].version;
		vmc->sdk = items[iitem].sdk_vers;
		if (fb->swap)
		    swap_version_min_command(vmc, gByteOrder);
		q += vmc->cmdsize;
	    }
	    else {
		struct build_version_command* bv =
		(struct build_version_command*)q;
		
		bv->cmd = LC_BUILD_VERSION;
		bv->cmdsize = sizeof(*bv); // for now
		bv->platform = items[iitem].platform;
		bv->minos = items[iitem].version;
		bv->sdk = items[iitem].sdk_vers;
		bv->ntools = 0;
		q += bv->cmdsize;
		
		struct build_tool_version* tools =
		(struct build_tool_version*)q;
		
		// copy the tools for this platform
		for (uint32_t itool=iitem + 1; itool < nitem; ++itool) {
		    if (items[itool].type != kAddTool)
			continue;
		    if (items[itool].platform != bv->platform)
			continue;
		    struct build_tool_version* bt =
		    (struct build_tool_version*)q;
		    bt->tool = items[itool].tool;
		    bt->version = items[itool].version;
		    q += sizeof(*bt);
		    bv->ntools += 1;
		    bv->cmdsize += sizeof(*bt);
		}
		
		if (fb->swap) {
		    swap_build_version_command(bv, gByteOrder);
		    swap_build_tool_version(tools, bv->ntools, gByteOrder);
		}
	    }
	    
	    ncmds += 1;
	}
	if (items[iitem].type == kSetSource) {
	    struct source_version_command* sv =
	    (struct source_version_command*)q;
	    sv->cmd = LC_SOURCE_VERSION;
	    sv->cmdsize = sizeof(*sv);
	    sv->version = items[iitem].src_vers;
	    if (fb->swap)
		swap_source_version_command(sv, gByteOrder);
	    q += sv->cmdsize;
	    
	    ncmds += 1;
	}
    }
    
    // update the mach_header and load commands in memory
    struct mach_header_64 mh;
    memcpy(&mh, &fb->mh, sizeof(mh));
    mh.sizeofcmds = sizeofnewcmds;
    mh.ncmds = ncmds;
    if (fb->swap)
	swap_mach_header_64(&mh, gByteOrder);
    
    q = fb->buf + fb->fat_archs[fb->fat_arch_idx].offset;
    memcpy(q, &mh, fb->mh_size);
    q += fb->mh_size;
    memcpy(q, newcmds, totalcmdspace);
    
    free(newcmds);
    free(items);
    
    return 0;
}

/*
 * command_show() prints the requested version commands in a Mach-O file.
 */
int command_show(struct file* fb)
{
    // print the file name
    const NXArchInfo* archInfo = NULL;
    if (fb->nfat_arch > 1 || gOptions.narch) {
	archInfo = NXGetArchInfoFromCpuType(fb->mh.cputype,
					    fb->mh.cpusubtype);
    }
    printf("%s", gOptions.inPath);
    if (archInfo) {
	printf(" (architecture %s)", archInfo->name);
    }
    printf(":\n");
    
    // walk the load commands ...
    struct lcmds* lcmds = fb->lcmds;
    off_t sectoffset = fb->fat_archs[fb->fat_arch_idx].size;
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    {
	struct load_command* lc = lcmds->items[icmd];
	
	// measure the offset to the first section
	if (lc->cmd == LC_SEGMENT)
	{
	    struct segment_command* sg = (struct segment_command*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section* sc = (struct section*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	else if (lc->cmd == LC_SEGMENT_64)
	{
	    struct segment_command_64* sg = (struct segment_command_64*)lc;
	    for (uint32_t isect = 0; isect < sg->nsects; ++isect)
	    {
		struct section_64* sc = (struct section_64*)
		(((unsigned char*)(sg+1)) + isect * sizeof(*sc));
		if (sc->flags & S_ZEROFILL)
		    continue;
		if (sc->offset < sectoffset)
		    sectoffset = sc->offset;
	    }
	}
	
	// display all build and source version load commands
	else if (lc->cmd == LC_VERSION_MIN_MACOSX ||
	    lc->cmd == LC_VERSION_MIN_IPHONEOS ||
	    lc->cmd == LC_VERSION_MIN_WATCHOS ||
	    lc->cmd == LC_VERSION_MIN_TVOS)
	{
	    if (gOptions.show == kShowAll || gOptions.show == kShowBuild)
	    {
		struct version_min_command* vmc =
		    (struct version_min_command*)lc;
		printf("Load command %u\n", icmd);
		print_version_min_command(vmc);
	    }
	}
	else if (lc->cmd == LC_BUILD_VERSION)
	{
	    if (gOptions.show == kShowAll || gOptions.show == kShowBuild)
	    {
		struct build_version_command* bv =
		    (struct build_version_command*)lc;
		
		printf("Load command %u\n", icmd);
		print_build_version_command(bv);
		
		unsigned char* q = (unsigned char*)(bv+1);
		for (uint32_t itool = 0; itool < bv->ntools; ++itool) {
		    struct build_tool_version* tv =
			(struct build_tool_version*)q;
		    print_build_tool_version(tv->tool, tv->version);
		    q += sizeof(*tv);
		}
	    }
	}
	else if (lc->cmd == LC_SOURCE_VERSION)
	{
	    // verify we are printing build versions
	    if (gOptions.show == kShowAll ||
		gOptions.show == kShowSource)
	    {
		struct source_version_command* sv =
		    (struct source_version_command*)lc;
		printf("Load command %u\n", icmd);
		print_source_version_command(sv);
	    }
	}
    } // for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd)
    
    if (gOptions.show == kShowSpace ) {
        printf("  Mach header size:  %5d\n", (int)fb->mh_size);
        printf("  Load command size: %5d\n", (int)fb->mh.sizeofcmds);
        printf("  Available space:   %5d\n",
               (int)(sectoffset - fb->mh_size - fb->mh.sizeofcmds));
        printf("  Total:             %5d\n", (int)sectoffset);
    }
    
    return 0;
}

/*
 * file_read() reads a Mach-O or fat file into memory and walks enough of its
 * structure to determine which architectures this file is relevant for.
 *
 * Upon success, the following struct file fields will be initialized:
 *
 *   buf
 *   len
 *   mode
 *   nfat_arch
 *   fat_archs
 */
int file_read(const char* path, struct file* fb)
{
    memset(fb, 0, sizeof(*fb));
    
    // open the file for reading
    int fd = open(path, O_RDONLY);
    if (-1 == fd) {
	fprintf(stderr, "%s error: %s: can't open file: %s\n",
		gProgramName, path, strerror(errno));
	return -1;
    }
    
    // stat the file
    struct stat sb;
    if (fstat(fd, &sb)) {
	fprintf(stderr, "%s error: %s: can't stat file: %s\n",
		gProgramName, path, strerror(errno));
	close(fd);
	return -1;
    }
    
    fb->len = sb.st_size;
    fb->mode = sb.st_mode;
    fb->buf = calloc(1, fb->len);
    
    // read the contents of the file into memory
    ssize_t readed = read(fd, fb->buf, fb->len); // that's unpossible!
    if (-1 == readed) {
	fprintf(stderr, "%s error: %s: can't read file: %s\n",
		gProgramName, path, strerror(errno));
	close(fd);
	free(fb->buf);
	return -1;
    }
    else if (readed != fb->len) {
	fprintf(stderr, "%s error: %s: partial read (0x%zx of 0x%llx)\n",
		gProgramName, path, readed, fb->len);
	close(fd);
	free(fb->buf);
	return -1;
    }
    
    // close the file, we're done with it.
    if (close(fd)) {
	fprintf(stderr, "%s warning: %s: can't close file: %s\n",
		gProgramName, path, strerror(errno));
    }
    
    // read the magic
    uint32_t magic;
    
    if (fb->len < sizeof(magic)) {
	fprintf(stderr, "%s error: %s file is not mach-o\n",
		gProgramName, path);
	free(fb->buf);
	return -1;
    }
    
    magic = *(uint32_t*)(fb->buf);
    
    if (magic == MH_MAGIC || magic == MH_CIGAM ||
	magic == MH_MAGIC_64 || magic == MH_CIGAM_64)
    {
	// get the mach_header size, and confirm it fits
	if (magic == MH_MAGIC || magic == MH_CIGAM) {
	    fb->mh_size = sizeof(struct mach_header);
	} else {
	    fb->mh_size = sizeof(struct mach_header_64);
	}
	if (fb->len < fb->mh_size) {
	    fprintf(stderr, "%s error: %s file is not mach-o\n",
		    gProgramName, path);
	    free(fb->buf);
	    return -1;
	}
	
	// read the mach_header and swap it if needed.
	if (magic == MH_MAGIC || magic == MH_CIGAM) {
	    memcpy(&fb->mh, fb->buf, fb->mh_size);
	    fb->mh.reserved = 0;
	}
	else if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
	    memcpy(&fb->mh, fb->buf, fb->mh_size);
	}
	if (magic == MH_CIGAM || magic == MH_CIGAM_64) {
	    fb->swap = true;
	    swap_mach_header_64(&fb->mh, NXHostByteOrder());
	} else {
	    fb->swap = false;
	}
	
	// build a fat_arch table describing this file
	//
	// note that we don't know the alignment for this file, and while we
	// could guess at the alignment from the cputype, as other cctools do,
	// we don't acutally need this value, so we'll leave it blank.
	fb->nfat_arch = 1;
	fb->fat_archs = calloc(1, sizeof(struct fat_arch));
	fb->fat_archs[0].cputype = fb->mh.cputype;
	fb->fat_archs[0].cpusubtype = fb->mh.cpusubtype;
	fb->fat_archs[0].offset = 0;
	fb->fat_archs[0].size = (uint32_t)fb->len;
	fb->fat_archs[0].align = 0;
    }
    else if (magic == FAT_MAGIC || magic == FAT_CIGAM) {
	struct fat_header fh;
	
	// read the fat header
	if (fb->len < sizeof(fh)) {
	    fprintf(stderr, "%s error: %s file is not mach-o\n",
		    gProgramName, path);
	    free(fb->buf);
	    return -1;
	}
	
	memcpy(&fh, fb->buf, sizeof(fh));
	swap_fat_header(&fh, NXHostByteOrder());
	
	// read the initial list of fat archs. deal with arm64ageddon binaries
	// by just reserving 1 additional spot in the fat_arch array, and
	// reading one additional item off of the arch list.
	if (fb->len < sizeof(fh) + sizeof(struct fat_arch) * (fh.nfat_arch+1)) {
	    fprintf(stderr, "%s error: %s file is not mach-o\n",
		    gProgramName, path);
	    free(fb->buf);
	    return -1;
	}
	
	fb->nfat_arch = fh.nfat_arch;
	fb->fat_archs = malloc(sizeof(*fb->fat_archs) * (fb->nfat_arch + 1));
	memcpy(fb->fat_archs, ((struct fat_header*)fb->buf) + 1,
	       sizeof(*fb->fat_archs) * (fb->nfat_arch + 1));
	swap_fat_arch(fb->fat_archs, fb->nfat_arch + 1, NXHostByteOrder());
	
	// look for arm64ageddon binaries
	bool foundARM32 = false;
	bool foundARM64 = false;
	for (uint32_t i = 0; i < fb->nfat_arch; ++i)
	{
	    if (CPU_TYPE_ARM == fb->fat_archs[i].cputype)
		foundARM32 = true;
	    if (CPU_TYPE_ARM64 == fb->fat_archs[i].cputype)
		foundARM64 = true;
	}
	if (foundARM32 && !foundARM64 &&
	    CPU_TYPE_ARM64 == fb->fat_archs[fb->nfat_arch].cputype)
	{
	    fb->nfat_arch += 1;
	}
	
	// verify the fat file contains all its subfiles.
	for (uint32_t i = 0; i < fb->nfat_arch; ++i)
	{
	    if (fb->len < fb->fat_archs[i].offset + fb->fat_archs[i].size) {
		fprintf(stderr, "%s error: %s file #%d for cputype (%u, %u) "
			"extends beyond file boundaries (%llu < %u + %u)\n",
			gProgramName, path, i, fb->fat_archs[i].cputype,
			fb->fat_archs[i].cpusubtype, fb->len,
			fb->fat_archs[i].offset, fb->fat_archs[i].size);
		free(fb->buf);
		free(fb->fat_archs);
		return -1;
	    }
	}
	
    }
    else {
	fprintf(stderr, "%s error: %s file is not mach-o\n",
		gProgramName, path);
	free(fb->buf);
	return -1;
    }
    
    return 0;
}

/*
 * file_select_macho() prepares the Mach-O file for reading by completing
 * initialization of the file struct and verifying the Mach-O header and load
 * command array fit in memory.
 *
 * Upon success, the following struct file fields will be initialized:
 *
 *   fat_arch_idx
 *   mh
 *   mh_size
 *   lcs
 *   lcmds
 *   swap
 */
int file_select_macho(const char* path, struct file* fb, uint32_t index)
{
    if (index >= fb->nfat_arch) {
	fprintf(stderr, "%s internal error: reading beyond fat_arch array\n",
		gProgramName);
	return -1;
    }
    
    fb->fat_arch_idx = index;
    
    // re-verify the magic
    uint32_t offset = fb->fat_archs[index].offset;
    const unsigned char* buf = fb->buf + offset;
    uint32_t magic = *(uint32_t*)(buf);
    
    if (magic == MH_MAGIC || magic == MH_CIGAM ||
	magic == MH_MAGIC_64 || magic == MH_CIGAM_64)
    {
	// get the mach_header size, and confirm it fits
	if (magic == MH_MAGIC || magic == MH_CIGAM) {
	    fb->mh_size = sizeof(struct mach_header);
	} else {
	    fb->mh_size = sizeof(struct mach_header_64);
	}
	if (fb->len < offset + fb->mh_size) {
	    fprintf(stderr, "%s error: %s file #%d for cputype (%u, %u) "
		    "is not mach-o\n",
		    gProgramName, path, index, fb->fat_archs[index].cputype,
		    fb->fat_archs[index].cpusubtype);
	    return -1;
	}
	
	// read the mach_header and swap it if needed.
	if (magic == MH_MAGIC || magic == MH_CIGAM) {
	    memcpy(&fb->mh, buf, fb->mh_size);
	    fb->mh.reserved = 0;
	}
	else if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
	    memcpy(&fb->mh, buf, fb->mh_size);
	}
	if (magic == MH_CIGAM || magic == MH_CIGAM_64) {
	    fb->swap = true;
	    swap_mach_header_64(&fb->mh, NXHostByteOrder());
	} else {
	    fb->swap = false;
	}
	
	// verify the load commands fit in the file
	if (fb->len < offset + fb->mh_size + fb->mh.sizeofcmds) {
	    fprintf(stderr, "%s error: %s file #%d for cputype (%u, %u) "
		    "load command extend beyond length of file\n",
		    gProgramName, path, index, fb->fat_archs[index].cputype,
		    fb->fat_archs[index].cpusubtype);
	    return -1;
	}
	
	// as a convenience, compute the location of the load commands
	fb->lcs = (unsigned char*)&buf[fb->mh_size];
	
	// also cache the load commands in a convenient indexable form.
	if (fb->lcmds)
	    lcmds_free(fb->lcmds);
	fb->lcmds = lcmds_alloc(fb);
	if (!fb->lcmds)
	    return -1;
    }
    else {
	fprintf(stderr, "%s error: %s file #%d for cputype (%u, %u) "
		"is not mach-o\n",
		gProgramName, path, index, fb->fat_archs[index].cputype,
		fb->fat_archs[index].cpusubtype);
	return -1;
    }
    
    return 0;
}

/*
 * file_write writes the entire file buffer to the specified path. the new file
 * is written into a temporary location and then moved into place. permissions
 * on the new file match that of the original file used to initialize fb.
 */
int file_write(const char* path, struct file* fb)
{
    int res = 0;
    bool warn = true;
    bool fake_sign = false;  /* cctools-port */

    // warn if any Mach-O files source file contain code signatures.
    for (uint32_t iarch = 0; 0 == res && warn && iarch < fb->nfat_arch; ++iarch)
    {
	// prepare the mach-o for reading
	res = file_select_macho(path, fb, iarch);
	if (res)
	    continue;
	
	// walk the load commands looking for LC_CODE_SIGNATURE
	struct lcmds* lcmds = fb->lcmds;
	for (uint32_t icmd = 0; warn && icmd < lcmds->count; ++icmd) {
	    struct load_command* lc = lcmds->items[icmd];
	    if (lc->cmd == LC_CODE_SIGNATURE) {
		fprintf(stderr, "%s warning: code signature will be invalid "
			"for %s\n", gProgramName, path);
		warn = false;
		fake_sign = true; /* cctools-port */
	    }
	}
    }
    
    // compute a temporary path to hold our output file during assembly.
    size_t pathlen = strlen(path);
    const char* prefix = ".XXXXXX";
    size_t tmpsize = pathlen + strlen(prefix) + 1;
    char* tmppath = calloc(1, tmpsize);
    snprintf(tmppath, tmpsize, "%s%s", path, prefix);
    
    // open the temp file for writing
    int fd = -1;
    if (0 == res) {
	fd = mkstemp(tmppath);
	if (-1 == fd) {
	    fprintf(stderr, "%s error: ", gProgramName);
	    perror("mkstemp");
	    res = -1;
	}
    }
    
    // write the file
    if (0 == res) {
	ssize_t wrote = write(fd, fb->buf, fb->len);
	if (wrote == -1) {
	    fprintf(stderr, "%s error: %s: write: %s\n", gProgramName, tmppath,
		    strerror(errno));
	    res = -1;
	}
	else if (wrote != fb->len) {
	    fprintf(stderr, "%s error: %s: partial write (0x%zx of 0x%llx)\n",
		    gProgramName, tmppath, wrote, fb->len);
	    res = -1;
	}
    }
    
    // close the file and move the temporary file to its final destination
    if (0 == res && close(fd)) {
	fprintf(stderr, "%s error: %s: can't close file: %s\n",
		gProgramName, tmppath, strerror(errno));
	res = -1;
    }
    
    if (0 == res && chmod(tmppath, fb->mode)) {
	fprintf(stderr, "%s error: %s: can't change file permissions: %s\n",
		gProgramName, tmppath, strerror(errno));
	res = -1;
    }
    
    if (0 == res && rename(tmppath, path)) {
	fprintf(stderr, "%s error: %s: can't rename file: %s\n", gProgramName,
		tmppath, strerror(errno));
	res = -1;
    }

    /* cctools-port */
    if (0 == res && fake_sign)
		FAKE_SIGN_BINARY(path, 1);
    /* cctools-port end */
    
    // try to lean up if something went wrong
    if (res) {
	unlink(tmppath);
    }
    
    free(tmppath);
    
    return res;
}

/*
 * lcmds_alloc reads the load commands from the supplied file, swaps them as
 * necessary, and stores the modified structures in a new list of load
 * commands. In this form, load commands can be looped over multiple times and
 * accessed in random order.
 *
 * BUG: for brevity, only some load command types are fully swapped. All
 * cmd and cmdsize fields will be swapped. Other fields will only be swapped
 * for segment, build version, and source version load commands.
 *
 * TODO: Fully support all load commands.
 */
struct lcmds* lcmds_alloc(struct file* fb)
{
    struct lcmds* lcmds = calloc(1, sizeof(*lcmds));
    if (!lcmds)
	return NULL;
    
    lcmds->count = fb->mh.ncmds;
    lcmds->items = calloc(lcmds->count, sizeof(*lcmds->items));
    if (!lcmds->items) {
	lcmds_free(lcmds);
	return NULL;
    }

    unsigned char* p = fb->lcs;
    for (uint32_t icmd = 0; icmd < fb->mh.ncmds; ++icmd)
    {
	// does the next abstract load command struct entirely fit in the
	// remaining file?
	if (fb->mh.sizeofcmds < p - fb->lcs + sizeof(struct load_command)) {
	    fprintf(stderr, "%s error: load command %d extends beyond "
		    "range\n", gProgramName, icmd);
	    lcmds_free(lcmds);
	    return NULL;
	}
	
	// read the abstract load command
	struct load_command lc;
	memcpy(&lc, p, sizeof(lc));
	if (fb->swap) {
	    swap_load_command(&lc, gByteOrder);
	}
	
	// does the next concrete load command struct entirely fit in the
	// remaining file?
	if (fb->mh.sizeofcmds < p - fb->lcs + lc.cmdsize) {
	    fprintf(stderr, "%s error: load command %d (0x%X) extends "
		    "beyond range\n", gProgramName, icmd, lc.cmd);
	    lcmds_free(lcmds);
	    return NULL;
	}
	
	lcmds->items[icmd] = calloc(1, lc.cmdsize);
	memcpy(lcmds->items[icmd], p, lc.cmdsize);
	
	if (LC_SEGMENT == lc.cmd)
	{
	    // verify the load command fits in the command buffer
	    if (lc.cmdsize < sizeof(struct segment_command)) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }
	    
	    // swap the load command
	    struct segment_command* sg =
		(struct segment_command*)lcmds->items[icmd];
	    if (fb->swap) {
		swap_segment_command(sg, gByteOrder);
	    }
	    
	    // verify the sections also fit in the command buffer
	    if (lc.cmdsize != sizeof(struct segment_command) +
		sizeof(struct section) * sg->nsects) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }

	    // swap the sections
	    struct section* sc = (struct section*)(sg+1);
	    if (fb->swap) {
		swap_section(sc, sg->nsects, gByteOrder);
	    }
	}
	else if (LC_SEGMENT_64 == lc.cmd)
	{
	    // verify the load command fits in the command buffer
	    if (lc.cmdsize < sizeof(struct segment_command_64)) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }

	    struct segment_command_64* sg =
	    (struct segment_command_64*)lcmds->items[icmd];
	    if (fb->swap) {
		swap_segment_command_64(sg, gByteOrder);
	    }
	    
	    // verify the sections also fit in the command buffer
	    if (lc.cmdsize != sizeof(struct segment_command_64) +
		sizeof(struct section_64) * sg->nsects) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }
	    
	    // swap the sections
	    struct section_64* sc = (struct section_64*)(sg + 1);
	    if (fb->swap) {
		swap_section_64(sc, sg->nsects, gByteOrder);
	    }
	}
	else if (LC_VERSION_MIN_TVOS     == lc.cmd ||
		 LC_VERSION_MIN_MACOSX   == lc.cmd ||
		 LC_VERSION_MIN_WATCHOS  == lc.cmd ||
		 LC_VERSION_MIN_IPHONEOS == lc.cmd)
	{
	    // verify the load command fits in the command buffer
	    if (sizeof(struct version_min_command) != lc.cmdsize) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }
	    // swap the load command
	    struct version_min_command* vm =
		(struct version_min_command*)lcmds->items[icmd];
	    if (fb->swap) {
		swap_version_min_command(vm, gByteOrder);
	    }
	}
	else if (LC_BUILD_VERSION == lc.cmd)
	{
	    // verify the load command fits in the command buffer
	    if (lc.cmdsize < sizeof(struct build_version_command)) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }
	    
	    // swap the load command
	    struct build_version_command* bv =
		(struct build_version_command*)lcmds->items[icmd];
	    if (fb->swap) {
		swap_build_version_command(bv, gByteOrder);
	    }
	    
	    // verify the build tools also fit in the command buffer
	    if (lc.cmdsize < sizeof(struct build_version_command) +
		sizeof(struct build_tool_version) * bv->ntools) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }
	    
	    // swap the build tools
	    struct build_tool_version* tv = (struct build_tool_version*)(bv+1);
	    if (fb->swap) {
		swap_build_tool_version(tv, bv->ntools, gByteOrder);
	    }
	}
	else if (LC_SOURCE_VERSION == lc.cmd)
	{
	    // verify the load command fits in the command buffer
	    if (lc.cmdsize != sizeof(struct source_version_command)) {
		fprintf(stderr,
			"%s error: %s file for cputype (%u, %u) load "
			"command %d (0x%x) has incorrect size (%d)\n",
			gProgramName, gOptions.inPath, fb->mh.cputype,
			fb->mh.cpusubtype, icmd, lc.cmd, lc.cmdsize);
		lcmds_free(lcmds);
		return NULL;
	    }

	    // swap the load command
	    struct source_version_command* sv =
		(struct source_version_command*)lcmds->items[icmd];
	    if (fb->swap)
		swap_source_version_command(sv, gByteOrder);
	}
	else {
	    // currently lcmds->items[icmd] is unswapped load command data of
	    // some size. But we were able to successfully read and swap the
	    // abstract load command. Let's just write the swapped abstract
	    // load command into lcmds for now, until the day comes we process
	    // all defined load commands. See BUG: above.
	    memcpy(lcmds->items[icmd], &lc, sizeof(lc));
	}
	
	p += lc.cmdsize;
    } // for (uint32_t icmd = 0; icmd < fb->mh.ncmds; ++icmd)
    return lcmds;
}

/*
 * lcmds_free frees memory consumed by lcmds.
 *
 * lcmds_free can be called on a partially initialied structure.
 */
void lcmds_free(struct lcmds* lcmds)
{
    for (uint32_t icmd = 0; icmd < lcmds->count; ++icmd) {
	if (lcmds->items[icmd])
	    free(lcmds->items[icmd]);
    }
    if (lcmds->items)
	free(lcmds->items);
    free(lcmds);
}

/*
 * parse_version_abcde parses a version number out of a supplied string, and
 * returns it in the supplied version pointer. The version is packed into 64
 * bits as follows: a24.b10.c10.d10.e10. this function will fail if any of the
 * individual components are too large to fit in the available space.
 */
int parse_version_abcde(const char* rostr, uint64_t *version)
{
    uint64_t a, b, c, d, e;
    char* str = strdup(rostr);
    
    // get the major version
    char* start = str;
    char* end = strchr(start, '.');
    if (end) *end = 0;
    a = (uint64_t)strtoull(start, NULL, 10);
    
    // get the minor version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	b = (uint64_t)strtoull(start, NULL, 10);
    } else {
	b = 0;
    }
    
    // get the revision version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	c = (uint64_t)strtoull(start, NULL, 10);
    } else {
	c = 0;
    }
    
    // get the penultimate version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	d = (uint64_t)strtoull(start, NULL, 10);
    } else {
	d = 0;
    }
    
    // get the ultimate version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	e = (uint64_t)strtoull(start, NULL, 10);
    } else {
	e = 0;
    }
    
    free(str);
    
    if (end) {
	fprintf(stderr, "%s error: version has more than 5 components: %s\n",
		gProgramName, rostr);
	return -1;
    }
    
    if (a > 0xFFFFFF) {
	fprintf(stderr, "%s error: major version %llu is too large\n",
		gProgramName, a);
	return -1;
    }
    
    if (b > 0x3FF) {
	fprintf(stderr, "%s error: minor version %llu is too large\n",
		gProgramName, b);
	return -1;
    }
    
    if (c > 0x3FF) {
	fprintf(stderr, "%s error: revision version %llu is too large\n",
		gProgramName, c);
	return -1;
    }
    
    if (d > 0x3FF) {
	fprintf(stderr, "%s error: penultimate version %llu is too large\n",
		gProgramName, d);
	return -1;
    }
    
    if (e > 0x3FF) {
	fprintf(stderr, "%s error: ultimate version %llu is too large\n",
		gProgramName, e);
	return -1;
    }
    
    if (version) {
	*version = (a << 40) | (b << 30) | (c << 20) | (d << 10) | e;
    }
    
    return 0;
}

/*
 * parse_version_xyz parses a version number out of a supplied string, and
 * returns it in the supplied version pointer. The version is packed into 4
 * bytes as follows: XXXX.YY.ZZ. this function will fail if any of the
 * individual components are too large to fit in the available space.
 */
int parse_version_xyz(const char* rostr, uint32_t *version)
{
    uint32_t x, y, z;
    char* str = strdup(rostr);
    
    // get the major version
    char* start = str;
    char* end = strchr(start, '.');
    if (end) *end = 0;
    x = (uint32_t)strtoul(start, NULL, 10);
    
    // get the minor version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	y = (uint32_t)strtoul(start, NULL, 10);
    } else {
	y = 0;
    }
    
    // get the revision version
    if (end) {
	start = end + 1;
	end = strchr(start, '.');
	if (end) *end = 0;
	z = (uint32_t)strtoul(start, NULL, 10);
    } else {
	z = 0;
    }
    
    free(str);
    
    if (end) {
	fprintf(stderr, "%s error: version has more than 3 components: %s\n",
		gProgramName, rostr);
	return -1;
    }
    
    if (x > 0xFFFF) {
	fprintf(stderr, "%s error: major version is too large\n", gProgramName);
	return -1;
    }
    if (y > 0xFF) {
	fprintf(stderr, "%s error: minor version is too large\n", gProgramName);
	return -1;
    }
    if (z > 0xFF) {
	fprintf(stderr, "%s error: revision version is too large\n",
		gProgramName);
	return -1;
    }
    
    if (version) {
	*version = (x << 16) | (y << 8) | z;
    }
    
    return 0;
}

struct platform_entry {
    uint32_t platformid;
    const char* name;
    uint32_t vmlc;
};

static const struct platform_entry kPlatforms[] = {
    { PLATFORM_MACOS,               "macos",        LC_VERSION_MIN_MACOSX },
    { PLATFORM_IOS,                 "ios",          LC_VERSION_MIN_IPHONEOS },
    { PLATFORM_WATCHOS,             "watchos",      LC_VERSION_MIN_WATCHOS },
    { PLATFORM_TVOS,                "tvos",         LC_VERSION_MIN_TVOS },
    { PLATFORM_BRIDGEOS,            "bridgeos",     0 },
    { PLATFORM_MACCATALYST,         "maccatalyst",  0 },
    { PLATFORM_MACCATALYST,         "uikitformac",  0 }, // temporary
    { PLATFORM_IOSSIMULATOR,        "iossim",       0 },
    { PLATFORM_WATCHOSSIMULATOR,    "watchossim",   0 },
    { PLATFORM_DRIVERKIT,           "driverkit",    0 },
};

/*
 * platform_id_for_name returns a platform enum for a given name. returns 0
 * if not found.
 */
uint32_t platform_id_for_name(const char* name)
{
    int count = sizeof(kPlatforms) / sizeof(*kPlatforms);
    for (int i = 0; i < count; ++i)
    {
	if (name && 0 == strcmp(name, kPlatforms[i].name))
	    return kPlatforms[i].platformid;
    }
    return 0;
}

/*
 * platform_id_for_vmlc returns a platform enum for a version min load command.
 * returns 0 if vmlc is not a valid version min load command.
 */
uint32_t platform_id_for_vmlc(uint32_t vmlc)
{
    int count = sizeof(kPlatforms) / sizeof(*kPlatforms);
    for (int i = 0; i < count; ++i)
    {
	if (kPlatforms[i].vmlc == vmlc)
	    return kPlatforms[i].platformid;
    }
    return 0;
}

/*
 * platform_name_for_id returns the name for a given platform id. returns NULL
 * if not found.
 */
const char* platform_name_for_id(uint32_t platform_id)
{
    int count = sizeof(kPlatforms) / sizeof(*kPlatforms);
    for (int i = 0; i < count; ++i)
    {
	if (platform_id == kPlatforms[i].platformid)
	    return kPlatforms[i].name;
    }
    return NULL;
}
							    
/*
 * platform_vmlc_for_id returns a version min load command id for a given
 * platform id. returns 0 if the platform is not found or if that platform does
 * not have an associated version min load command.
 */
uint32_t platform_vmlc_for_id(uint32_t platform_id)
{
    int count = sizeof(kPlatforms) / sizeof(*kPlatforms);
    for (int i = 0; i < count; ++i)
    {
	if (platform_id == kPlatforms[i].platformid)
	    return kPlatforms[i].vmlc;
    }
    return 0;
}
							    
/*
 * Print a formatted version number where components are encoded into a
 * uint32_t in nibbles: X.Y.Z => 0xXXXXYYZZ. If a 'label' is supplied, it will
 * precede the version number and a newline will follow.
 */
void print_version_xyz(const char* label, uint32_t version)
{
    const char* space = " ";
    const char* nl = "\n";
    if (label == NULL)
	label = space = nl = "";
    if((version & 0xff) == 0)
	printf("%s%s%u.%u%s",
	       label,
	       space,
	       version >> 16,
	       (version >> 8) & 0xff,
	       nl);
    else
	printf("%s%s%u.%u.%u%s",
	       label,
	       space,
	       version >> 16,
	       (version >> 8) & 0xff,
	       version & 0xff,
	       nl);
}

/*
 * print a version_min_command.  The version_min_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void print_version_min_command(struct version_min_command *vd)
{
    if(vd->cmd == LC_VERSION_MIN_MACOSX)
	printf("      cmd LC_VERSION_MIN_MACOSX\n");
    else if(vd->cmd == LC_VERSION_MIN_IPHONEOS)
	printf("      cmd LC_VERSION_MIN_IPHONEOS\n");
    else if(vd->cmd == LC_VERSION_MIN_WATCHOS)
	printf("      cmd LC_VERSION_MIN_WATCHOS\n");
    else if(vd->cmd == LC_VERSION_MIN_TVOS)
	printf("      cmd LC_VERSION_MIN_TVOS\n");
    else
	printf("      cmd %u (?)\n", vd->cmd);
    printf("  cmdsize %u", vd->cmdsize);
    if(vd->cmdsize != sizeof(struct version_min_command))
	printf(" Incorrect size\n");
    else
	printf("\n");
    print_version_xyz("  version", vd->version);
    if(vd->sdk == 0)
	printf("      sdk n/a\n");
    else
	print_version_xyz("      sdk", vd->sdk);
}

/*
 * print a build_version_command.  The build_version_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void print_build_version_command(struct build_version_command *bv)
{
    if(bv->cmd == LC_BUILD_VERSION)
	printf("      cmd LC_BUILD_VERSION\n");
    else
	printf("      cmd %u (?)\n", bv->cmd);
    printf("  cmdsize %u", bv->cmdsize);
    if(bv->cmdsize != sizeof(struct build_version_command) +
       bv->ntools * sizeof(struct build_tool_version))
	printf(" Incorrect size\n");
    else
	printf("\n");
    
    printf(" platform ");
    switch(bv->platform){
	case PLATFORM_MACOS:
	    printf("MACOS\n");
	    break;
	case PLATFORM_IOS:
	    printf("IOS\n");
	    break;
	case PLATFORM_TVOS:
	    printf("TVOS\n");
	    break;
	case PLATFORM_WATCHOS:
	    printf("WATCHOS\n");
	    break;
	case PLATFORM_BRIDGEOS:
	    printf("BRIDGEOS\n");
	    break;
	case PLATFORM_MACCATALYST:
	    printf("MACCATALYST\n");
	    break;
	case PLATFORM_IOSSIMULATOR:
	    printf("IOSSIMULATOR\n");
	    break;
	case PLATFORM_TVOSSIMULATOR:
	    printf("TVOSSIMULATOR\n");
	    break;
	case PLATFORM_WATCHOSSIMULATOR:
	    printf("WATCHOSSIMULATOR\n");
	    break;
	case PLATFORM_DRIVERKIT:
	    printf("DRIVERKIT\n");
	    break;
	default:
	    printf("%u\n", bv->platform);
	    break;
    }
    
    print_version_xyz("    minos", bv->minos);
    if(bv->sdk == 0)
	printf("      sdk n/a\n");
    else{
	print_version_xyz("      sdk", bv->sdk);
    }
    printf("   ntools %u\n", bv->ntools);
}

void print_build_tool_version(uint32_t tool, uint32_t version)
{
    printf("     tool ");
    switch(tool){
	case TOOL_CLANG:
	    printf("CLANG\n");
	    break;
	case TOOL_SWIFT:
	    printf("SWIFT\n");
	    break;
	case TOOL_LD:
	    printf("LD\n");
	    break;
	default:
	    printf("%u\n", tool);
	    break;
    }
    
    print_version_xyz("  version", version);
}

/*
 * print a source_version_command.  The source_version_command structure
 * specified must be aligned correctly and in the host byte sex.
 */
void print_source_version_command(struct source_version_command *sv)
{
    uint64_t a, b, c, d, e;
    
    printf("      cmd LC_SOURCE_VERSION\n");
    printf("  cmdsize %u", sv->cmdsize);
    if(sv->cmdsize != sizeof(struct source_version_command))
	printf(" Incorrect size\n");
    else
	printf("\n");
    a = (sv->version >> 40) & 0xffffff;
    b = (sv->version >> 30) & 0x3ff;
    c = (sv->version >> 20) & 0x3ff;
    d = (sv->version >> 10) & 0x3ff;
    e = sv->version & 0x3ff;
    if(e != 0)
	printf("  version %llu.%llu.%llu.%llu.%llu\n", a, b, c, d, e);
    else if(d != 0)
	printf("  version %llu.%llu.%llu.%llu\n", a, b, c, d);
    else if(c != 0)
	printf("  version %llu.%llu.%llu\n", a, b, c);
    else
	printf("  version %llu.%llu\n", a, b);
}

struct tool_entry {
    uint32_t toolid;
    const char* name;
};

static const struct tool_entry kTools[] = {
    { TOOL_CLANG,	"clang" },
    { TOOL_SWIFT,	"swift" },
    { TOOL_LD,		"ld" },
};

/*
 * tool_id_for_name returns a tool enum for a given name. returns 0 if
 * not found.
 */

uint32_t tool_id_for_name(const char* name)
{
    int count = sizeof(kTools) / sizeof(*kTools);
    for (int i = 0; i < count; ++i)
    {
	if (name && 0 == strcmp(name, kTools[i].name))
	    return kTools[i].toolid;
    }
    return 0;
}

/*
 * tool_name_for_id returns the name for a given tool id. returns NULL
 * if not found.
 */
const char* tool_name_for_id(uint32_t tool_id)
{
    int count = sizeof(kTools) / sizeof(*kTools);
    for (int i = 0; i < count; ++i)
    {
	if (tool_id == kTools[i].toolid)
	    return kTools[i].name;
    }
    return NULL;
}

/*
 * usage
 */
void usage(const char * __restrict format, ...)
{
    const char* basename = strrchr(gProgramName, '/');
    if (basename)
	basename++;
    else
	basename = gProgramName;
    
    va_list args;
    va_start(args, format);
    
    if (format) {
	fprintf(stderr, "error: ");
	vfprintf(stderr, format, args);;
	fprintf(stderr, "\n");
    }
    
    va_end(args);
    
    int size = snprintf(NULL, 0, "%s", basename);
    char* spaces = calloc(1, size + 1);
    memset(spaces, ' ', size);

    fprintf(stderr,
"usage: %s [-arch <arch>] ... <show_command> <file>\n", basename);
    fprintf(stderr,
"       %s [-arch <arch>] ... <set_command> ... [-replace] [-output <output>] "
	    "<file>\n", basename);
    fprintf(stderr,
"       %s [-arch <arch>] ... <remove_command> ... [-output <output>] <file>\n",
	    basename);
    fprintf(stderr,
"       %s -help\n", basename);
    fprintf(stderr, "  show_command is exactly one of:\n");
    fprintf(stderr, "    -show\n");
    fprintf(stderr, "    -show-build\n");
    fprintf(stderr, "    -show-source\n");
    fprintf(stderr, "    -show-space\n");
    fprintf(stderr, "  set_command is one or more of:\n");
    fprintf(stderr, "    -set-build-version <platform> <minos> <sdk> [-tool "
	    "<tool> <version>] ...\n");
    fprintf(stderr, "    -set-build-tool <platform> <tool> <version>\n");
    fprintf(stderr, "    -set-version-min <platform> <minos> <sdk>\n");
    fprintf(stderr, "    -set-source-version <version>\n");
    fprintf(stderr, "  remove_command is one or more of:\n");
    fprintf(stderr, "    -remove-build-version <platform>\n");
    fprintf(stderr, "    -remove-build-tool <platform> <tool>\n");
    fprintf(stderr, "    -remove-source-version\n");
    
    if (gOptions.command == kCommandHelp) {
#if 0
	fprintf(stderr, "  arch is one of:\n");
	for (const NXArchInfo* archInfo = NXGetAllArchInfos();
	     archInfo->name;
	     ++archInfo)
	{
	    fprintf(stderr, "    %s\n", archInfo->name);
	}
#endif
	int count = sizeof(kPlatforms) / sizeof(*kPlatforms);
	fprintf(stderr, "  platform is one of:\n");
	for (uint32_t i = 0; i < count; ++i) {
	    fprintf(stderr, "    %s\n", kPlatforms[i].name);
	}

	count = sizeof(kTools) / sizeof(*kTools);
	fprintf(stderr, "  tool is one of:\n");
	for (uint32_t i = 0; i < count; ++i) {
	    fprintf(stderr, "    %s\n", kTools[i].name);
	}

	fprintf(stderr,
		"  platform and tool can also be specified by number\n");
    }
    exit(EXIT_FAILURE);
}
