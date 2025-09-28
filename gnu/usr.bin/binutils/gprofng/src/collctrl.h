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

/* This file describes the data structures used to control
 * data collection; it is used by various commands in the MPMT
 * tree, and is also shared with dbx.  Care should be taken
 * to ensure that both the mpmt and dbx builds continue.

 * To remove any APIs or change any enum cases:
 *
 * 1.  Make the changes in mpmt, and preserve the old APIs
 *	as scaffolding and any old enum values as #defines.
 *
 * 2.  Add the new APIs and new cases to dbx, remove the
 *	old ones.
 *
 * 3.  Remove the old API scaffolding and enum values here
 *
 */

#ifndef	_COLLCTRL_H
#define	_COLLCTRL_H

#include "hwcentry.h"
#include "cc_libcollector.h"

/*---------------------------------------------------------------------------*/

/* class */

typedef struct {
    int min;
    int res;
    int max;
    int hival;
    int normval;
    int lowval;
} clk_params_t;

#define PROFINT_HIGH 997
#define PROFINT_NORM 10007
#define PROFINT_LOW 100003

#define PROFINT_MIN 500
#define PROFINT_MAX 1000000

class Coll_Ctrl {
public:

  /* _interactive is 1 for dbx, 0 for collect */
  Coll_Ctrl(int _interactive = 0, bool _defHWC = false, bool _kernelHWC = false);
  ~Coll_Ctrl();

  Coll_Ctrl(Coll_Ctrl *cc);       /* constructor for duplicate */
  char *check_expt(char **);      /* check the experiment directory */
  char *setup_experiment();       /* set up the experiment directory, etc. */
  void close_expt();
  void interrupt();               /* the user interrupts experiment */
  void delete_expt();

  /* enable/disable the experiment */
  char *enable_expt();
  void disable_expt()     { enabled = 0; };
  int isenabled()         { return enabled; };

  /* check for active experiment */
  int isopened()          { return opened; };

  /* set the parameters for clock-profiling */
  void set_clk_params(int min, int res, int max, int hi, int norm, int lo);
  char *set_clkprof(const char *valptr, char **warn);
  char *reset_clkprof(int val); /* called if profiler must reset value */
  int get_sys_period()    { return clk_params.min; };
  int get_clk_min()       { return clk_params.min; };
  int get_clk_max()       { return clk_params.max; };
  int get_clk_res()       { return clk_params.res; };
  int get_clkprof_mode()  { return clkprof_enabled; };
  int get_clkprof_timer() { return clkprof_timer; };

  /* set the parameters for synchronization tracing */
  char *set_synctrace(const char *valptr);
  int get_synctrace_mode()    { return synctrace_enabled; };
  int get_synctrace_thresh()  { return synctrace_thresh; };
  int get_synctrace_scope()   { return synctrace_scope; };

  /* set the parameters for heap tracing */
  char *set_heaptrace(const char *);
  int get_heaptrace_mode()    { return heaptrace_enabled; };
  int get_heaptrace_checkmode() { return heaptrace_checkenabled; };

  /* set the parameters for I/O tracing */
  char *set_iotrace(const char *);
  int get_iotrace_mode()      { return iotrace_enabled; };

  /* set the parameters for HW counting */
  void setup_hwc();
  char *set_hwcstring(const char *str, char **warn);
  char *add_hwcstring(const char *str, char **warn);
  char *add_default_hwcstring(const char *str, char **warn, bool add, bool forKernel = false);
  void set_hwcdefault();
  void disable_hwc();
  int get_hwc_cnt()       { return hwcprof_enabled_cnt; };
  int get_hwc_mode()      { return hwcprof_enabled_cnt ? 1 : 0; };
  char *get_hwc_string()  { return hwc_string; };

  Hwcentry *
  get_hwc_entry (int n)
  {
    if (n < 0 || n >= hwcprof_enabled_cnt)
      return 0;
    return &hwctr[n];
  };

  void hwcentry_dup (Hwcentry *, Hwcentry *);
  char *get_hwc_counter (int n) { return get_hwc_entry (n)->name; };

  /* set the parameters for count data */
  char *set_count (const char *);
  int get_count ()              { return count_enabled; };
  void set_Iflag ()             { Iflag = 1; };
  void set_Nflag ()             { Nflag = 1; };

  /* set time interval for attach with dbx timed collection */
  /* also used for er_kernel */
  char *set_time_run (const char *);
  int get_time_run (void)       { return time_run; };
  int get_start_delay (void)    { return start_delay; };

  /* set pid for attach with dbx to collect data */
  char *set_attach_pid (char *);
  int get_attach_pid (void)     { return attach_pid; };

  /* set java mode, "on" = yes; "off" = no; anthing else implies
   *	yes, and is the path to the java to use
   *	java_mode is returned as zero for off, one for on
   *	java_default is returned as zero for explicitly set, one for defaulted on
   */
  char *set_java_mode (const char *);
  int get_java_mode ()          { return java_mode; };
  int get_java_default ()       { return java_default; };

  /* setting Java path explicitly */
  char *set_java_path (const char *);
  char *get_java_path ()        { return java_path; };

  /* set additional arguments for Java invocation */
  char *set_java_args (char *);
  char *get_java_args ()        { return java_args; };
  int get_java_arg_cnt ()       { return njava_args; };

  /* set load-object archive mode, 0 = no; other = yes */
  char *set_archive_mode (const char *);
  char *get_archive_mode ()     { return archive_mode; };

  /* set follow-descendants mode, 0 = no; other = yes */
  char *set_follow_mode (const char *);
  Follow_type get_follow_mode () { return follow_mode; };
  int get_follow_default ()     { return follow_default; };
  char *get_follow_usr_spec ()  { return follow_spec_usr; };
  char *get_follow_cmp_spec ()  { return follow_spec_cmp; };

  /* set profile idle cpus mode, 1 = no; 0 = yes */
  char *set_prof_idle (const char *);
  int get_prof_idle ()          { return prof_idle; };

  /* set debug more, 1 = yes; other = no */
  /* if set, target will be set to halt at exit from exec */
  char *set_debug_mode (int);
  int get_debug_mode ()         { return debug_mode; };

  /* find a signal from a string */
  int find_sig (const char *);
  /* find a signal name from a signal value */
  char *find_signal_name (int signal);

  /* set the pauseresume (delayed initialization) signal */
  char *set_pauseresume_signal (int, int);
  int get_pauseresume_signal () { return pauseresume_sig; };
  int get_pauseresume_pause ()  { return pauseresume_pause; };

  /* set the sample signal */
  char *set_sample_signal (int);
  int get_sample_signal ()      { return sample_sig; };

  /* set the periodic sampling */
  char *set_sample_period (const char *);
  int get_sample_period (void)  { return sample_period; };

  /* set experiment size limit */
  char *set_size_limit (const char *);
  int get_size_limit (void)     { return size_limit; };

  /* naming methods */
  /* set the target executable name */
  int set_target (char *);
  char *get_target ()           { return target_name; };

  /* set the experiment name */
  void set_default_stem (const char *);
  char *set_expt (const char *, char **, bool);
  char *get_expt ()             { return expt_name; };

  /* set the experiment directory */
  char *set_directory (char *, char **);

  char *get_directory ()        { return udir_name ? udir_name : store_dir; };

  /* return the real experiment ptr file name */
  char *get_experiment ()       { return store_ptr; };
  char *update_expt_name (bool verbose = true, bool ckonly = false, bool newname = false);

  /* remove the experiment */
  void remove_exp_dir ();

  /* return the data descriptor */
  char *
  get_data_desc ()
  {
    return data_desc;
  };

  /* set the experiment group */
  char *set_group (char *);
  char *get_group ()            { return expt_group; };

  /* return the experiment settings as a string */
  char *show (int); /* full show */
  char *show_expt (); /* short form */

  /* return an argv array to compose a "collect" command from settings */
  char **get_collect_args ();

  /* determine characteristics of system */
  char *get_node_name ()        { return node_name; };
  long get_ncpus ()             { return ncpus; };
  int get_cpu_clk_freq ()       { return cpu_clk_freq; };
  int get_cpc_cpuver ()         { return cpc_cpuver; };

    /* disable warning about non-local filesystems */
  void set_nofswarn ()          { nofswarn = 1; };

  //========== Special functions to communicate with the Collector GUI ==========//
  char *get (char *);   /* get control's value */
  char *set (char *, const char *); /* set control's value */
  char *unset (char *); /* reset control's value to its default */
  void set_project_home (char *);

private:
  int interactive;      /* 1 - dbx, 0 - collect */
  bool defHWC;          /* true if default HWC experiment should be run */
  bool kernelHWC;       /* T if default HWC counters are for kernel profiling */
  int opened;           /* T if an experiment is opened */
  int enabled;          /* T if an experiment is enabled */
  volatile int uinterrupt; /* set if interrupt from user */

  /* experiment/machine characteristics */
  char *node_name;      /* name of machine on which experiment is run */
  long ncpus;           /* number of online CPUs */
  int cpu_clk_freq;     /* chip clock (MHz.), as reported from processor_info */
  int cpc_cpuver;       /* chip version, as reported from libcpc */
  long sys_resolution;  /* system clock resolution */
  int sys_period;       /* profiling clock resolution on the system */
  int sample_period;    /* period for sampling, seconds */
  int sample_default;    /* if period for sampling set by default */
  int size_limit;       /* experiment size limit, MB */
  long npages;          /* number of pages configured */
  long page_size;       /* size of system page */
  clk_params_t clk_params;

  /* user specification of name */
  /*  user may specify both uexpt_name and udir_name
   *	if uexpt_name is absolute path, udir_name is ignored, with warning
   *	otherwise, udir_name is prepended to uexpt_name
   *
   *	if uexpt_name is of the form: <dir>/zzzz.nnn.er, where nnn is numeric,
   *	nnn will be reset to one greater than the the highest experiment
   *	  with a name of the same form.
   */
  char *default_stem;   /* default stem for experiment name */
  char *uexpt_name;     /* suggested experiment name */
  char *expt_name;      /* experiment name, after defaulting */
  char *expt_dir;       /* directory part of suggested experiment name */
  char *base_name;      /* basename of suggested experiment name */
  char *udir_name;      /* user name of directory for data */

  char *store_dir;      /* directory to contain experiment dir. */
  char *prev_store_dir; /* previously set store directory */
  char *store_ptr;      /* experiment pointer file */
  char *expt_group;     /* name of experiment group, if any */
  char *project_home;   /* argv[0] */

  char *target_name;    /* target executable name */
  char *data_desc;      /* string describing the data to be collected */
  char *lockname;       /* name of directory lock file */
  int lockfd;           /* fd of open lock file */

  int nofswarn;         /* if 1, don't warn of filesystem */
  int expno;            /* number in <stem>.<expno>.er */

  /* T if an target is to be left for debugger attach */
  int debug_mode;

  /* clock-profiling controls */
  /* T if clock-based profiling */
  int clkprof_enabled;

  /* T if on by default, rather than explicit setting */
  int clkprof_default;

  /* value for timer, microseconds. */
  int clkprof_timer; // adjusted clock profiling interval
  int clkprof_timer_target; // desired clock profiling interval

  /* HW counter-profiling controls */
  /* >0 if HW counter-based profiling */
  int hwcprof_default;
  int hwcprof_enabled_cnt;
  char *hwc_string;
  Hwcentry hwctr[MAX_PICS];

  int synctrace_enabled;    /* T if synchronization tracing */
  /*  sync trace threshold value, microsec. */
  /* if 0, record all */
  /* if <0, calibrate */
  int synctrace_thresh;

  /* sync trace scope -- a bit mask */
  /* 	definitions in data_pckts.h */
  int synctrace_scope;

  int heaptrace_enabled;    /* T if heap tracing */
  /* if 0 no checking;
   * if 1, check for over- and under-write
   * if 2, also set patterns in malloc'd and free'd areas
   */
  int heaptrace_checkenabled;
  int iotrace_enabled;  /* T if I/O tracing */

  /* count controls */
  /* 1 if counting is enabled; -1 if count static is enabled */
  int count_enabled;
  int Iflag;    /* specify bit output directory -- only with count_enabled */
  int Nflag;    /* specify bit library to ignore -- only with count_enabled */

  /* pid, if -P <pid> is invoked for attach with dbx */
  int attach_pid;

  /* time_run -- non-zero if timed execution request (dbx/er_kernel) */
  int time_run;
  int start_delay;

  /* T if Java profiling is requested */
  int java_mode;
  int java_default;
  char *java_path;
  char *java_args;
  int njava_args;

  /* whether/how following-descendants is requested */
  Follow_type follow_mode;
  int follow_default;
  char *follow_spec_usr;    // user's selective follow spec
  char *follow_spec_cmp;    // compiled selective follow spec
  int prof_idle;            // whether profiling idle cpus is requested
  char *archive_mode;       // how load-objects archiving is requested

  // signals to control pause-resume (delayed initialization) and samples
  int pauseresume_sig;      // for pause-resume signal -- delayed initialization
  int pauseresume_pause;    // 1 if pauseresume and start paused; 0 if not
  int sample_sig;           // to trigger sample
  char *report_signal_conflict (int);
  char *check_consistency ();   // check the experiment settings for consistency
  void determine_profile_params ();
  char *preprocess_names ();
  char *get_exp_name (const char *);
  char *create_exp_dir ();
  void build_data_desc ();
  char *check_group ();
  char *join_group ();
  void free_hwc_fields (Hwcentry *tmpctr);

  // propagates clkprof_timer change to Hwcentry hwctr[]
  void set_clkprof_timer_target (int microseconds);
  void adjust_clkprof_timer (int microseconds);
  hrtime_t clkprof_timer_2_hwcentry_min_time (int clkprof_microseconds);
};

#endif /* !_COLLCTRL_H */
