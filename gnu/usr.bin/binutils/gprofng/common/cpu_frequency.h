/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef	_CPU_FREQUENCY_H
#define	_CPU_FREQUENCY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <alloca.h>
#include <unistd.h> /* processor_info_t	*/
#include <fcntl.h>

  typedef unsigned char uint8_t;

#define MAXSTRLEN               1024
  /*
   * This file provide the api to detect Intel CPU frequency variation features
   */

#define COL_CPUFREQ_NONE        0x0000
#define COL_CPUFREQ_SCALING     0x0001
#define COL_CPUFREQ_TURBO       0x0002

#if defined(__i386__) || defined(__x86_64)
  // XXXX This is a rough table to estimate frequency increment due to intel turbo boost.
  // CPU with different stepping and different core number have different turbo increment.
  //  It is used internally here, and is not implemented on SPARC

  // YLM: one can use cputrack to estimate max turbo frequency
  // example: for a cpu-bound app that runs for > 10 seconds, count cycles for 10 seconds:
  //     cputrack -T 10 -v -c cpu_clk_unhalted.thread_p a.out

  static int
  get_max_turbo_freq (int model)
  {
    switch (model)
      {
	// Nehalem
      case 30:// Core i7-870: 2/2/4/5
	return 2 * 133333;
      case 26:// Xeon L5520: 1/1/1/2
	return 2 * 133333;
      case 46:// Xeon E7540: 2
	return 2 * 133333;
	// Westmere
      case 37:// Core i5-520M: 2/4
	return 2 * 133333;
      case 44:// Xeon E5620: 1/1/2/2
	return 2 * 133333;
      case 47:// Xeon E7-2820: 1/1/1/2
	return 1 * 133333;
	// Sandy Bridge
      case 42:// Core i5-2500: 1/2/3/4
	return 3 * 100000;
	// http://ark.intel.com/products/64584/Intel-Xeon-Processor-E5-2660-20M-Cache-2_20-GHz-8_00-GTs-Intel-QPI
      case 45:// Xeon E5-2660 GenuineIntel 206D7 family 6 model 45 step 7 clock 2200 MHz
	return 8 * 100000;
	// Ivy Bridge
      case 58:// Core i7-3770: 3/4/5/5
	return 4 * 100000;
      case 62:// Xeon E5-2697: 3/3/3/3/3/3/3/4/5/6/7/8
	return 7 * 100000;
	// Haswell
      case 60:
	return 789000; // empirically we see 3189 MHz - 2400 MHz
      case 63:
	return 1280000; // empirically we see 3580 MHz - 2300 MHz for single-threaded
	//  return  500000;   // empirically we see 2800 MHz - 2300 MHz for large throughput
	// Broadwell
	// where are these values listed?
	// maybe try https://en.wikipedia.org/wiki/Broadwell_%28microarchitecture%29#Server_processors
      case 61:
	return 400000;
      case 71:
	return 400000;
      case 79:
	return 950000; // empirically we see (3550-2600) MHz for single-threaded on x6-2a
      case 85:
	return 1600000; // X7: empirically see ~3.7GHz with single thread, baseline is 2.1Ghz  Return 3,700,000-2,100,000
      case 31: // Nehalem?
      case 28: // Atom
      case 69: // Haswell
      case 70: // Haswell
      case 78: // Skylake
      case 94: // Skylake
      default:
	return 0;
      }
  }
#endif

  /*
   * parameter: mode, pointer to a 8bit mode indicator
   * return: max cpu frequency in MHz
   */
  //YXXX Updating this function?  Check similar cut/paste code in:
  // collctrl.cc::Coll_Ctrl()
  // collector.c::log_header_write()
  // cpu_frequency.h::get_cpu_frequency()

  static int
  get_cpu_frequency (uint8_t *mode)
  {
    int ret_freq = 0;
    if (mode != NULL)
      *mode = COL_CPUFREQ_NONE;
    FILE *procf = fopen ("/proc/cpuinfo", "r");
    if (procf != NULL)
      {
	char temp[1024];
	int cpu = -1;
#if defined(__i386__) || defined(__x86_64)
	int model = -1;
	int family = -1;
#endif
	while (fgets (temp, 1024, procf) != NULL)
	  {
	    if (strncmp (temp, "processor", strlen ("processor")) == 0)
	      {
		char *val = strchr (temp, ':');
		cpu = val ? atoi (val + 1) : -1;
	      }
#if defined(__i386__) || defined(__x86_64)
	    else if (strncmp (temp, "model", strlen ("model")) == 0
		     && strstr (temp, "name") == 0)
	      {
		char *val = strchr (temp, ':');
		model = val ? atoi (val + 1) : -1;
	      }
	    else if (strncmp (temp, "cpu family", strlen ("cpu family")) == 0)
	      {
		char *val = strchr (temp, ':');
		family = val ? atoi (val + 1) : -1;
	      }
#endif
	    else if (strncmp (temp, "cpu MHz", strlen ("cpu MHz")) == 0)
	      {
		char *val = strchr (temp, ':');
		int mhz = val ? atoi (val + 1) : 0; /* reading it as int is fine */
		char scaling_freq_file[MAXSTRLEN + 1];
		snprintf (scaling_freq_file, sizeof (scaling_freq_file),
			  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_driver", cpu);
		int intel_pstate = 0;
		int no_turbo = 0;
		if (access (scaling_freq_file, R_OK) == 0)
		  {
		    FILE *cpufreqd = fopen (scaling_freq_file, "r");
		    if (cpufreqd != NULL)
		      {
			if (fgets (temp, 1024, cpufreqd) != NULL
			    && strncmp (temp, "intel_pstate", sizeof ("intel_pstate") - 1) == 0)
			  intel_pstate = 1;
			fclose (cpufreqd);
		      }
		  }
		snprintf (scaling_freq_file, sizeof (scaling_freq_file),
			  "/sys/devices/system/cpu/intel_pstate/no_turbo");
		if (access (scaling_freq_file, R_OK) == 0)
		  {
		    FILE *pstatent = fopen (scaling_freq_file, "r");
		    if (pstatent != NULL)
		      {
			if (fgets (temp, 1024, pstatent) != NULL)
			  if (strncmp (temp, "1", sizeof ("1") - 1) == 0)
			    no_turbo = 1;
			fclose (pstatent);
		      }
		  }

		snprintf (scaling_freq_file, sizeof (scaling_freq_file),
			  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu);
		int frequency_scaling = 0;
		int turbo_mode = 0;
		if (access (scaling_freq_file, R_OK) == 0)
		  {
		    FILE *cpufreqf = fopen (scaling_freq_file, "r");
		    if (cpufreqf != NULL)
		      {
			if (fgets (temp, 1024, cpufreqf) != NULL)
			  {
			    int ondemand = 0;
			    if (strncmp (temp, "ondemand", sizeof ("ondemand") - 1) == 0)
			      ondemand = 1;
			    int performance = 0;
			    if (strncmp (temp, "performance", sizeof ("performance") - 1) == 0)
			      performance = 1;
			    int powersave = 0;
			    if (strncmp (temp, "powersave", sizeof ("powersave") - 1) == 0)
			      powersave = 1;
			    if (intel_pstate || ondemand || performance)
			      {
				snprintf (scaling_freq_file, sizeof (scaling_freq_file),
					  "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpu);
				if (access (scaling_freq_file, R_OK) == 0)
				  {
				    FILE * cpufreqf_max;
				    if ((cpufreqf_max = fopen (scaling_freq_file, "r")) != NULL)
				      {
					if (fgets (temp, 1024, cpufreqf_max) != NULL)
					  {
					    int tmpmhz = atoi (temp);
					    snprintf (scaling_freq_file, sizeof (scaling_freq_file),
						      "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cpu);
					    if (intel_pstate)
					      {
						frequency_scaling = 1;
						turbo_mode = !no_turbo;
						if (powersave)
						  // the system might have been relatively cold
						  // so we might do better with scaling_max_freq
						  mhz = (int) (((double) tmpmhz / 1000.0) + 0.5);
					      }
					    else if (access (scaling_freq_file, R_OK) == 0)
					      {
						FILE * cpufreqf_ava;
						if ((cpufreqf_ava = fopen (scaling_freq_file, "r")) != NULL)
						  {
						    if (fgets (temp, 1024, cpufreqf_ava) != NULL)
						      {
							if (strchr (temp, ' ') != strrchr (temp, ' ') && ondemand)
							  frequency_scaling = 1;
							if (tmpmhz > 1000)
							  {
#if defined(__i386__) || defined(__x86_64)
							    if (family == 6)
							      {
							        // test turbo mode
							        char non_turbo_max_freq[1024];
							        snprintf (non_turbo_max_freq, sizeof (non_turbo_max_freq),
							                  "%d", tmpmhz - 1000);
							        if (strstr (temp, non_turbo_max_freq))
							          {
							            turbo_mode = 1;
							            tmpmhz = (tmpmhz - 1000) + get_max_turbo_freq (model);
							          }
							      }
#endif
							  }
						      }
						    fclose (cpufreqf_ava);
						  }
						mhz = (int) (((double) tmpmhz / 1000.0) + 0.5);
					      }
					  }
					fclose (cpufreqf_max);
				      }
				  }
			      }
			  }
			fclose (cpufreqf);
		      }
		  }
		if (mhz > ret_freq)
		  ret_freq = mhz;
		if (frequency_scaling && mode != NULL)
		  *mode |= COL_CPUFREQ_SCALING;
		if (turbo_mode && mode != NULL)
		  *mode |= COL_CPUFREQ_TURBO;
	      }
	    else if (strncmp (temp, "Cpu", 3) == 0 && temp[3] != '\0' &&
		     strncmp (strchr (temp + 1, 'C') ? strchr (temp + 1, 'C') : (temp + 4), "ClkTck", 6) == 0)
	      { // sparc-Linux
		char *val = strchr (temp, ':');
		if (val)
		  {
		    unsigned long long freq;
		    sscanf (val + 2, "%llx", &freq);
		    int mhz = (unsigned int) (((double) freq) / 1000000.0 + 0.5);
		    if (mhz > ret_freq)
		      ret_freq = mhz;
		  }
	      }
	  }
	fclose (procf);
      }
    return ret_freq;
  }

#ifdef __cplusplus
}
#endif

#endif  /*_CPU_FREQUENCY_H*/
