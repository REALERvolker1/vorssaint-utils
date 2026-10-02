#pragma once

#include <QJsonObject>

namespace vorssaint {

class SystemMetrics {
public:
    static QJsonObject snapshot();
};

} // namespace vorssaint
