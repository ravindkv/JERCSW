#include "Logger.h"

// ANSI color codes for different log levels
const std::string COLOR_DEBUG = "\033[36m";  // Cyan
const std::string COLOR_INFO = "\033[32m";   // Green
const std::string COLOR_WARNING = "\033[33m"; // Yellow
const std::string COLOR_ERROR = "\033[31m";   // Red
const std::string COLOR_CRITICAL = "\033[1;31m"; // Bold Red
const std::string COLOR_RESET = "\033[0m";   // Reset color

// Initialize the global console log level to INFO
LogLevel Logger::consoleLogLevel = INFO;

// Singleton instance of Logger
Logger* Logger::instance = nullptr;

// Get the singleton instance of the logger
auto Logger::getInstance() -> Logger& {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return *instance;
}

// Initialize the logger with a file name
void Logger::init(const std::string& filename)
{
    if (!logFile.is_open()) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error opening log file." << '\n';
        }
    }
}

// Sets the console log level to filter console output
void Logger::setConsoleLogLevel(LogLevel level) {
    consoleLogLevel = level;
}

// General log function that handles all levels
void Logger::log(LogLevel level, const std::string& message)
{
    // Get current timestamp
    time_t now = time(nullptr);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Create log entry
    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] " << levelToString(level) << ": " << message << '\n';

    // Output to console only if the log level meets or exceeds the console log level
    if (level >= consoleLogLevel) {
        std::cout << getColorCode(level) << logEntry.str() << resetColor();
    }

    // Output to log file for all levels
    if (logFile.is_open()) {
        logFile << logEntry.str();
        logFile.flush(); // Ensure immediate write to file
    }
}

// Logging functions for specific severity levels
void Logger::LogDebug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::LogInfo(const std::string& message) {
    log(INFO, message);
}

void Logger::LogWarning(const std::string& message) {
    log(WARNING, message);
}

void Logger::LogError(const std::string& message) {
    log(ERROR, message);
}

void Logger::LogCritical(const std::string& message) {
    log(CRITICAL, message);
}

// Converts log level to a string for output
auto Logger::levelToString(LogLevel level) -> std::string
{
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

// Returns the appropriate ANSI color code based on the log level
auto Logger::getColorCode(LogLevel level) -> std::string
{
    switch (level) {
        case DEBUG:
            return COLOR_DEBUG;
        case INFO:
            return COLOR_INFO;
        case WARNING:
            return COLOR_WARNING;
        case ERROR:
            return COLOR_ERROR;
        case CRITICAL:
            return COLOR_CRITICAL;
        default:
            return COLOR_RESET;
    }
}

// Resets the console color
auto Logger::resetColor() -> std::string {
    return COLOR_RESET;
}

