#include "system/SystemMetrics.hpp"

#include <QFile>
#include <QJsonObject>
#include <QStorageInfo>
#include <QSysInfo>
#include <QThread>

namespace vorssaint {
namespace {

struct CpuSample {
    quint64 idle = 0;
    quint64 total = 0;
};

CpuSample cpuSample() {
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QList<QByteArray> fields = file.readLine().simplified().split(' ');
    if (fields.size() < 5 || fields.first() != "cpu")
        return {};

    CpuSample sample;
    for (int i = 1; i < fields.size(); ++i) {
        bool ok = false;
        const quint64 value = fields.at(i).toULongLong(&ok);
        if (!ok) continue;
        sample.total += value;
        if (i == 4 || i == 5)
            sample.idle += value;
    }
    return sample;
}

double cpuPercent() {
    const CpuSample a = cpuSample();
    QThread::msleep(80);
    const CpuSample b = cpuSample();

    const quint64 total = b.total - a.total;
    const quint64 idle = b.idle - a.idle;
    if (total == 0)
        return 0.0;
    return 100.0 * (1.0 - static_cast<double>(idle) /
                              static_cast<double>(total));
}

QJsonObject memory() {
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    quint64 totalKb = 0;
    quint64 availableKb = 0;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine();
        if (line.startsWith("MemTotal:"))
            totalKb = line.simplified().split(' ').value(1).toULongLong();
        else if (line.startsWith("MemAvailable:"))
            availableKb = line.simplified().split(' ').value(1).toULongLong();
    }

    const quint64 usedKb = totalKb > availableKb ? totalKb - availableKb : 0;
    return {
        {QStringLiteral("totalBytes"), static_cast<qint64>(totalKb * 1024)},
        {QStringLiteral("availableBytes"), static_cast<qint64>(availableKb * 1024)},
        {QStringLiteral("usedBytes"), static_cast<qint64>(usedKb * 1024)},
        {QStringLiteral("usedPercent"),
         totalKb == 0 ? 0.0 : (100.0 * usedKb / totalKb)},
    };
}

QJsonObject loadAverage() {
    QFile file(QStringLiteral("/proc/loadavg"));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QList<QByteArray> parts = file.readAll().simplified().split(' ');
    return {
        {QStringLiteral("one"), parts.value(0).toDouble()},
        {QStringLiteral("five"), parts.value(1).toDouble()},
        {QStringLiteral("fifteen"), parts.value(2).toDouble()},
    };
}

double uptimeSeconds() {
    QFile file(QStringLiteral("/proc/uptime"));
    if (!file.open(QIODevice::ReadOnly))
        return 0;
    return file.readAll().split(' ').value(0).toDouble();
}

QJsonObject networkTotals() {
    QFile file(QStringLiteral("/proc/net/dev"));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    quint64 rx = 0;
    quint64 tx = 0;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon < 0)
            continue;

        const QString iface = line.left(colon).trimmed();
        if (iface == QStringLiteral("lo"))
            continue;

        const QStringList fields =
            line.mid(colon + 1).simplified().split(QLatin1Char(' '));
        if (fields.size() < 9)
            continue;

        rx += fields.at(0).toULongLong();
        tx += fields.at(8).toULongLong();
    }

    return {
        {QStringLiteral("rxBytes"), static_cast<qint64>(rx)},
        {QStringLiteral("txBytes"), static_cast<qint64>(tx)},
    };
}

} // namespace

QJsonObject SystemMetrics::snapshot() {
    const QStorageInfo root = QStorageInfo::root();

    return {
        {QStringLiteral("hostname"), QSysInfo::machineHostName()},
        {QStringLiteral("kernel"),
         QSysInfo::kernelType() + QLatin1Char(' ') + QSysInfo::kernelVersion()},
        {QStringLiteral("cpuPercent"), cpuPercent()},
        {QStringLiteral("memory"), memory()},
        {QStringLiteral("load"), loadAverage()},
        {QStringLiteral("uptimeSeconds"), uptimeSeconds()},
        {QStringLiteral("network"), networkTotals()},
        {QStringLiteral("disk"), QJsonObject{
            {QStringLiteral("totalBytes"),
             root.isValid() ? static_cast<qint64>(root.bytesTotal()) : 0},
            {QStringLiteral("availableBytes"),
             root.isValid() ? static_cast<qint64>(root.bytesAvailable()) : 0},
        }},
    };
}

} // namespace vorssaint
