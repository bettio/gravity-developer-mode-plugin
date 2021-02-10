/*
 *
 */

#include "DeveloperModePlugin.h"

#include "DeviceInfo.h"

#include <QtCore/QLoggingCategory>

#include <HemeraCore/CommonOperations>

#include <GravitySupermassive/GalaxyManager>

Q_LOGGING_CATEGORY(LOG_DEVELOPERMODE, "DeveloperModePlugin")

using namespace Gravity;

DeveloperModePlugin::DeveloperModePlugin()
    : Gravity::Plugin()
{
    setName(QStringLiteral("Hemera Developer Mode"));
}

DeveloperModePlugin::~DeveloperModePlugin()
{
}

void DeveloperModePlugin::unload()
{
    qCDebug(LOG_DEVELOPERMODE) << "Developer mode plugin unloaded";
    setUnloaded();
}

void DeveloperModePlugin::load()
{
    m_deviceInfo = new DeviceInfo(galaxyManager());

    connect(m_deviceInfo->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op){
        if (op->isError()) {
            qCDebug(LOG_DEVELOPERMODE) << "Error in loading DeviceInfo: " << op->errorName() << op->errorMessage();
        } else {
            qCDebug(LOG_DEVELOPERMODE) << "Developer mode plugin loaded";
            setLoaded();
        }
    });
}

