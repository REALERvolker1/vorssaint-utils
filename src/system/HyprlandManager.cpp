#include "system/HyprlandManager.hpp"

#include "security/Validation.hpp"

#include <QProcess>

namespace vorssaint {
namespace {

QJsonObject runHyprctl(const QStringList &arguments) {
    QProcess process;
    process.setProgram(QStringLiteral("hyprctl"));
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted(2000)) {
        return {
            {QStringLiteral("ok"), false},
            {QStringLiteral("stderr"), process.errorString()},
        };
    }

    if (!process.waitForFinished(7000)) {
        process.kill();
        process.waitForFinished();
        return {
            {QStringLiteral("ok"), false},
            {QStringLiteral("stderr"), QStringLiteral("hyprctl timed out")},
        };
    }

    return {
        {QStringLiteral("ok"), process.exitCode() == 0},
        {QStringLiteral("exitCode"), process.exitCode()},
        {QStringLiteral("stdout"),
         QString::fromUtf8(process.readAllStandardOutput()).trimmed()},
        {QStringLiteral("stderr"),
         QString::fromUtf8(process.readAllStandardError()).trimmed()},
    };
}

} // namespace

QJsonObject HyprlandManager::openWorkspace(const QString &monitor,
                                           const QString &workspace) const {
    int workspaceNumber = 0;
    if (!validation::monitorName(monitor) ||
        !validation::workspaceId(workspace, &workspaceNumber)) {
        return {
            {QStringLiteral("ok"), false},
            {QStringLiteral("stderr"), QStringLiteral("Invalid monitor or workspace")},
        };
    }

    // argv only. monitor/workspace are validated before reaching Hyprland.
    QJsonObject focus = runHyprctl(
        {QStringLiteral("dispatch"), QStringLiteral("focusmonitor"), monitor});
    if (!focus.value(QStringLiteral("ok")).toBool())
        return focus;

    return runHyprctl({
        QStringLiteral("dispatch"),
        QStringLiteral("workspace"),
        QString::number(workspaceNumber),
    });
}

QJsonObject HyprlandManager::moveActiveToWorkspace(const QString &workspace) const {
    int workspaceNumber = 0;
    if (!validation::workspaceId(workspace, &workspaceNumber)) {
        return {
            {QStringLiteral("ok"), false},
            {QStringLiteral("stderr"), QStringLiteral("Invalid workspace")},
        };
    }

    return runHyprctl({
        QStringLiteral("dispatch"),
        QStringLiteral("movetoworkspace"),
        QString::number(workspaceNumber),
    });
}

} // namespace vorssaint
