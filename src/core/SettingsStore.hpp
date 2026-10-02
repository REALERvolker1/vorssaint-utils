#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace vorssaint {

class SettingsStore {
public:
    SettingsStore();

    QJsonObject load(QString *error = nullptr) const;
    bool save(const QJsonObject &settings, QString *error = nullptr) const;
    bool set(const QString &path, const QJsonValue &value, QString *error = nullptr) const;
    QJsonValue get(const QString &path, QString *error = nullptr) const;

    static QJsonObject defaults();

private:
    QString m_path;
};

} // namespace vorssaint
