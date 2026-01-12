/*
** R-Type - High Score Manager
** Manages high score persistence with JSON format
*/

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace rtype {

    /**
     * @brief A single high score entry
     */
    struct ScoreEntry {
        std::string name;       // Player name (3 characters typically)
        int score;              // Score value
        std::string date;       // Date achieved (YYYY-MM-DD format)

        ScoreEntry() : score(0) {}
        ScoreEntry(const std::string& n, int s, const std::string& d = "")
            : name(n), score(s), date(d) {
            if (date.empty()) {
                date = getCurrentDate();
            }
        }

        static std::string getCurrentDate() {
            std::time_t now = std::time(nullptr);
            std::tm* tm = std::localtime(&now);
            std::ostringstream oss;
            oss << std::put_time(tm, "%Y-%m-%d");
            return oss.str();
        }

        bool operator<(const ScoreEntry& other) const {
            return score > other.score;  // Higher score first
        }
    };

    /**
     * @brief Manages high score storage and retrieval
     * 
     * Handles saving/loading high scores to/from a JSON file.
     * Maintains a sorted list of top scores (default max 10).
     */
    class HighScoreManager {
    public:
        static constexpr int DEFAULT_MAX_SCORES = 10;

        /**
         * @brief Construct a new High Score Manager
         * @param maxScores Maximum number of high scores to keep
         */
        explicit HighScoreManager(int maxScores = DEFAULT_MAX_SCORES)
            : m_maxScores(maxScores), m_dirty(false) {}

        /**
         * @brief Load high scores from a JSON file
         * @param path Path to the high scores file
         * @return true if loaded successfully
         */
        bool load(const std::string& path) {
            m_filePath = path;
            m_scores.clear();

            std::ifstream file(path);
            if (!file.is_open()) {
                return false;  // File doesn't exist yet - that's OK
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
            file.close();

            return parseJson(content);
        }

        /**
         * @brief Save high scores to a JSON file
         * @param path Path to save to (empty = use load path)
         * @return true if saved successfully
         */
        bool save(const std::string& path = "") {
            std::string savePath = path.empty() ? m_filePath : path;
            if (savePath.empty()) {
                savePath = "highscores.json";
            }

            std::ofstream file(savePath);
            if (!file.is_open()) {
                return false;
            }

            file << toJson();
            file.close();
            m_dirty = false;
            return true;
        }

        /**
         * @brief Check if a score qualifies as a high score
         * @param score The score to check
         * @return true if score would make the list
         */
        bool isHighScore(int score) const {
            if (m_scores.size() < static_cast<size_t>(m_maxScores)) {
                return true;
            }
            return score > m_scores.back().score;
        }

        /**
         * @brief Add a new high score entry
         * @param name Player name
         * @param score Score value
         * @return Rank position (1-based), or 0 if not added
         */
        int addScore(const std::string& name, int score) {
            if (!isHighScore(score)) {
                return 0;
            }

            ScoreEntry entry(name, score);
            m_scores.push_back(entry);
            std::sort(m_scores.begin(), m_scores.end());

            // Trim to max size
            if (m_scores.size() > static_cast<size_t>(m_maxScores)) {
                m_scores.resize(m_maxScores);
            }

            m_dirty = true;

            // Find rank
            for (size_t i = 0; i < m_scores.size(); ++i) {
                if (m_scores[i].score == score && m_scores[i].name == name) {
                    return static_cast<int>(i) + 1;
                }
            }
            return 0;
        }

        /**
         * @brief Get all high score entries
         * @return Reference to sorted score list
         */
        const std::vector<ScoreEntry>& getScores() const {
            return m_scores;
        }

        /**
         * @brief Get highest score
         * @return Highest score, or 0 if no scores
         */
        int getHighScore() const {
            return m_scores.empty() ? 0 : m_scores.front().score;
        }

        /**
         * @brief Check if there are unsaved changes
         * @return true if dirty
         */
        bool isDirty() const {
            return m_dirty;
        }

        /**
         * @brief Clear all scores
         */
        void clear() {
            m_scores.clear();
            m_dirty = true;
        }

    private:
        std::vector<ScoreEntry> m_scores;
        std::string m_filePath;
        int m_maxScores;
        bool m_dirty;

        /**
         * @brief Parse JSON content (simple parser)
         */
        bool parseJson(const std::string& content) {
            // Simple JSON parser for our format
            // Format: {"highscores":[{"name":"AAA","score":10000,"date":"2025-01-08"},...]
            
            size_t pos = content.find("\"highscores\"");
            if (pos == std::string::npos) return false;

            pos = content.find('[', pos);
            if (pos == std::string::npos) return false;

            size_t end = content.find(']', pos);
            if (end == std::string::npos) return false;

            std::string arrayContent = content.substr(pos + 1, end - pos - 1);

            // Parse each entry
            size_t entryStart = 0;
            while ((entryStart = arrayContent.find('{', entryStart)) != std::string::npos) {
                size_t entryEnd = arrayContent.find('}', entryStart);
                if (entryEnd == std::string::npos) break;

                std::string entryStr = arrayContent.substr(entryStart, entryEnd - entryStart + 1);
                
                ScoreEntry entry;
                entry.name = extractString(entryStr, "name");
                entry.score = extractInt(entryStr, "score");
                entry.date = extractString(entryStr, "date");

                if (!entry.name.empty() && entry.score > 0) {
                    m_scores.push_back(entry);
                }

                entryStart = entryEnd + 1;
            }

            std::sort(m_scores.begin(), m_scores.end());
            return true;
        }

        /**
         * @brief Extract a string value from JSON
         */
        std::string extractString(const std::string& json, const std::string& key) {
            std::string searchKey = "\"" + key + "\"";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return "";

            pos = json.find(':', pos);
            if (pos == std::string::npos) return "";

            pos = json.find('"', pos + 1);
            if (pos == std::string::npos) return "";

            size_t end = json.find('"', pos + 1);
            if (end == std::string::npos) return "";

            return json.substr(pos + 1, end - pos - 1);
        }

        /**
         * @brief Extract an integer value from JSON
         */
        int extractInt(const std::string& json, const std::string& key) {
            std::string searchKey = "\"" + key + "\"";
            size_t pos = json.find(searchKey);
            if (pos == std::string::npos) return 0;

            pos = json.find(':', pos);
            if (pos == std::string::npos) return 0;

            // Skip whitespace
            pos++;
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

            // Parse integer
            int value = 0;
            while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
                value = value * 10 + (json[pos] - '0');
                pos++;
            }
            return value;
        }

        /**
         * @brief Convert scores to JSON string
         */
        std::string toJson() const {
            std::ostringstream oss;
            oss << "{\n  \"highscores\": [\n";

            for (size_t i = 0; i < m_scores.size(); ++i) {
                const auto& entry = m_scores[i];
                oss << "    {\"name\": \"" << entry.name 
                    << "\", \"score\": " << entry.score 
                    << ", \"date\": \"" << entry.date << "\"}";
                if (i < m_scores.size() - 1) oss << ",";
                oss << "\n";
            }

            oss << "  ]\n}\n";
            return oss.str();
        }
    };

} // namespace rtype
