#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

class HelperSplit {
public:
    // Function to split a vector into smaller vectors
    static inline std::vector<std::vector<std::string>>
    splitVector(const std::vector<std::string>& strings, int n)
    {
        std::vector<std::vector<std::string>> out;

        if (n <= 0) return out;
        out.reserve(static_cast<std::size_t>(n));

        const std::size_t total = strings.size();
        const std::size_t base  = total / static_cast<std::size_t>(n);
        const std::size_t rem   = total % static_cast<std::size_t>(n);

        std::size_t index = 0;
        for (int i = 0; i < n; ++i) {
            const std::size_t take = base + (static_cast<std::size_t>(i) < rem ? 1u : 0u);

            // If n > total, some will be empty vectors. Keep behavior stable.
            std::vector<std::string> chunk;
            chunk.reserve(take);

            const std::size_t end = std::min(index + take, total);
            for (std::size_t j = index; j < end; ++j) {
                chunk.push_back(strings[j]);
            }
            out.push_back(std::move(chunk));
            index = end;
        }
        return out;
    }

    // Existing API (string delimiter) preserved
    static inline std::vector<std::string>
    splitString(const std::string& s, const std::string& delimiter)
    {
        // Keep semantics identical to your old code for edge cases.
        if (delimiter.empty()) return {s};

        std::vector<std::string> tokens;

        // Heuristic reserve: for 1-char delimiter we can approximate count by scanning.
        if (delimiter.size() == 1) {
            const char d = delimiter[0];
            std::size_t count = 1;
            for (char c : s) if (c == d) ++count;
            tokens.reserve(count);
        }

        std::size_t start = 0;
        while (true) {
            const std::size_t end = s.find(delimiter, start);
            if (end == std::string::npos) break;
            tokens.emplace_back(s.substr(start, end - start));
            start = end + delimiter.size();
        }
        tokens.emplace_back(s.substr(start));
        return tokens;
    }

    // OPTIONAL: faster overload (no allocations until final tokens)
    // Use in new code if you want (no downstream breakage).
    static inline std::vector<std::string>
    splitString(std::string_view s, std::string_view delimiter)
    {
        if (delimiter.empty()) return {std::string(s)};

        std::vector<std::string> tokens;

        if (delimiter.size() == 1) {
            const char d = delimiter[0];
            std::size_t count = 1;
            for (char c : s) if (c == d) ++count;
            tokens.reserve(count);
        }

        std::size_t start = 0;
        while (true) {
            const std::size_t end = s.find(delimiter, start);
            if (end == std::string_view::npos) break;
            tokens.emplace_back(s.substr(start, end - start));
            start = end + delimiter.size();
        }
        tokens.emplace_back(s.substr(start));
        return tokens;
    }
};

