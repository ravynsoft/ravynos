#
# Local Apple addition for locale resources
# Copyright (c) 2004-2005, 2007, 2012-2019 Apple Inc. All rights reserved.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# NOTE (January 2019): Please use ICU's new data filtering to select locale
# files.  This makefile is no longer used to filter locale files.
# With no filtering, all data files are built.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

GENRB_SOURCE_LOCAL = iu.txt iu_CA.txt\
	ms_Arab.txt ms_Arab_BN.txt ms_Arab_MY.txt\
	en_AD.txt en_AL.txt en_AR.txt en_BA.txt en_BD.txt\
	en_BG.txt\
	en_BR.txt en_CL.txt en_CN.txt en_CO.txt en_CZ.txt en_EE.txt en_ES.txt\
	en_FR.txt en_GR.txt en_HR.txt en_HU.txt en_ID.txt\
	en_IS.txt en_IT.txt en_JP.txt en_KR.txt en_LT.txt\
	en_LU.txt en_LV.txt en_ME.txt en_MM.txt en_MV.txt en_MX.txt en_NO.txt\
	en_PL.txt en_PT.txt en_RO.txt en_RS.txt en_RU.txt en_SK.txt\
	en_TR.txt en_TW.txt en_UA.txt\
	es_AG.txt es_AI.txt es_AW.txt es_BB.txt es_BL.txt\
	es_BM.txt es_BQ.txt es_BS.txt es_CA.txt es_CW.txt es_DM.txt\
	es_FK.txt es_GD.txt es_GF.txt es_GL.txt es_GP.txt\
	es_GY.txt es_HT.txt es_KN.txt es_KY.txt es_LC.txt\
	es_MF.txt es_MQ.txt es_MS.txt es_PM.txt en_SA.txt es_SR.txt\
	es_SX.txt es_TC.txt en_TH.txt es_TT.txt es_VC.txt es_VG.txt\
	es_VI.txt\
	pt_FR.txt\
	ks_Arab.txt ks_Arab_IN.txt\
	ks_Aran.txt ks_Aran_IN.txt\
	ur_Arab.txt ur_Arab_IN.txt ur_Arab_PK.txt\
	ur_Aran.txt ur_Aran_IN.txt ur_Aran_PK.txt\
	arn_CL.txt arn.txt ba_RU.txt ba.txt byn_ER.txt\
	byn.txt co_FR.txt co.txt cv_RU.txt cv.txt\
	dv_MV.txt dv.txt gaa_GH.txt gaa.txt gez_ER.txt\
	gez_ET.txt gez.txt gn_PY.txt gn.txt io_001.txt\
	io.txt jbo_001.txt jbo.txt kaj_NG.txt kaj.txt\
	kcg_NG.txt kcg.txt kpe_GN.txt kpe_LR.txt kpe.txt\
	mni_IN.txt mni.txt moh_CA.txt moh.txt myv_RU.txt\
	myv.txt nqo_GN.txt nqo.txt nr_ZA.txt nr.txt\
	nso_ZA.txt nso.txt ny_MW.txt ny.txt oc_FR.txt\
	oc.txt sa_IN.txt sa.txt sc_IT.txt sc.txt\
	scn_IT.txt scn.txt ss_SZ.txt ss_ZA.txt ss.txt\
	st_LS.txt st_ZA.txt st.txt syr_IQ.txt syr_SY.txt\
	syr.txt tig_ER.txt tig.txt tn_BW.txt tn_ZA.txt\
	tn.txt trv_TW.txt trv.txt ts_ZA.txt ts.txt\
	ve_ZA.txt ve.txt wa_BE.txt wa.txt wal_ET.txt\
	wal.txt

GENRB_ALIAS_SOURCE_LOCAL = wuu.txt
