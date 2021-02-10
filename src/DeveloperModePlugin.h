/*
 *
 */

#ifndef GRAVITY_DEVELOPERMODEPLUGIN_H
#define GRAVITY_DEVELOPERMODEPLUGIN_H

#include <gravityplugin.h>

namespace Hemera {
    class Operation;
}

class DeviceInfo;

namespace Gravity {

struct Orbit;

class DeveloperModePlugin : public Gravity::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ispirata.Hemera.GravityCenter.Plugin")
    Q_CLASSINFO("D-Bus Interface", "com.ispirata.Hemera.GravityCenter.Plugins.DeveloperMode")
    Q_INTERFACES(Gravity::Plugin)

public:
    explicit DeveloperModePlugin();
    virtual ~DeveloperModePlugin();

protected:
    virtual void unload() override final;
    virtual void load() override final;

private:
    DeviceInfo *m_deviceInfo;
};
}

#endif // GRAVITY_DEVELOPERMODEPLUGIN_H
