/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "replicodesettings.h"
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QXmlStreamWriter>

ReplicodeSettings::ReplicodeSettings(QObject *parent)
    : QObject(parent)
{
    load();
}

void ReplicodeSettings::load()
{
    QSettings settings(QStringLiteral("replicode"), QStringLiteral("replicode"));

    settings.beginGroup(QStringLiteral("Load"));
    userOperatorPath = settings.value(QStringLiteral("User Operator Module Path"), QString()).toString();
    userClassPath = settings.value(QStringLiteral("User Class File Path"), QString()).toString();
    sourcePath = settings.value(QStringLiteral("Source File Path"), QString()).toString();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Init"));
    basePeriod = settings.value(QStringLiteral("Base Period"), 50000).toInt();
    reductionCoreCount = settings.value(QStringLiteral("Reduction Core Count"), 6).toInt();
    timeCoreCount = settings.value(QStringLiteral("Time Core Count"), 2).toInt();

    settings.beginGroup(QStringLiteral("System"));
    mdlInertiaSuccessRateThreshold = settings.value(QStringLiteral("Model Inertia Success Rate Threshold"), 0.9).toFloat();
    mdlInertiaCountThreshold = settings.value(QStringLiteral("Model Inertia Count Threshold"), 6).toInt();
    tpxDeltaSuccessRateThreshold = settings.value(QStringLiteral("Targeted Pattern Extractor Delta Success Rate Threshold"), 0.1).toFloat();
    minimumSimulationTimeHorizon = settings.value(QStringLiteral("Minimum Simulation Time Horizon"), 0).toInt();
    maximumSimulationTimeHorizon = settings.value(QStringLiteral("Maximum Simulation Time Horizon"), 0).toInt();
    simulationTimeHorizon = settings.value(QStringLiteral("Simulation Time Horizon"), 0.3).toFloat();
    tpxTimehorizon = settings.value(QStringLiteral("Targeted Pattern Extractor Time Horizon"), 500000).toInt();
    perfSamplingPeriod = settings.value(QStringLiteral("Perf Sampling Period"), 250000).toInt();
    floatTolerance = settings.value(QStringLiteral("Float Tolerance"), 0.00001).toFloat();
    timeTolerance = settings.value(QStringLiteral("Timer Tolerance"), 10000).toInt();
    primaryTimeHorizon = settings.value(QStringLiteral("Primary Time Horizon"), 3600000).toInt();
    secondaryTimeHorizon = settings.value(QStringLiteral("Secondary Time Horizon"), 7200000).toInt();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Debug"));
    debug = settings.value(QStringLiteral("Debug"), true).toBool();
    debugWindows = settings.value(QStringLiteral("Debug Windows"), 1).toInt();
    traceLevels = settings.value(QStringLiteral("Trace Levels"), QStringLiteral("CC")).toString().toInt(nullptr, 16);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Resilience"));
    notificationMarkerResilience = settings.value(QStringLiteral("Notification Marker Resilience"), 1).toInt();
    goalPredictionSuccessResilience = settings.value(QStringLiteral("Goal Prediction Success Resilience"), 1000).toInt();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Objects"));
    getObjects = settings.value(QStringLiteral("Get Objects"), true).toBool();
    decompileObjects = settings.value(QStringLiteral("Decompile Objects"), true).toBool();
    decompilationFilePath = settings.value(QStringLiteral("Decompilation Files Paths"), QString()).toString();
    ignoreNamedObjects = settings.value(QStringLiteral("Ignore Named Objects"), false).toBool();
    objectsPath = settings.value(QStringLiteral("Objects Path"), QString()).toString();
    testObjects = settings.value(QStringLiteral("Test Objects"), false).toBool();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Run"));
    runTime = settings.value(QStringLiteral("Run Time"), 1080).toInt();
    probeLevel = settings.value(QStringLiteral("Probe Level"), 2).toInt();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Models"));
    getModels = settings.value(QStringLiteral("Get Models"), false).toBool();
    decompileModels = settings.value(QStringLiteral("Decompile Models"), false).toBool();
    ignoreNamedModels = settings.value(QStringLiteral("Ignore Named Models"), true).toBool();
    modelsPath = settings.value(QStringLiteral("Models Path"), QString()).toString();
    testModels = settings.value(QStringLiteral("Test Models"), false).toBool();
    settings.endGroup();
}

void ReplicodeSettings::save()
{
    QSettings settings(QStringLiteral("replicode"), QStringLiteral("replicode"));

    settings.beginGroup(QStringLiteral("Load"));
    settings.setValue(QStringLiteral("User Operator Module Path"), userOperatorPath);
    settings.setValue(QStringLiteral("User Class File Path"), userClassPath);
    settings.setValue(QStringLiteral("Source File Path"), sourcePath);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Init"));
    settings.setValue(QStringLiteral("Base Period"), basePeriod);
    settings.setValue(QStringLiteral("Reduction Core Count"), reductionCoreCount);
    settings.setValue(QStringLiteral("Time Core Count"), timeCoreCount);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("System"));
    settings.setValue(QStringLiteral("Model Inertia Success Rate Threshold"), mdlInertiaSuccessRateThreshold);
    settings.setValue(QStringLiteral("Model Inertia Count Threshold"), mdlInertiaCountThreshold);
    settings.setValue(QStringLiteral("Targeted Pattern Extractor Delta Success Rate Threshold"), tpxDeltaSuccessRateThreshold);
    settings.setValue(QStringLiteral("Minimum Simulation Time Horizon"), minimumSimulationTimeHorizon);
    settings.setValue(QStringLiteral("Maximum Simulation Time Horizon"), maximumSimulationTimeHorizon);
    settings.setValue(QStringLiteral("Simulation Time Horizon"), simulationTimeHorizon);
    settings.setValue(QStringLiteral("Targeted Pattern Extractor Time Horizon"), tpxTimehorizon);
    settings.setValue(QStringLiteral("Perf Sampling Period"), perfSamplingPeriod);
    settings.setValue(QStringLiteral("Float Tolerance"), floatTolerance);
    settings.setValue(QStringLiteral("Timer Tolerance"), timeTolerance);
    settings.setValue(QStringLiteral("Primary Time Horizon"), primaryTimeHorizon);
    settings.setValue(QStringLiteral("Secondary Time Horizon"), secondaryTimeHorizon);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Debug"));
    settings.setValue(QStringLiteral("Debug"), debug);
    settings.setValue(QStringLiteral("Debug Windows"), debugWindows);
    settings.setValue(QStringLiteral("Trace Levels"), QString::number(traceLevels, 16));
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Resilience"));
    settings.setValue(QStringLiteral("Notification Marker Resilience"), notificationMarkerResilience);
    settings.setValue(QStringLiteral("Goal Prediction Success Resilience"), goalPredictionSuccessResilience);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Objects"));
    settings.setValue(QStringLiteral("Get Objects"), getObjects);
    settings.setValue(QStringLiteral("Decompile Objects"), decompileObjects);
    settings.setValue(QStringLiteral("Decompilation Files Paths"), decompilationFilePath);
    settings.setValue(QStringLiteral("Ignore Named Objects"), ignoreNamedObjects);
    settings.setValue(QStringLiteral("Objects Path"), objectsPath);
    settings.setValue(QStringLiteral("Test Objects"), testObjects);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Run"));
    settings.setValue(QStringLiteral("Run Time"), runTime);
    settings.setValue(QStringLiteral("Probe Level"), probeLevel);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Models"));
    settings.setValue(QStringLiteral("Get Models"), getModels);
    settings.setValue(QStringLiteral("Decompile Models"), decompileModels);
    settings.setValue(QStringLiteral("Ignore Named Models"), ignoreNamedModels);
    settings.setValue(QStringLiteral("Models Path"), modelsPath);
    settings.setValue(QStringLiteral("Test Models"), testModels);
}

void ReplicodeSettings::setDefaults()
{
    // Load
    userOperatorPath = QString();
    userClassPath = QString();
    sourcePath = QString();

    // Init
    basePeriod = 50000;
    reductionCoreCount = 6;
    timeCoreCount = 2;

    // System
    mdlInertiaSuccessRateThreshold = 0.9f;
    mdlInertiaCountThreshold = 6;
    tpxDeltaSuccessRateThreshold = 0.1f;
    minimumSimulationTimeHorizon = 0;
    maximumSimulationTimeHorizon = 0;
    simulationTimeHorizon = 0.3f;
    tpxTimehorizon = 500000;
    perfSamplingPeriod = 250000;
    floatTolerance = 0.00001f;
    timeTolerance = 10000;
    primaryTimeHorizon = 3600000;
    secondaryTimeHorizon = 7200000;

    // Debug
    debug = true;
    debugWindows = 1;
    traceLevels = 0xCC;

    // Debug Resilience
    notificationMarkerResilience = 1;
    goalPredictionSuccessResilience = 1000;

    // Debug Objects
    getObjects = true;
    decompileObjects = true;
    decompilationFilePath = QString();
    ignoreNamedObjects = false;
    objectsPath = QString();
    testObjects = false;

    // Run
    runTime = 1080;
    probeLevel = 2;
    getModels = false;
    decompileModels = false;
    ignoreNamedModels = true;
    modelsPath = QString();
    testModels = false;
}
