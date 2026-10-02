#include "core/SettingsStore.hpp"
#include "core/XdgPaths.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace vorssaint {
namespace {

QJsonObject mergeObjects(QJsonObject base, const QJsonObject &overlay) {
    for (auto it = overlay.constBegin(); it != overlay.constEnd(); ++it) {
        if (it.value().isObject() && base.value(it.key()).isObject())
            base.insert(it.key(), mergeObjects(base.value(it.key()).toObject(), it.value().toObject()));
        else
            base.insert(it.key(), it.value());
    }
    return base;
}

QJsonObject setPath(QJsonObject object, const QStringList &parts,
                    const QJsonValue &value, int index = 0) {
    const QString &key = parts.at(index);
    if (index == parts.size() - 1) {
        object.insert(key, value);
        return object;
    }

    object.insert(key, setPath(object.value(key).toObject(), parts, value, index + 1));
    return object;
}

QJsonValue valueAtPath(const QJsonObject &object, const QStringList &parts) {
    QJsonValue current(object);
    for (const QString &part : parts) {
        if (!current.isObject())
            return {};
        current = current.toObject().value(part);
    }
    return current;
}

} // namespace

SettingsStore::SettingsStore() : m_path(XdgPaths::settingsFile()) {
    XdgPaths::ensureDirectories();
}

QJsonObject SettingsStore::defaults() {
    return {
        {QStringLiteral("panel"), QJsonObject{
            {QStringLiteral("width"), 760},
            {QStringLiteral("height"), 600},
        }},
        {QStringLiteral("system"), QJsonObject{
            {QStringLiteral("refreshMs"), 2000},
        }},
        {QStringLiteral("services"), QJsonObject{
            {QStringLiteral("defaultScope"), QStringLiteral("all")},
            {QStringLiteral("limit"), 100},
        }},
        {QStringLiteral("ui"), QJsonObject{
            {QStringLiteral("compactRows"), false},
            {QStringLiteral("showUtilities"), true},
        }},
    };
}

QJsonObject SettingsStore::load(QString *error) const {
    QFile file(m_path);
    if (!file.exists())
        return defaults();

    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = file.errorString();
        return defaults();
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) *error = parseError.errorString();
        return defaults();
    }

    return mergeObjects(defaults(), doc.object());
}

bool SettingsStore::save(const QJsonObject &settings, QString *error) const {
    if (!XdgPaths::ensureDirectories()) {
        if (error) *error = QStringLiteral("Could not create XDG directories");
        return false;
    }

    QSaveFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = file.errorString();
        return false;
    }

    file.write(QJsonDocument(settings).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}

bool SettingsStore::set(const QString &path, const QJsonValue &value, QString *error) const {
    const QStringList parts = path.split(QLatin1Char('.'), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        if (error) *error = QStringLiteral("Empty settings path");
        return false;
    }

    QJsonObject settings = load(error);
    return save(setPath(settings, parts, value), error);
}

QJsonValue SettingsStore::get(const QString &path, QString *error) const {
    const QStringList parts = path.split(QLatin1Char('.'), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        if (error) *error = QStringLiteral("Empty settings path");
        return {};
    }
    return valueAtPath(load(error), parts);
}

} // namespace vorssaint
