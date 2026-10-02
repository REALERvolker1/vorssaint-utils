#pragma once

#include <QStringView>
#include <optional>

namespace vorssaint {

// fzf-style dynamic-programming scorer. It rewards exact substrings,
// consecutive matches, word/path boundaries, and camelCase boundaries,
// while penalizing gaps and late starts.
class FuzzyMatcher {
public:
    static std::optional<int> score(QStringView needle, QStringView haystack);
};

} // namespace vorssaint
