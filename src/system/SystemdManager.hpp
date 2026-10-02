#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace vorssaint {

class SystemdManager {
public:
    QJsonArray list(const QString &scope, const QString &query, int limit,
                    QString *error = nullptr) const;
    QJsonObject action(const QString &scope, const QString &verb,
                       const QString &unit) const;
    QJsonObject inspect(const QString &scope, const QString &unit) const;

private:
    QJsonArray collect(const QString &scope, QString *error) const;
};

} // namespace vorssaint
