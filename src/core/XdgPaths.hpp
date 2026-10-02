#pragma once

#include <QJsonObject>
#include <QString>

namespace vorssaint {

class XdgPaths {
public:
    static QString configHome();
    static QString dataHome();
    static QString cacheHome();
    static QString runtimeHome();

    static QString configDir();
    static QString dataDir();
    static QString cacheDir();
    static QString runtimeDir();
    static QString settingsFile();

    static bool ensureDirectories();
    static QJsonObject toJson();
};

} // namespace vorssaint
