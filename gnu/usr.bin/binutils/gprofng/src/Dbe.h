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

#ifndef _DBE_H_
#define _DBE_H_

#include <stdio.h>
#include "enums.h"

class MetricList;
template <class ITEM> class Vector;
typedef long long Obj;

Vector<char*> *dbeGetInitMessages (void);
Vector<char*> *dbeGetExpPreview (int dbevindex, char *exp_name);
char *dbeGetExpParams (int dbevindex, char *exp_name);
char *dbeCreateDirectories (const char *dirname);
char *dbeDeleteFile (const char *pathname);
Vector<char*> *dbeReadFile (const char *pathname);
int dbeWriteFile (const char *pathname, const char *contents);
char *dbeGetFileAttributes (const char *filename, const char *format);
char *dbeGetFiles (const char *dirname, const char *format);
char *dbeGetRunningProcesses (const char *format);
char *dbeOpenExperimentList (int dbevindex, Vector<Vector<char*>*> *groups,
			     bool sessionRestart);
char *dbeReadRCFile (int dbevindex, char* path);
char *dbeSetExperimentsGroups (Vector<Vector<char*>*> *groups);
Vector<Vector<char*>*> *dbeGetExperimensGroups ();
char *dbeDropExperiment (int dbevindex, Vector<int> *drop_index);
Vector<char*> *dbeGetExpsProperty (const char *prop_name);
Vector<char*> *dbeGetExpName (int dbevindex);
Vector<int> *dbeGetExpState (int dbevindex);
Vector<bool> *dbeGetExpEnable (int dbevindex);
bool dbeSetExpEnable (int dbevindex, Vector<bool> *enable);
Vector<char*> *dbeGetExpInfo (int dbevindex);
bool dbeGetViewModeEnable ();
bool dbeGetJavaEnable ();
int dbeUpdateNotes (int dbevindex, int exp_id, int type, char* text,
		    bool handle_file);
Vector<void*> *dbeGetTabListInfo (int dbevindex);
Vector<bool> *dbeGetTabSelectionState (int dbevindex);
void dbeSetTabSelectionState (int dbevindex, Vector<bool> *selected);
Vector<bool> *dbeGetMemTabSelectionState (int dbevindex);
void dbeSetMemTabSelectionState (int dbevindex, Vector<bool> *selected);
Vector<bool> *dbeGetIndxTabSelectionState (int dbevindex);
void dbeSetIndxTabSelectionState (int dbevindex, Vector<bool> *selected);
Vector<char*> *dbeGetLoadObjectName (int dbevindex);
Vector<void *> *dbeGetLoadObjectList (int dbevindex);
Vector<char*> *dbeGetSearchPath (int dbevindex);
void dbeSetSearchPath (int dbevindex, Vector<char*> *path);
Vector<void*> *dbeGetPathmaps (int dbevindex);
char *dbeSetPathmaps (Vector<char*> *from, Vector<char*> *to);
char *dbeAddPathmap (int dbevindex, char *from, char *to);
char *dbeGetMsg (int dbevindex, int type);
int dbeInitView (int index, int cloneindex);
void dbeDeleteView (int dbevindex);

//	methods concerning metrics
MetricList *dbeGetMetricListV2 (int dbevindex, MetricType mtype,
				Vector<int> *type, Vector<int> *subtype,
				Vector<bool> *sort, Vector<int> *vis,
				Vector<char*> *aux, Vector<char*> *expr_spec,
				Vector<char*> *legends);
Vector<void*> *dbeGetRefMetricsV2 ();
Vector<void*> *dbeGetCurMetricsV2 (int dbevindex, MetricType mtype);
void dbeSetSort (int dbevindex, int sort_index, MetricType mtype, bool reverse);

//	methods concerning metrics for Overview Tab
Vector<void*> *dbeGetRefMetricTree (int dbevindex, bool include_unregistered);
Vector<void*> *dbeGetRefMetricTreeValues (int dbevindex, Vector<char *> *met_cmds,
					  Vector<char *> *non_met_cmds);
Vector<char*> *dbeGetOverviewText (int dbevindex);
Vector<int> *dbeGetAnoValue (int dbevindex);
void dbeSetAnoValue (int dbevindex, Vector<int> *set);
int dbeGetNameFormat (int dbevindex);
bool dbeGetSoName (int dbevindex);
void dbeSetNameFormat (int dbevindex, int fnames, bool soname);
int dbeGetViewMode (int dbevindex);
void dbeSetViewMode (int dbevindex, int nmode);
Vector<void*> *dbeGetTLValue (int dbevindex);
void dbeSetTLValue (int dbevindex, const char *tldata_cmd,
		    int entitiy_prop_id, int stackalign, int stackdepth);
Vector<void*> *dbeGetExpFounderDescendants ();
Vector<void*> *dbeGetExpSelection (int dbevindex);
Vector<void*> *dbeGetSampleStatus (int dbevindex, int nselected,
				   Vector<bool> *selected);
Vector<unsigned> *dbeGetSampleSize (int dbevindex, Vector<bool> *selected);
char *dbeCheckPattern (int dbevindex, Vector<bool> *selected, char *pattern,
		       int type);
char *dbeSetFilterStr (int dbevindex, char *filter_str);
char *dbeGetFilterStr (int dbevindex);
int dbeValidateFilterExpression (char *str_expr);
Vector<void*> *dbeGetFilterKeywords (int dbevindex);
Vector<void*> *dbeGetFilters (int dbevindex, int nexp);
bool dbeUpdateFilters (int dbevindex, Vector<bool> *selected,
		       Vector<char*> *pattern_str);
char *dbeComposeFilterClause (int dbevindex, int type, int subtype,
			      Vector<int>*selections);
Vector<int> *dbeGetLoadObjectState (int dbevindex);
void dbeSetLoadObjectState (int dbevindex, Vector<int> *selected);
void dbeSetLoadObjectDefaults (int dbevindex);
Vector<void*> *dbeGetMemObjects (int dbevindex);
char *dbeDefineMemObj (char *name, char *index_expr, char *_machmodel,
		       char *sdesc, char *ldesc);
char *dbeDeleteMemObj (char *name);
Vector<char*> *dbeGetCPUVerMachineModel (int dbevindex);
char *dbeLoadMachineModel (char *name);
char *dbeGetMachineModel ();
Vector<char*> *dbeListMachineModels ();
void dbeDetectLoadMachineModel (int dbevindex);
Vector<void*> *dbeGetIndxObjDescriptions (int dbevindex);
Vector<void*> *dbeGetCustomIndxObjects (int dbevindex);
char *dbeDefineIndxObj (char *name, char *index_expr, char *sdesc, char *ldesc);
void dbeSetSelObj (int dbevindex, Obj sel_obj, int type, int subtype);
void dbeSetSelObjV2 (int dbevindex, uint64_t id);
Obj dbeGetSelObj (int dbevindex, int type, int subtype);
uint64_t dbeGetSelObjV2 (int dbevindex, char *typeStr);
int dbeGetSelIndex (int dbevindex, Obj sel_obj, int type, int subtype);
Vector<uint64_t> *dbeGetSelObjsIO (int dbevindex, Vector<uint64_t> *ids, int type);
Vector<uint64_t> *dbeGetSelObjIO (int dbevindex, uint64_t id, int type);
uint64_t dbeGetSelObjHeapTimestamp (int dbevindex, uint64_t id);
int dbeGetSelObjHeapUserExpId (int dbevindex, uint64_t id);
char *dbeSetPrintLimit (int dbevindex, int limit);
int dbeGetPrintLimit (int dbevindex);
char *dbeSetPrintMode (int dbevindex, char *printmode);
int dbeGetPrintMode (int dbevindex);
char *dbeGetPrintModeString (int dbevindex);
char dbeGetPrintDelim (int dbevindex);
Vector<void*> *dbeGetTotals (int dbevindex, int dsptype, int subtype);
Vector<void*> *dbeGetHotMarks (int dbevindex, int type);
Vector<void*> *dbeGetHotMarksInc (int dbevindex, int type);
Vector<void*> *dbeGetSummaryHotMarks (int dbevindex, Vector<Obj> *sel_objs, int type);
Vector<uint64_t> *dbeGetFuncId (int dbevindex, int type, int begin, int length);
Vector<void*> *dbeGetFuncCalleeInfo (int dbevindex, int type, Vector<int>* idxs, int groupId);
Vector<void*> *dbeGetFuncCallerInfo (int dbevindex, int type, Vector<int>* idxs, int groupId);
Vector<void*> *dbeGetFuncCalleeInfoById (int dbevindex, int type, int idx);
Vector<void*> *dbeGetFuncCallerInfoById (int dbevindex, int type, int idx);
char *dbePrintData (int dbevindex, int type, int subtype, char *printer,
		    char *fname, FILE *outfile);
int dbeSetFuncData (int dbevindex, Obj sel_obj, int type, int subtype);
Vector<void*> *dbeGetFuncList (int dbevindex, int type, int subtype);
Vector<void*> *dbeGetFuncListV2 (int dbevindex, int mtype, Obj sel_obj, int type, int subtype);
Vector<void*> *dbeGetFuncListMini (int dbevindex, int type, int subtype);
Vector<Obj> *dbeGetComparableObjsV2 (int dbevindex, Obj sel_obj, int type);
Obj dbeConvertSelObj (Obj obj, int type);
Vector<int> *dbeGetGroupIds (int dbevindex);
Vector<void*> *dbeGetTableDataV2 (int dbevindex, char *mlistStr, char *modeStr,
				  char *typeStr, char *subtypeStr, Vector<uint64_t> *ids);

int dbeGetCallTreeNumLevels (int dbevindex);
Vector<void*> *dbeGetCallTreeLevel (int dbevindex, char *mcmd, int level);
Vector<void*> *dbeGetCallTreeLevels (int dbevindex, char *mcmd);
Vector<void*> *dbeGetCallTreeChildren (int dbevindex, char *mcmd, Vector<int /*NodeIdx*/>*nodes);
Vector<void*> *dbeGetCallTreeLevelFuncs (int dbevindex, int level_start, int level_end);
Vector<void*> *dbeGetCallTreeFuncs (int dbevindex);
Vector<char*> *dbeGetNames (int dbevindex, int type, Obj sel_obj);
Vector<void*> *dbeGetTotalMax (int dbevindex, int type, int subtype);
Vector<void*> *dbeGetStatisOverviewList (int dbevindex);
Vector<void*> *dbeGetStatisList (int dbevindex);
Vector<void*> *dbeGetSummary (int dbevindex, Vector<Obj> *objs, int type, int subtype);
Vector<void*> *dbeGetSummaryV2 (int dbevindex, Vector<Obj> *objs, int type, int subtype);
Vector<int> *dbeGetFounderExpId (Vector<int> *expIds);
Vector<int> *dbeGetUserExpId (Vector<int> *expIds); // filter "user visible" experiment id
Vector<int> *dbeGetExpGroupId (Vector<int> *expIds);
char *dbeGetExpName (int dbevindex, char *dir_name);
Vector<char*> *dbeGetHwcHelp (int dbevindex, bool forKernel);
Vector<Vector<char*>*> *dbeGetHwcSets (int dbevindex, bool forKernel);
Vector<void*> *dbeGetHwcsAll (int dbevindex, bool forKernel);
Vector<char*> *dbeGetHwcAttrList (int dbevindex, bool forKernel);
int dbeGetHwcMaxConcurrent (int dbevindex, bool forKernel);
int dbeGetHwcMaxReg (int dbevindex); // TBR?

Vector<char*> *dbeGetIfreqData (int dbevindex);
Vector<void*> *dbeGetLeakListInfo (int dbevindex, bool leakflag);
Vector<void*> *dbeMpviewGetTlFuncReps (int dbevindex, int exp_id,
				       long long binSizeTime, long long startTime, long long endTime,
				       long long binSizeRank, long long startRank, long long endRank);
Vector<void*> *dbeMpviewGetTlMsgReps (int dbevindex, int exp_id, int throttle,
				      long long binSizeTime, long long startTime, long long endTime,
				      long long binSizeRank, long long startRank, long long endRank);
Vector<long long> *dbeMpviewGetAxisRange (int dbevindex, int exp_id,
					  int chart_type, int axis_type);
Vector<char*> *dbeMpviewGetAxisDiscreteLabels (int dbevindex, int exp_id,
					       int chart_type, int axis_type);
Vector<void*> *dbeMpviewGetFuncDetails (int dbevindex, int exp_id, Obj funcId);
Vector<void*> *dbeMpviewGetMesgDetails (int dbevindex, int exp_id, Obj mesgId);
Vector<long long> *dbeMpviewGetChartData (int dbevindex, int exp_id, int ctype,
					  int attr1, long long start1,
					  long long end1, int nbins1,
					  int attr2, long long start2,
					  long long end2, int nbins2,
					  int metric, int reduction);
void dbeMpviewFilterSet (int dbevindex, int exp_id, Vector<int> *ctid,
			 Vector<int > *axid, Vector<long long> *startVal,
			 Vector<long long> *endVal);
void dbeMpviewLoadStacks (int dbevindex, int exp_id);


Obj dbeGetObject (int dbevindex, Obj sel_func, Obj sel_pc);
char *dbeGetName (int dbevindex, int exp_id);
Vector<char*> *dbeGetExpVerboseName (Vector<int> *exp_ids);
long long dbeGetStartTime (int dbevindex, int exp_id);
long long dbeGetRelativeStartTime (int dbevindex, int exp_id);
long long dbeGetEndTime (int dbevindex, int exp_id);
int dbeGetClock (int dbevindex, int exp_id);
long long dbeGetWallStartSec (int dbevindex, int exp_id);
char *dbeGetHostname (int dbevindex, int exp_id);
Vector<void*> *dbeGetEntityProps (int dbevindex);
Vector<void*> *dbeGetEntities (int dbevindex, int exp_id, int ekind);
Vector<void*> *dbeGetEntitiesV2 (int dbevindex, Vector<int> *exp_ids, int ekind);
Vector<void*> *dbeGetTLDetails (int dbevindex, int exp_id, int data_id,
				int entity_prop_id, Obj event_id);
Vector<Obj> *dbeGetStackFunctions (int dbevindex, Obj stack);
Vector<void*> *dbeGetStacksFunctions (int dbevindex, Vector<Obj> *stacks);
Vector<Obj> *dbeGetStackPCs (int dbevindex, Obj stack);
Vector<char*> *dbeGetStackNames (int dbevindex, Obj stack);
Vector<void*> *dbeGetSamples (int dbevindex, int exp_id, int64_t lo, int64_t hi);
Vector<void*> *dbeGetGCEvents (int dbevindex, int exp_id, int64_t lo, int64_t hi);
Vector<Vector<char*>*>* dbeGetIOStatistics (int dbevindex);
Vector<Vector<char*>*>* dbeGetHeapStatistics (int dbevindex);
Vector<char*> *dbeGetFuncNames (int dbevindex, Vector<Obj> *funcs);
Vector<char*> *dbeGetObjNamesV2 (int dbevindex, Vector<uint64_t> *ids);
char *dbeGetFuncName (int dbevindex, Obj func);
char *dbeGetObjNameV2 (int dbevindex, uint64_t id);
Vector<uint64_t> *dbeGetFuncIds (int dbevindex, Vector<Obj> *funcs);
uint64_t dbeGetFuncId (int dbevindex, Obj func);
char *dbeGetDataspaceTypeDesc (int dbevindex, Obj stack);
Vector<void*> *dbeGetDataDescriptorsV2 (int exp_id);
Vector<void*> *dbeGetDataPropertiesV2 (int exp_id, int data_id);
Vector<void*> *dbeGetExperimentTimeInfo (Vector<int> *exp_ids);
Vector<void*> *dbeGetExperimentDataDescriptors (Vector<int> *exp_ids);

/*   New Timeline Interface */
Vector<long long> *dbeGetAggregatedValue (int data_id, char *lfilter, char *fexpr,
					  char *pname_ts, hrtime_t start_ts,
					  hrtime_t delta, int num,
					  char *pname_key, char *aggr_func);
Vector<char*> *dbeGetLineInfo (Obj pc);
int dbeSetAlias (char *name, char *uname, char *expr);
Vector<char*> *dbeGetAlias (char *name);
Vector<Vector<long long>*> *dbeGetXYPlotData (int data_id, char *lfilter,
					      char *arg, char *func1, char *aggr1,
					      char *func2, char *aggr2,
					      char *func3, char *aggr3);
Vector<bool> *dbeHasTLData (int dbev_index, Vector<int> *exp_ids,
			    Vector<int> *data_ids, // DATA_*
			    Vector<int> *entity_prop_ids, // LWP,CPU,THR, etc
			    Vector<int> *entity_prop_values,
			    Vector<int> *auxs);
Vector<void*> *dbeGetTLData (int dbevindex, int exp_id, int data_id,
			     int entity_prop_id, int entity_prop_val, int aux,
			     hrtime_t start_ts, hrtime_t delta, int num,
			     bool getRepresentatives, Vector<char*> *chartProperties);
Vector<long long> *dbeGetTLEventCenterTime (int dbevindex, int exp_id,
					    int data_id, int entity_prop_id,
					    int entity_prop_val, int aux,
					    long long event_idx, long long move_count);
long long dbeGetTLEventIdxNearTime (int dbevindex, int exp_id,
				    int data_id,
				    int entity_prop_id, int entity_prop_val, int aux,
				    int searchDirection,
				    long long timestamp);

/*   Interface for use by Collector GUI */
int dbeGetSignalValue (char *);
char *dbeSendSignal (pid_t, int);
char *dbeGetCollectorControlValue (char *);
char *dbeSetCollectorControlValue (char *, char *);
char *dbeUnsetCollectorControlValue (char *);
char *dbeCheckConnection (char *);
void dbe_archive (Vector<long long> *ids, Vector<const char *> *locations);
void dbeSetLocation (const char *fname, const char *location);
void dbeSetLocations (Vector<const char *> *fnames, Vector<const char *> *locations);
Vector<void*> *dbeResolvedWith_setpath (const char *path);
Vector<void*> *dbeResolvedWith_pathmap (const char *old_prefix, const char *new_prefix);

#endif /* _DBE_H_ */
