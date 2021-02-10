#include "DeviceInfo.h"

#include "DeviceInfoProducer.h"

#include <GravitySupermassive/GalaxyManager>
#include <GravitySupermassive/StarSequence>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Fingerprints>
#include <HemeraCore/Literals>

#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#include "developermodeconfig.h"

#include <sys/sysinfo.h>
#include <unistd.h>

#include <zypp/ZConfig.h>

Q_LOGGING_CATEGORY(LOG_DEVICEINFO, "DeviceInfo")

DeviceInfo::DeviceInfo(Gravity::GalaxyManager *manager, QObject *parent)
    : Hemera::AsyncInitObject(parent)
    , m_galaxyManager(manager)
    , m_fileSystemWatcher(new QFileSystemWatcher(this))
    , m_producer(new DeviceInfoProducer(this))
    , m_applianceName(manager->name())
    , m_isProductionBoard(!QFile::exists(QString::fromLatin1("%1/libgravity-center-plugin-developer-mode.so").arg(QLatin1String(StaticConfig::gravityCenterPluginsPath()))))
    , m_availableCores(sysconf(_SC_NPROCESSORS_ONLN))
    , m_totalMemory(get_phys_pages() * getpagesize())
{
}

DeviceInfo::~DeviceInfo()
{
}

void DeviceInfo::initImpl()
{
    setParts(3);

    // When we are ready, we send everything
    connect(this, &Hemera::AsyncInitObject::ready, this, &DeviceInfo::sendAll);

	// Hardware ID
    Hemera::ByteArrayOperation *op = Hemera::Fingerprints::globalHardwareId();
    connect(op, &Hemera::Operation::finished, this, [this, op] {
        if (op->isError()) {
            qCDebug(LOG_DEVICEINFO) << tr("Could not retrieve global hardware ID!");
        } else {
            m_hardwareId = QString::fromLatin1(op->result());
        }
        setOnePartIsReady();
    });

	// Appliance ID
    op = Hemera::Fingerprints::globalSystemId();
    connect(op, &Hemera::Operation::finished, this, [this, op] {
        if (!op->isError()) {
            qCDebug(LOG_DEVICEINFO) << tr("Could not retrieve global system ID!");
        } else {
            m_applianceId = QString::fromLatin1(op->result());
        }
        setOnePartIsReady();
    });

    // Installed applications
    {
        QDir appsDir(QString::fromLatin1(StaticConfig::hemeraServicesPath()));
        m_installedApps = appsDir.entryList(QStringList() << QStringLiteral("*.ha"),
                          QDir::Files).replaceInStrings(QStringLiteral(".ha"), QStringLiteral(""));
    }

    // keep an eye on hemera's installed applications
    m_fileSystemWatcher->addPath(QString::fromLatin1(StaticConfig::hemeraServicesPath()));

    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &DeviceInfo::cacheInstalledApplications);

    // Find out stuff about our current device
    // Stars
    {
        m_stars.clear();
        m_stars.reserve(m_galaxyManager->stars().count());
        for (const Gravity::StarSequence *star : m_galaxyManager->stars()) {
            m_stars.append(star->star());
        }
    }
    // Hemera release
    {
        QFile hr(QStringLiteral("/etc/hemera-release"));
        hr.open(QIODevice::ReadOnly | QIODevice::Text);
        m_hemeraRelease = QLatin1String(hr.readAll());
        m_hemeraRelease.remove(QLatin1Char('\n'));
    }
    // Board name
    {
        QFile bn(QStringLiteral("/etc/boardname"));
        bn.open(QIODevice::ReadOnly | QIODevice::Text);
        m_boardName = QLatin1String(bn.readAll());
        m_boardName.remove(QLatin1Char('\n'));
    }
    // Architecture (courtesy of zypp)
    {
        zypp::Arch systemArchitecture = zypp::ZConfig::instance().systemArchitecture();

        // Hemera has no such thing as i686. If we hit a i686 arch, we want to use i586 instead.
        if (systemArchitecture == zypp::Arch_i686) {
            systemArchitecture = zypp::Arch_i586;
        }
        // Same thing goes for i386. We're likely i486 then.
        else if (systemArchitecture == zypp::Arch_i386) {
            systemArchitecture = zypp::Arch_i486;
        }

        m_architecture = QString::fromStdString(systemArchitecture.asString());
    }
    // CPU Frequency and flags
    {
        QFile cpu(QStringLiteral("/proc/cpuinfo"));
        cpu.open(QIODevice::ReadOnly | QIODevice::Text);

        // Using QTextStream is necessary for files in /proc
        QTextStream in(&cpu);
        QString line = in.readLine();
        while (!line.isNull()) {
            if (line.contains(QStringLiteral("cpu MHz"))) {
                double cpufreq = line.split(QLatin1Char(':')).last().toDouble();
                cpufreq *= 1024;
                m_cpuFrequency = cpufreq;
            } else if (line.contains(QStringLiteral("flags"))) {
                QString flags = line.split(QLatin1Char(':')).last();
                // Clean whitespace
                while (flags.startsWith(QLatin1Char(' '))) {
                    flags.remove(0, 1);
                }

                m_cpuFlags = flags.split(QLatin1Char(' '));
            }

            line = in.readLine();
        }
    }
    // Build environment?
    {
        QDir dir(QStringLiteral("/srv/hemera/targets/"));
        m_hasBuildEnvironment = dir.exists() || QFile::exists(QStringLiteral("/usr/bin/gcc"));
    }

    setOnePartIsReady();
}

void DeviceInfo::sendAll()
{
    QTimer::singleShot(0, this, [this] {
        qCDebug(LOG_DEVICEINFO) << tr("Sending all device information");
        m_producer->setSystemInfoApplianceName(m_applianceName);
        m_producer->setSystemInfoBoardName(m_boardName);
        m_producer->setSystemInfoHardwareId(m_hardwareId);
        m_producer->setSystemInfoApplianceId(m_applianceId);
        m_producer->setSystemInfoInstalledApps(m_installedApps.join(QStringLiteral(",")));
        m_producer->setSystemInfoIsProductionBoard(m_isProductionBoard);
        m_producer->setSystemInfoHasBuildEnvironment(m_hasBuildEnvironment);
        m_producer->setSystemInfoStars(m_stars.join(QStringLiteral(",")));
        m_producer->setSystemInfoArchitecture(m_architecture);
        m_producer->setSystemInfoHemeraRelease(m_hemeraRelease);
        m_producer->setSystemInfoCpuFlags(m_cpuFlags.join(QStringLiteral(",")));
        m_producer->setSystemInfoCpuFrequency(m_cpuFrequency);
        m_producer->setSystemInfoAvailableCores(m_availableCores);
        m_producer->setSystemInfoTotalMemory(m_totalMemory);
    });
}

void DeviceInfo::cacheInstalledApplications()
{
    QDir appsDir(QString::fromLatin1(StaticConfig::hemeraServicesPath()));
    m_installedApps = appsDir.entryList(QStringList() << QStringLiteral("*.ha"),
                                        QDir::Files).replaceInStrings(QStringLiteral(".ha"), QStringLiteral(""));

    Q_EMIT installedAppsChanged();
    m_producer->setSystemInfoInstalledApps(m_installedApps.join(QStringLiteral(",")));
}

QString DeviceInfo::applianceName() const
{
    return m_applianceName;
}

QString DeviceInfo::architecture() const
{
    return m_architecture;
}

QString DeviceInfo::applianceId() const
{
    return m_applianceId;
}

QString DeviceInfo::hardwareId() const
{
    return m_hardwareId;
}

QString DeviceInfo::boardName() const
{
    return m_boardName;
}

QStringList DeviceInfo::installedApps() const
{
    return m_installedApps;
}

QStringList DeviceInfo::stars() const
{
    return m_stars;
}

bool DeviceInfo::isProductionBoard() const
{
    return m_isProductionBoard;
}

bool DeviceInfo::hasBuildEnvironment() const
{
    return m_hasBuildEnvironment;
}

QStringList DeviceInfo::cpuFlags() const
{
    return m_cpuFlags;
}

int DeviceInfo::cpuFrequency() const
{
    return m_cpuFrequency;
}

QString DeviceInfo::hemeraRelease() const
{
    return m_hemeraRelease;
}

int DeviceInfo::availableCores() const
{
    return m_availableCores;
}

int DeviceInfo::totalMemory() const
{
    return m_totalMemory;
}
