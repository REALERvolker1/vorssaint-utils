#pragma once

#include <QString>

namespace vorssaint::validation {

bool serviceUnit(const QString &unit);
bool serviceAction(const QString &action);
bool monitorName(const QString &monitor);
bool windowAddress(const QString &address);
bool workspaceId(const QString &text, int *value = nullptr);

} // namespace vorssaint::validation
