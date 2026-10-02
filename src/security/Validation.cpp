#include "security/Validation.hpp"

#include <QRegularExpression>
#include <QStringList>

namespace vorssaint::validation {

bool serviceUnit(const QString &unit) {
    static const QRegularExpression re(
        QStringLiteral(R"(^[A-Za-z0-9_.@:\x2d]+\.service$)"));
    return unit.size() <= 256 && re.match(unit).hasMatch();
}

bool serviceAction(const QString &action) {
    static const QStringList allowed = {
        QStringLiteral("start"),
        QStringLiteral("stop"),
        QStringLiteral("restart"),
        QStringLiteral("reload"),
        QStringLiteral("enable"),
        QStringLiteral("disable"),
        QStringLiteral("mask"),
        QStringLiteral("unmask"),
        QStringLiteral("reset-failed"),
    };
    return allowed.contains(action);
}

bool monitorName(const QString &monitor) {
    static const QRegularExpression re(QStringLiteral(R"(^[A-Za-z0-9_.:\x2d]+$)"));
    return !monitor.isEmpty() && monitor.size() <= 128 && re.match(monitor).hasMatch();
}

bool windowAddress(const QString &address) {
    static const QRegularExpression re(QStringLiteral(R"(^0x[0-9A-Fa-f]+$)"));
    return address.size() <= 34 && re.match(address).hasMatch();
}

bool workspaceId(const QString &text, int *value) {
    static const QRegularExpression re(QStringLiteral(R"(^[1-9][0-9]{0,3}$)"));
    if (!re.match(text).hasMatch())
        return false;

    bool ok = false;
    const int parsed = text.toInt(&ok);
    if (!ok || parsed < 1 || parsed > 9999)
        return false;

    if (value) *value = parsed;
    return true;
}

} // namespace vorssaint::validation
