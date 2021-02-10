#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <HemeraCore/AsyncInitObject>

namespace Gravity {
class GalaxyManager;
}

class QFileSystemWatcher;
class DeviceInfoProducer;

class DeviceInfo : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceInfo)

    Q_PROPERTY(QString          applianceName       READ applianceName)
    Q_PROPERTY(QString          boardName           READ boardName)
    Q_PROPERTY(QString          hardwareId          READ hardwareId)
    Q_PROPERTY(QString          applianceId         READ applianceId)
    Q_PROPERTY(QStringList      installedApps       READ installedApps NOTIFY installedAppsChanged)
    Q_PROPERTY(bool             isProductionBoard   READ isProductionBoard)
    Q_PROPERTY(bool             hasBuildEnvironment READ hasBuildEnvironment)
    Q_PROPERTY(QStringList      stars               READ stars)

    // Now, for infos on the system
    Q_PROPERTY(QString          architecture        READ architecture)
    Q_PROPERTY(QString          hemeraRelease       READ hemeraRelease)
    Q_PROPERTY(QStringList      cpuFlags            READ cpuFlags)
    Q_PROPERTY(int              cpuFrequency        READ cpuFrequency)
    Q_PROPERTY(int              availableCores      READ availableCores)
    Q_PROPERTY(int              totalMemory         READ totalMemory)

public:
    explicit DeviceInfo(Gravity::GalaxyManager *manager, QObject *parent = nullptr);
    virtual ~DeviceInfo();

    QString applianceName() const;
    QString boardName() const;
    QString hardwareId() const;
    QString applianceId() const;
    QStringList installedApps() const;
    bool isProductionBoard() const;
    bool hasBuildEnvironment() const;
    QStringList stars() const;

    QString architecture() const;
    QString hemeraRelease() const;
    QStringList cpuFlags() const;
    int cpuFrequency() const;
    int availableCores() const;
    int totalMemory() const;

    virtual bool isValid() { return true; }

Q_SIGNALS:
	void installedAppsChanged();

protected:
    virtual void initImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private Q_SLOTS:
    void cacheInstalledApplications();
    void sendAll();

private:
    Gravity::GalaxyManager *m_galaxyManager;
    QFileSystemWatcher *m_fileSystemWatcher;
    DeviceInfoProducer *m_producer;

    QString m_applianceId;
    QString m_hardwareId;
    QString m_applianceName;
    QString m_boardName;
    QStringList m_installedApps;
    bool m_isProductionBoard;
    bool m_hasBuildEnvironment;
    QStringList m_stars;

    QString m_architecture;
    QString m_hemeraRelease;
    QStringList m_cpuFlags;
    quint64 m_cpuFrequency;
    uint m_availableCores;
    quint64 m_totalMemory;

};

#endif
