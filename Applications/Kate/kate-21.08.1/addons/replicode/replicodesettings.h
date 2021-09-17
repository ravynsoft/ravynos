/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2014 Martin Sandsmark <martin.sandsmark@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPLICODESETTINGS_H
#define REPLICODESETTINGS_H

#include <QObject>

class QIODevice;

class ReplicodeSettings : public QObject
{
    Q_OBJECT
public:
    explicit ReplicodeSettings(QObject *parent = nullptr);
    void load();
    void save();
    void setDefaults();

    ///////
    // Load
    QString userOperatorPath;
    QString userClassPath;
    QString sourcePath;

    ///////
    // Init
    int basePeriod = 0;
    int reductionCoreCount = 0;
    int timeCoreCount = 0;

    /////////
    // System
    int perfSamplingPeriod = 0;
    float floatTolerance = 0;
    int timeTolerance = 0;
    int primaryTimeHorizon = 0;
    int secondaryTimeHorizon = 0;

    // Model
    float mdlInertiaSuccessRateThreshold = 0;
    int mdlInertiaCountThreshold = 0;

    // Targeted Pattern Extractor
    float tpxDeltaSuccessRateThreshold = 0;
    int tpxTimehorizon{};

    // Simulation
    int minimumSimulationTimeHorizon = 0;
    int maximumSimulationTimeHorizon = 0;
    float simulationTimeHorizon = 0;

    ////////
    // Debug
    bool debug = false;
    int notificationMarkerResilience = 0;
    int goalPredictionSuccessResilience = 0;
    int debugWindows = 0;
    int traceLevels = 0;

    bool getObjects = false;
    bool decompileObjects = false;
    QString decompilationFilePath;
    bool ignoreNamedObjects = false;
    QString objectsPath;
    bool testObjects = false;

    //////
    // Run
    int runTime = 0;
    int probeLevel = 0;

    bool getModels = false;
    bool decompileModels = false;
    bool ignoreNamedModels = false;
    QString modelsPath;
    bool testModels = false;
};

#endif // REPLICODESETTINGS_H
