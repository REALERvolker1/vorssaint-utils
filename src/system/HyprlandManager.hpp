#pragma once

#include <QJsonObject>
#include <QString>

namespace vorssaint {

class HyprlandManager {
public:
    QJsonObject openWorkspace(const QString &monitor, const QString &workspace) const;
    QJsonObject moveActiveToWorkspace(const QString &workspace) const;
};

} // namespace vorssaint
