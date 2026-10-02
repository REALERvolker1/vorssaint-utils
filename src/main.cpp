#include "core/SettingsStore.hpp"
#include "core/XdgPaths.hpp"
#include "system/HyprlandManager.hpp"
#include "system/SystemMetrics.hpp"
#include "system/SystemdManager.hpp"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>

using namespace vorssaint;

namespace {

int printJson(const QJsonValue &value, int exitCode = 0) {
    const QJsonDocument document =
        value.isArray() ? QJsonDocument(value.toArray())
                        : QJsonDocument(value.toObject());
    QTextStream(stdout) << document.toJson(QJsonDocument::Compact) << Qt::endl;
    return exitCode;
}

QJsonValue parseScalar(const QString &text) {
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(
        QByteArrayLiteral("[") + text.toUtf8() + QByteArrayLiteral("]"),
        &error);

    if (error.error == QJsonParseError::NoError &&
        doc.isArray() && !doc.array().isEmpty())
        return doc.array().first();

    return text;
}

QString cleanUrl(const QString &input) {
    QUrl url = QUrl::fromUserInput(input);
    if (!url.isValid())
        return input;

    static const QStringList exact = {
        QStringLiteral("fbclid"), QStringLiteral("gclid"),
        QStringLiteral("dclid"), QStringLiteral("igshid"),
        QStringLiteral("mc_cid"), QStringLiteral("mc_eid"),
        QStringLiteral("si"),
    };

    QUrlQuery query(url);
    const auto items = query.queryItems(QUrl::FullyDecoded);
    query.clear();

    for (const auto &item : items) {
        const QString key = item.first.toLower();
        if (key.startsWith(QStringLiteral("utm_")) || exact.contains(key))
            continue;
        query.addQueryItem(item.first, item.second);
    }

    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

QJsonObject doctor() {
    const auto executable = [](const QString &name) {
        const QString path = QStandardPaths::findExecutable(name);
        return QJsonObject{
            {QStringLiteral("found"), !path.isEmpty()},
            {QStringLiteral("path"), path},
        };
    };

    return {
        {QStringLiteral("xdg"), XdgPaths::toJson()},
        {QStringLiteral("quickshell"), executable(QStringLiteral("qs"))},
        {QStringLiteral("hyprctl"), executable(QStringLiteral("hyprctl"))},
        {QStringLiteral("systemctl"), executable(QStringLiteral("systemctl"))},
        {QStringLiteral("pkexec"), executable(QStringLiteral("pkexec"))},
    };
}

void usage() {
    QTextStream out(stdout);
    out << "vorssaintctl commands:\n"
        << "  metrics\n"
        << "  doctor\n"
        << "  xdg\n"
        << "  url clean <url>\n"
        << "  settings dump\n"
        << "  settings get <path>\n"
        << "  settings set <path> <json-or-string>\n"
        << "  services list <all|user|system> [query] [limit]\n"
        << "  services inspect <user|system> <unit.service>\n"
        << "  services action <user|system> <verb> <unit.service>\n"
        << "  hypr open-workspace <monitor> <1..9999>\n"
        << "  hypr move-active <1..9999>\n";
}

} // namespace

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("vorssaintctl"));
    QCoreApplication::setOrganizationName(QStringLiteral("vorssaint"));

    XdgPaths::ensureDirectories();

    const QStringList args = app.arguments().mid(1);
    if (args.isEmpty() || args.first() == QStringLiteral("help") ||
        args.first() == QStringLiteral("--help")) {
        usage();
        return 0;
    }

    const QString command = args.first();

    if (command == QStringLiteral("metrics"))
        return printJson(SystemMetrics::snapshot());

    if (command == QStringLiteral("doctor"))
        return printJson(doctor());

    if (command == QStringLiteral("xdg"))
        return printJson(XdgPaths::toJson());

    if (command == QStringLiteral("url") && args.size() >= 3 &&
        args.at(1) == QStringLiteral("clean")) {
        return printJson(QJsonObject{
            {QStringLiteral("url"), cleanUrl(args.mid(2).join(QLatin1Char(' ')))}
        });
    }

    SettingsStore settings;
    if (command == QStringLiteral("settings")) {
        if (args.size() >= 2 && args.at(1) == QStringLiteral("dump"))
            return printJson(settings.load());

        if (args.size() >= 3 && args.at(1) == QStringLiteral("get")) {
            return printJson(QJsonObject{
                {QStringLiteral("value"), settings.get(args.at(2))}
            });
        }

        if (args.size() >= 4 && args.at(1) == QStringLiteral("set")) {
            QString error;
            const bool ok = settings.set(
                args.at(2),
                parseScalar(args.mid(3).join(QLatin1Char(' '))),
                &error);
            return printJson(QJsonObject{
                {QStringLiteral("ok"), ok},
                {QStringLiteral("error"), error},
                {QStringLiteral("settings"), settings.load()},
            }, ok ? 0 : 1);
        }
    }

    if (command == QStringLiteral("services")) {
        SystemdManager manager;

        if (args.size() >= 3 && args.at(1) == QStringLiteral("list")) {
            QString error;
            const QJsonArray units = manager.list(
                args.at(2),
                args.value(3),
                args.value(4, QStringLiteral("100")).toInt(),
                &error);
            return printJson(QJsonObject{
                {QStringLiteral("units"), units},
                {QStringLiteral("error"), error},
            });
        }

        if (args.size() >= 4 && args.at(1) == QStringLiteral("inspect"))
            return printJson(manager.inspect(args.at(2), args.at(3)));

        if (args.size() >= 5 && args.at(1) == QStringLiteral("action")) {
            const QJsonObject result =
                manager.action(args.at(2), args.at(3), args.at(4));
            return printJson(result,
                result.value(QStringLiteral("ok")).toBool() ? 0 : 1);
        }
    }

    if (command == QStringLiteral("hypr")) {
        HyprlandManager manager;

        if (args.size() >= 4 && args.at(1) == QStringLiteral("open-workspace")) {
            const QJsonObject result =
                manager.openWorkspace(args.at(2), args.at(3));
            return printJson(result,
                result.value(QStringLiteral("ok")).toBool() ? 0 : 1);
        }

        if (args.size() >= 3 && args.at(1) == QStringLiteral("move-active")) {
            const QJsonObject result = manager.moveActiveToWorkspace(args.at(2));
            return printJson(result,
                result.value(QStringLiteral("ok")).toBool() ? 0 : 1);
        }
    }

    usage();
    return 2;
}
