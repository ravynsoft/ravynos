#
# Local Apple addition for region resources
# Copyright (c) 2012-2018 Apple Inc. All rights reserved.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# NOTE (January 2019): Please use ICU's new data filtering to select locale
# files.  This makefile is no longer used to filter locale files.
# With no filtering, all data files are built.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

REGION_SOURCE_LOCAL = iu.txt ms_Arab.txt\
	en_MV.txt\
	pt_FR.txt\
	arn.txt ba.txt byn.txt co.txt cv.txt\
	dv.txt gaa.txt gez.txt gn.txt io.txt\
	jbo.txt kaj.txt kcg.txt kpe.txt mni.txt\
	moh.txt myv.txt nqo.txt nr.txt nso.txt\
	ny.txt oc.txt sa.txt sc.txt scn.txt\
	ss.txt st.txt syr.txt tig.txt tn.txt\
	trv.txt ts.txt ve.txt wa.txt wal.txt

REGION_ALIAS_SOURCE_LOCAL = wuu.txt
