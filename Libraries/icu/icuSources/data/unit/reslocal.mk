#
# Local Apple addition for region resources
# Copyright (c) 2014-2019 Apple Inc. All rights reserved.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# NOTE (January 2019): Please use ICU's new data filtering to select locale
# files.  This makefile is no longer used to filter locale files.
# With no filtering, all data files are built.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

UNIT_SOURCE_LOCAL = iu.txt ms_Arab.txt\
	en_AD.txt en_AL.txt en_AR.txt en_BA.txt en_BD.txt en_BG.txt\
	en_CZ.txt en_EE.txt en_ES.txt en_FR.txt\
	en_GR.txt en_HR.txt en_HU.txt en_ID.txt en_IS.txt\
	en_IT.txt en_JP.txt en_LT.txt en_LU.txt en_LV.txt\
	en_ME.txt en_MM.txt en_MV.txt en_NO.txt en_PL.txt en_PT.txt\
	en_RO.txt en_RS.txt en_RU.txt en_SK.txt en_TH.txt en_TR.txt en_UA.txt\
	es_AG.txt es_AI.txt es_AW.txt es_BB.txt es_BL.txt\
	es_BM.txt es_BQ.txt es_BS.txt es_CA.txt es_CW.txt es_DM.txt\
	es_FK.txt es_GD.txt es_GF.txt es_GL.txt es_GP.txt\
	es_GY.txt es_HT.txt es_KN.txt es_KY.txt es_LC.txt\
	es_MF.txt es_MQ.txt es_MS.txt es_PM.txt es_SR.txt\
	es_SX.txt es_TC.txt es_TT.txt es_VC.txt es_VG.txt\
	es_VI.txt\
	pt_FR.txt\
	arn.txt /ba.txt /byn.txt /co.txt /cv.txt\
	dv.txt /gaa.txt /gez.txt /gn.txt /io.txt\
	jbo.txt /kaj.txt /kcg.txt /kpe.txt /mni.txt\
	moh.txt /myv.txt /nqo.txt /nr.txt /nso.txt\
	ny.txt /oc.txt /sa.txt /sc.txt /scn.txt\
	ss.txt /st.txt /syr.txt /tig.txt /tn.txt\
	trv.txt /ts.txt /ve.txt /wa.txt /wal.txt

UNIT_ALIAS_SOURCE_LOCAL = wuu.txt
