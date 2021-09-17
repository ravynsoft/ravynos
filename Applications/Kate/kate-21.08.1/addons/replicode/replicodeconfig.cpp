/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "replicodeconfig.h"
#include "replicodesettings.h"
#include "ui_config.h"

enum TraceLevels {
    CompositeInputs = 1 << 0,
    CompositeOutputs = 1 << 1,
    ModelInputs = 1 << 2,
    ModelOutputs = 1 << 3,
    PredictionMonitoring = 1 << 4,
    GoalMonitoring = 1 << 5,
    ModelRevision = 1 << 6,
    ModelCompositeInjection = 1 << 7
};

ReplicodeConfig::ReplicodeConfig(QWidget *parent)
    : QTabWidget(parent)
    , m_ui(new Ui_tabWidget)
    , m_settings(new ReplicodeSettings(this))
{
    m_ui->setupUi(this);
    load();
}

ReplicodeConfig::~ReplicodeConfig()
{
    delete m_ui;
}

void ReplicodeConfig::load()
{
    m_ui->basePeriod->setValue(m_settings->basePeriod);
    m_ui->debug->setChecked(m_settings->debug);
    m_ui->debugWindows->setValue(m_settings->debugWindows);
    m_ui->decObjFile->setText(m_settings->decompilationFilePath);
    m_ui->decompileModels->setChecked(m_settings->decompileModels);
    m_ui->decompileObjects->setChecked(m_settings->decompileObjects);
    m_ui->dumpModelsFile->setText(m_settings->modelsPath);
    m_ui->dumpObjFile->setText(m_settings->objectsPath);
    m_ui->floatTolerance->setValue(m_settings->floatTolerance);
    m_ui->goalPredSuccessRes->setValue(m_settings->goalPredictionSuccessResilience);
    m_ui->ignoreNamedModels->setChecked(m_settings->ignoreNamedModels);
    m_ui->maxSimTimeHoriz->setValue(m_settings->maximumSimulationTimeHorizon);
    m_ui->minSimTimeHoriz->setValue(m_settings->minimumSimulationTimeHorizon);
    m_ui->modelInertiacount->setValue(m_settings->mdlInertiaCountThreshold);
    m_ui->modelInertiaSuccessRate->setValue(m_settings->mdlInertiaSuccessRateThreshold);
    m_ui->notifMarkerRes->setValue(m_settings->notificationMarkerResilience);
    m_ui->perfSamplePeriod->setValue(m_settings->perfSamplingPeriod);
    m_ui->primaryTimeHoriz->setValue(m_settings->primaryTimeHorizon);
    m_ui->probeLevel->setValue(m_settings->probeLevel);
    m_ui->reductionCoreCount->setValue(m_settings->reductionCoreCount);
    m_ui->runTime->setValue(m_settings->runTime);
    m_ui->secondaryTimeHoriz->setValue(m_settings->secondaryTimeHorizon);
    m_ui->simTimeHoriz->setValue(m_settings->simulationTimeHorizon);
    m_ui->testModels->setChecked(m_settings->testModels);
    m_ui->testObj->setChecked(m_settings->testObjects);
    m_ui->timeCoreCount->setValue(m_settings->timeCoreCount);
    m_ui->timerTolerance->setValue(m_settings->timeTolerance);
    m_ui->tpxDeltaSuccessRate->setValue(m_settings->tpxDeltaSuccessRateThreshold);
    m_ui->tpxTimeHoriz->setValue(m_settings->tpxTimehorizon);
    m_ui->userClassPath->setText(m_settings->userClassPath);
    m_ui->userModulePath->setText(m_settings->userOperatorPath);

    m_ui->traceCompInputs->setChecked(m_settings->traceLevels & CompositeInputs);
    m_ui->traceCompOutputs->setChecked(m_settings->traceLevels & CompositeOutputs);
    m_ui->traceModelIn->setChecked(m_settings->traceLevels & ModelInputs);
    m_ui->traceModelOut->setChecked(m_settings->traceLevels & ModelOutputs);
    m_ui->tracePredMon->setChecked(m_settings->traceLevels & PredictionMonitoring);
    m_ui->traceGoalMon->setChecked(m_settings->traceLevels & GoalMonitoring);
    m_ui->traceModelRev->setChecked(m_settings->traceLevels & ModelRevision);
    m_ui->traceModComInj->setChecked(m_settings->traceLevels & ModelCompositeInjection);
}

void ReplicodeConfig::save()
{
    m_settings->basePeriod = m_ui->basePeriod->value();
    m_settings->debug = m_ui->debug->isChecked();
    m_settings->debugWindows = m_ui->debugWindows->value();
    m_settings->decompilationFilePath = m_ui->decObjFile->text();
    m_settings->decompileModels = m_ui->decompileModels->isChecked();
    m_settings->decompileObjects = m_ui->decompileObjects->isChecked();
    m_settings->modelsPath = m_ui->dumpModelsFile->text();
    m_settings->objectsPath = m_ui->dumpObjFile->text();
    m_settings->floatTolerance = m_ui->floatTolerance->value();
    m_settings->goalPredictionSuccessResilience = m_ui->goalPredSuccessRes->value();
    m_settings->ignoreNamedModels = m_ui->ignoreNamedModels->isChecked();
    m_settings->maximumSimulationTimeHorizon = m_ui->maxSimTimeHoriz->value();
    m_settings->minimumSimulationTimeHorizon = m_ui->minSimTimeHoriz->value();
    m_settings->mdlInertiaCountThreshold = m_ui->modelInertiacount->value();
    m_settings->mdlInertiaSuccessRateThreshold = m_ui->modelInertiaSuccessRate->value();
    m_settings->notificationMarkerResilience = m_ui->notifMarkerRes->value();
    m_settings->perfSamplingPeriod = m_ui->perfSamplePeriod->value();
    m_settings->primaryTimeHorizon = m_ui->primaryTimeHoriz->value();
    m_settings->probeLevel = m_ui->probeLevel->value();
    m_settings->reductionCoreCount = m_ui->reductionCoreCount->value();
    m_settings->runTime = m_ui->runTime->value();
    m_settings->secondaryTimeHorizon = m_ui->secondaryTimeHoriz->value();
    m_settings->simulationTimeHorizon = m_ui->simTimeHoriz->value();
    m_settings->testModels = m_ui->testModels->isChecked();
    m_settings->testObjects = m_ui->testObj->isChecked();
    m_settings->timeCoreCount = m_ui->timeCoreCount->value();
    m_settings->timeTolerance = m_ui->timerTolerance->value();
    m_settings->tpxDeltaSuccessRateThreshold = m_ui->tpxDeltaSuccessRate->value();
    m_settings->tpxTimehorizon = m_ui->tpxTimeHoriz->value();
    m_settings->userClassPath = m_ui->userClassPath->text();
    m_settings->userOperatorPath = m_ui->userModulePath->text();

    int trace = 0;
    if (m_ui->traceCompInputs->isChecked()) {
        trace |= CompositeInputs;
    }
    if (m_ui->traceCompOutputs->isChecked()) {
        trace |= CompositeOutputs;
    }
    if (m_ui->traceModelIn->isChecked()) {
        trace |= ModelInputs;
    }
    if (m_ui->traceModelOut->isChecked()) {
        trace |= ModelOutputs;
    }
    if (m_ui->tracePredMon->isChecked()) {
        trace |= PredictionMonitoring;
    }
    if (m_ui->traceGoalMon->isChecked()) {
        trace |= GoalMonitoring;
    }
    if (m_ui->traceModelRev->isChecked()) {
        trace |= ModelRevision;
    }
    if (m_ui->traceModComInj->isChecked()) {
        trace |= ModelCompositeInjection;
    }
    m_settings->traceLevels = trace;
    m_settings->save();
}

void ReplicodeConfig::reset()
{
    m_settings->setDefaults();
    load();
}
