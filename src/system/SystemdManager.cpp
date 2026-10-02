#include "system/SystemdManager.hpp"

#include "search/FuzzyMatcher.hpp"
#include "security/Validation.hpp"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>

#include <algorithm>

namespace vorssaint {
namespace {

struct CommandResult {
    int exitCode = -1;
    QString out;
    QString err;
    bool timedOut = false;
};

CommandResult runCommand(const QString &program, const QStringList &arguments,
                         int timeoutMs = 7000) {
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.start();

    CommandResult result;
    if (!process.waitForStarted(2000)) {
        result.err = process.errorString();
        return result;
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished();
        result.timedOut = true;
        result.err = QStringLiteral("Command timed out");
        return result;
    }

    result.exitCode = process.exitCode();
    result.out = QString::fromUtf8(process.readAllStandardOutput());
    result.err = QString::fromUtf8(process.readAllStandardError()).trimmed();
    return result;
}

QStringList scopeArgs(const QString &scope) {
    return scope == QStringLiteral("user")
        ? QStringList{QStringLiteral("--user")}
        : QStringList{};
}

bool validScope(const QString &scope) {
    return scope == QStringLiteral("user") || scope == QStringLiteral("system");
}

QJsonObject resultJson(const CommandResult &result) {
    return {
        {QStringLiteral("ok"), result.exitCode == 0 && !result.timedOut},
        {QStringLiteral("exitCode"), result.exitCode},
        {QStringLiteral("stdout"), result.out.trimmed()},
        {QStringLiteral("stderr"), result.err},
        {QStringLiteral("timedOut"), result.timedOut},
    };
}

QStringList fields(const QString &line) {
    return line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

} // namespace

QJsonArray SystemdManager::collect(const QString &scope, QString *error) const {
    if (!validScope(scope)) {
        if (error) *error = QStringLiteral("Invalid systemd scope");
        return {};
    }

    struct Unit {
        QString name;
        QString description;
        QString loadState;
        QString activeState;
        QString subState;
        QString fileState;
    };

    QHash<QString, Unit> units;

    QStringList fileArgs = scopeArgs(scope);
    fileArgs << QStringLiteral("list-unit-files")
             << QStringLiteral("--type=service")
             << QStringLiteral("--no-legend")
             << QStringLiteral("--no-pager")
             << QStringLiteral("--plain");

    const CommandResult fileResult = runCommand(QStringLiteral("systemctl"), fileArgs);
    if (fileResult.exitCode != 0 && error)
        *error = fileResult.err;

    for (const QString &line : fileResult.out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList parts = fields(line);
        if (parts.size() < 2)
            continue;

        Unit unit;
        unit.name = parts.at(0);
        unit.fileState = parts.at(1);
        units.insert(unit.name, unit);
    }

    QStringList unitArgs = scopeArgs(scope);
    unitArgs << QStringLiteral("list-units")
             << QStringLiteral("--type=service")
             << QStringLiteral("--all")
             << QStringLiteral("--no-legend")
             << QStringLiteral("--no-pager")
             << QStringLiteral("--plain");

    const CommandResult loaded = runCommand(QStringLiteral("systemctl"), unitArgs);
    if (loaded.exitCode != 0 && error && error->isEmpty())
        *error = loaded.err;

    for (const QString &line : loaded.out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList parts = fields(line);
        if (parts.size() < 4)
            continue;

        Unit unit = units.value(parts.at(0));
        unit.name = parts.at(0);
        unit.loadState = parts.at(1);
        unit.activeState = parts.at(2);
        unit.subState = parts.at(3);

        if (parts.size() > 4)
            unit.description = parts.mid(4).join(QLatin1Char(' '));

        units.insert(unit.name, unit);
    }

    QJsonArray result;
    QStringList names = units.keys();
    std::sort(names.begin(), names.end(), [](const QString &a, const QString &b) {
        return QString::compare(a, b, Qt::CaseInsensitive) < 0;
    });

    for (const QString &name : names) {
        const Unit &unit = units[name];
        result.append(QJsonObject{
            {QStringLiteral("name"), unit.name},
            {QStringLiteral("description"), unit.description},
            {QStringLiteral("scope"), scope},
            {QStringLiteral("loadState"), unit.loadState},
            {QStringLiteral("activeState"), unit.activeState},
            {QStringLiteral("subState"), unit.subState},
            {QStringLiteral("fileState"), unit.fileState},
        });
    }

    return result;
}

QJsonArray SystemdManager::list(const QString &scope, const QString &query, int limit,
                                QString *error) const {
    QJsonArray input;

    if (scope == QStringLiteral("all")) {
        QString userError;
        QString systemError;
        const QJsonArray user = collect(QStringLiteral("user"), &userError);
        const QJsonArray system = collect(QStringLiteral("system"), &systemError);
        for (const QJsonValue &value : user) input.append(value);
        for (const QJsonValue &value : system) input.append(value);
        if (error && user.isEmpty() && system.isEmpty())
            *error = userError + QLatin1Char(' ') + systemError;
    } else {
        input = collect(scope, error);
    }

    struct Ranked {
        QJsonObject object;
        int score = 0;
    };

    QVector<Ranked> ranked;
    ranked.reserve(input.size());

    for (const QJsonValue &value : input) {
        QJsonObject object = value.toObject();
        if (query.trimmed().isEmpty()) {
            ranked.push_back({object, 0});
            continue;
        }

        const QString searchable =
            object.value(QStringLiteral("name")).toString() + QLatin1Char(' ') +
            object.value(QStringLiteral("description")).toString();

        const auto score = FuzzyMatcher::score(query, searchable);
        if (!score.has_value())
            continue;

        object.insert(QStringLiteral("score"), *score);
        ranked.push_back({object, *score});
    }

    std::stable_sort(ranked.begin(), ranked.end(), [](const Ranked &a, const Ranked &b) {
        if (a.score != b.score)
            return a.score > b.score;
        return a.object.value(QStringLiteral("name")).toString()
             < b.object.value(QStringLiteral("name")).toString();
    });

    QJsonArray output;
    const int safeLimit = std::clamp(limit, 1, 500);
    for (int i = 0; i < ranked.size() && i < safeLimit; ++i)
        output.append(ranked.at(i).object);
    return output;
}

QJsonObject SystemdManager::action(const QString &scope, const QString &verb,
                                   const QString &unit) const {
    if (!validScope(scope))
        return {{QStringLiteral("ok"), false},
                {QStringLiteral("stderr"), QStringLiteral("Invalid scope")}};

    if (!validation::serviceAction(verb))
        return {{QStringLiteral("ok"), false},
                {QStringLiteral("stderr"), QStringLiteral("Invalid action")}};

    if (!validation::serviceUnit(unit))
        return {{QStringLiteral("ok"), false},
                {QStringLiteral("stderr"), QStringLiteral("Invalid service unit")}};

    // No shell anywhere in this path. Each value is a separate argv element.
    if (scope == QStringLiteral("system")) {
        return resultJson(runCommand(
            QStringLiteral("pkexec"),
            {QStringLiteral("systemctl"), verb, unit},
            120000));
    }

    return resultJson(runCommand(
        QStringLiteral("systemctl"),
        {QStringLiteral("--user"), verb, unit},
        15000));
}

QJsonObject SystemdManager::inspect(const QString &scope, const QString &unit) const {
    if (!validScope(scope) || !validation::serviceUnit(unit)) {
        return {{QStringLiteral("ok"), false},
                {QStringLiteral("stderr"), QStringLiteral("Invalid service selection")}};
    }

    QStringList args = scopeArgs(scope);
    args << QStringLiteral("show")
         << unit
         << QStringLiteral("--no-pager")
         << QStringLiteral("--property=Id,Description,LoadState,ActiveState,SubState,UnitFileState,FragmentPath");

    const CommandResult result = runCommand(QStringLiteral("systemctl"), args);
    QJsonObject object = resultJson(result);

    QJsonObject properties;
    for (const QString &line : result.out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const int equals = line.indexOf(QLatin1Char('='));
        if (equals <= 0)
            continue;
        properties.insert(line.left(equals), line.mid(equals + 1));
    }
    object.insert(QStringLiteral("properties"), properties);
    return object;
}

} // namespace vorssaint
