#include "search/FuzzyMatcher.hpp"
#include "security/Validation.hpp"

#include <QCoreApplication>

#include <cassert>

using vorssaint::FuzzyMatcher;

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    const auto exact =
        FuzzyMatcher::score(QStringLiteral("ssh"), QStringLiteral("sshd.service"));
    const auto sparse =
        FuzzyMatcher::score(QStringLiteral("ssh"),
                            QStringLiteral("some-super-huge.service"));
    const auto boundary =
        FuzzyMatcher::score(QStringLiteral("nm"),
                            QStringLiteral("NetworkManager.service"));
    const auto miss =
        FuzzyMatcher::score(QStringLiteral("zzzz"),
                            QStringLiteral("sshd.service"));

    assert(exact.has_value());
    assert(sparse.has_value());
    assert(boundary.has_value());
    assert(!miss.has_value());
    assert(*exact > *sparse);

    using namespace vorssaint::validation;

    assert(serviceUnit(QStringLiteral("sshd.service")));
    assert(serviceUnit(QStringLiteral("user@1000.service")));
    assert(!serviceUnit(QStringLiteral("sshd.service;reboot")));
    assert(!serviceUnit(QStringLiteral("$(reboot).service")));
    assert(!serviceUnit(QStringLiteral("evil service.service")));
    assert(!serviceUnit(QStringLiteral("evil\n.service")));

    assert(serviceAction(QStringLiteral("restart")));
    assert(!serviceAction(QStringLiteral("restart;reboot")));

    assert(monitorName(QStringLiteral("DP-1")));
    assert(!monitorName(QStringLiteral("DP-1;reboot")));
    assert(!monitorName(QStringLiteral("DP-1 $(id)")));

    int workspace = 0;
    assert(workspaceId(QStringLiteral("42"), &workspace) && workspace == 42);
    assert(!workspaceId(QStringLiteral("0")));
    assert(!workspaceId(QStringLiteral("1;reboot")));
    assert(!workspaceId(QStringLiteral("$(id)")));
    assert(!workspaceId(QStringLiteral("1 2")));

    assert(windowAddress(QStringLiteral("0xdeadBEEF")));
    assert(!windowAddress(QStringLiteral("0xdeadbeef,exec,reboot")));

    return 0;
}
