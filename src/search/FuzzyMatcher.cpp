#include "search/FuzzyMatcher.hpp"

#include <QChar>
#include <QString>
#include <QVector>

#include <algorithm>
#include <limits>

namespace vorssaint {
namespace {

constexpr int kNegInf = std::numeric_limits<int>::min() / 8;

bool separator(QChar c) {
    return c == QLatin1Char('/') || c == QLatin1Char('\\') ||
           c == QLatin1Char('_') || c == QLatin1Char('-') ||
           c == QLatin1Char('.') || c == QLatin1Char(' ') ||
           c == QLatin1Char(':');
}

int positionBonus(const QString &original, int index) {
    if (index == 0)
        return 32;

    const QChar prev = original.at(index - 1);
    const QChar cur = original.at(index);

    if (separator(prev))
        return 28;
    if (!prev.isLetterOrNumber() && cur.isLetterOrNumber())
        return 26;
    if (prev.isLower() && cur.isUpper())
        return 22;
    if (prev.isDigit() != cur.isDigit())
        return 8;
    return 0;
}

} // namespace

std::optional<int> FuzzyMatcher::score(QStringView needleView, QStringView haystackView) {
    const QString needleOriginal = needleView.toString();
    const QString hayOriginal = haystackView.toString();

    if (needleOriginal.isEmpty())
        return 0;
    if (hayOriginal.isEmpty() || needleOriginal.size() > hayOriginal.size())
        return std::nullopt;

    const QString needle = needleOriginal.toCaseFolded();
    const QString hay = hayOriginal.toCaseFolded();

    // fzf v2-style DP: each state is the best score when needle[i]
    // ends at hay[j]. This deliberately optimizes quality over O(n) speed;
    // service lists are small enough that O(query * candidate^2) is fine.
    QVector<int> prev(hay.size(), kNegInf);
    QVector<int> cur(hay.size(), kNegInf);

    for (int j = 0; j < hay.size(); ++j) {
        if (needle.at(0) != hay.at(j))
            continue;
        prev[j] = 40 + positionBonus(hayOriginal, j) - std::min(j * 2, 40);
    }

    for (int i = 1; i < needle.size(); ++i) {
        std::fill(cur.begin(), cur.end(), kNegInf);

        for (int j = i; j < hay.size(); ++j) {
            if (needle.at(i) != hay.at(j))
                continue;

            int best = kNegInf;
            for (int k = i - 1; k < j; ++k) {
                if (prev[k] == kNegInf)
                    continue;

                const int gap = j - k - 1;
                int transition = prev[k] + 40 + positionBonus(hayOriginal, j);

                if (gap == 0)
                    transition += 42;
                else
                    transition -= 6 + (gap * 3);

                best = std::max(best, transition);
            }
            cur[j] = best;
        }
        prev.swap(cur);
    }

    int best = *std::max_element(prev.cbegin(), prev.cend());
    if (best == kNegInf)
        return std::nullopt;

    const int exact = hay.indexOf(needle);
    if (exact >= 0) {
        best += 220 - std::min(exact * 4, 120);
        if (exact == 0)
            best += 100;
    }

    // Prefer concise candidates when the match quality is otherwise similar.
    best -= std::max(0, hay.size() - needle.size()) / 2;
    return best;
}

} // namespace vorssaint
