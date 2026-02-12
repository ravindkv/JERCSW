#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>

// Enum to represent log levels
enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
public:
    // Get the singleton instance of the logger
    static Logger& getInstance();

    // Logging functions for different severity levels
    void LogDebug(const std::string& message);
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);
    void LogCritical(const std::string& message);

    // Set the console log level (controls what gets printed to console)
    static void setConsoleLogLevel(LogLevel level);

    // Initialize the logger with a file name (called once in main)
    void init(const std::string& filename);

private:
    std::ofstream logFile;  // File stream for the log file
    static LogLevel consoleLogLevel;  // Global console log level
    static Logger* instance;  // Singleton instance

    // Private constructor to enforce singleton pattern
    Logger() {}

    // General logging function
    void log(LogLevel level, const std::string& message);

    // Converts log level to a string for output
    std::string levelToString(LogLevel level);

    // Returns ANSI color code based on log level
    std::string getColorCode(LogLevel level);

    // Resets ANSI color
    std::string resetColor();
};

#endif // LOGGER_H

