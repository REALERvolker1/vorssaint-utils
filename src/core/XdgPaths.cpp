#include "core/XdgPaths.hpp"

#include <QDir>

namespace vorssaint {
namespace {

QString envOrFallback(const char *name, const QString &fallback) {
    const QString value = qEnvironmentVariable(name);
    return value.isEmpty() ? fallback : value;
}

QString appDir(const QString &base) {
    return QDir(base).filePath(QStringLiteral("vorssaint"));
}

} // namespace

QString XdgPaths::configHome() {
    return envOrFallback("XDG_CONFIG_HOME",
                         QDir(QDir::homePath()).filePath(QStringLiteral(".config")));
}

QString XdgPaths::dataHome() {
    return envOrFallback("XDG_DATA_HOME",
                         QDir(QDir::homePath()).filePath(QStringLiteral(".local/share")));
}

QString XdgPaths::cacheHome() {
    return envOrFallback("XDG_CACHE_HOME",
                         QDir(QDir::homePath()).filePath(QStringLiteral(".cache")));
}

QString XdgPaths::runtimeHome() {
    const QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (!runtime.isEmpty())
        return runtime;

    const QString uid = qEnvironmentVariable("UID", QStringLiteral("unknown"));
    return QDir(QDir::tempPath()).filePath(
        QStringLiteral("vorssaint-runtime-%1").arg(uid));
}

QString XdgPaths::configDir() { return appDir(configHome()); }
QString XdgPaths::dataDir() { return appDir(dataHome()); }
QString XdgPaths::cacheDir() { return appDir(cacheHome()); }
QString XdgPaths::runtimeDir() { return appDir(runtimeHome()); }

QString XdgPaths::settingsFile() {
    return QDir(configDir()).filePath(QStringLiteral("settings.json"));
}

bool XdgPaths::ensureDirectories() {
    bool ok = true;
    for (const QString &path : {configDir(), dataDir(), cacheDir(), runtimeDir()})
        ok = QDir().mkpath(path) && ok;
    return ok;
}

QJsonObject XdgPaths::toJson() {
    return {
        {QStringLiteral("configHome"), configHome()},
        {QStringLiteral("dataHome"), dataHome()},
        {QStringLiteral("cacheHome"), cacheHome()},
        {QStringLiteral("runtimeHome"), runtimeHome()},
        {QStringLiteral("configDir"), configDir()},
        {QStringLiteral("dataDir"), dataDir()},
        {QStringLiteral("cacheDir"), cacheDir()},
        {QStringLiteral("runtimeDir"), runtimeDir()},
        {QStringLiteral("settingsFile"), settingsFile()},
    };
}

} // namespace vorssaint
