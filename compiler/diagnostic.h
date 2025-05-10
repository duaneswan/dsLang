/**
 * diagnostic.h - Diagnostic Reporting for dsLang
 * 
 * This file defines the DiagnosticReporter class which is responsible for
 * collecting and reporting errors and warnings during compilation.
 */

#ifndef DSLANG_DIAGNOSTIC_H
#define DSLANG_DIAGNOSTIC_H

#include "token.h"
#include <string>
#include <vector>
#include <iostream>

namespace dsLang {

/**
 * Diagnostic - Represents a single diagnostic message
 */
class Diagnostic {
public:
    enum class Level {
        ERROR,
        WARNING,
        NOTE
    };
    
    /**
     * Constructor
     * 
     * @param level The diagnostic level (error, warning, note)
     * @param message The diagnostic message
     * @param filename The source file name
     * @param line The line number
     * @param column The column number
     */
    Diagnostic(Level level, const std::string& message, 
               const std::string& filename, unsigned line, unsigned column)
        : level_(level), message_(message), 
          filename_(filename), line_(line), column_(column) {}
    
    /**
     * GetLevel - Get the diagnostic level
     * 
     * @return The diagnostic level
     */
    Level GetLevel() const { return level_; }
    
    /**
     * GetMessage - Get the diagnostic message
     * 
     * @return The diagnostic message
     */
    const std::string& GetMessage() const { return message_; }
    
    /**
     * GetFilename - Get the source file name
     * 
     * @return The source file name
     */
    const std::string& GetFilename() const { return filename_; }
    
    /**
     * GetLine - Get the line number
     * 
     * @return The line number
     */
    unsigned GetLine() const { return line_; }
    
    /**
     * GetColumn - Get the column number
     * 
     * @return The column number
     */
    unsigned GetColumn() const { return column_; }
    
    /**
     * ToString - Convert the diagnostic to a string
     * 
     * @return The string representation of the diagnostic
     */
    std::string ToString() const;
    
private:
    Level level_;
    std::string message_;
    std::string filename_;
    unsigned line_;
    unsigned column_;
};

/**
 * DiagnosticReporter - Collects and reports diagnostics
 */
class DiagnosticReporter {
public:
    /**
     * Constructor
     */
    DiagnosticReporter() : error_count_(0), warning_count_(0) {}
    
    /**
     * Report - Report a diagnostic
     * 
     * @param level The diagnostic level
     * @param message The diagnostic message
     * @param filename The source file name
     * @param line The line number
     * @param column The column number
     */
    void Report(Diagnostic::Level level, const std::string& message,
                const std::string& filename, unsigned line, unsigned column);
    
    /**
     * ReportError - Report an error
     * 
     * @param message The error message
     * @param filename The source file name
     * @param line The line number
     * @param column The column number
     */
    void ReportError(const std::string& message,
                     const std::string& filename, unsigned line, unsigned column);
    
    /**
     * ReportError - Report an error at a token
     * 
     * @param message The error message
     * @param token The token where the error occurred
     * @param filename The source file name
     */
    void ReportError(const std::string& message, const Token& token, const std::string& filename);
    
    /**
     * ReportWarning - Report a warning
     * 
     * @param message The warning message
     * @param filename The source file name
     * @param line The line number
     * @param column The column number
     */
    void ReportWarning(const std::string& message,
                       const std::string& filename, unsigned line, unsigned column);
    
    /**
     * ReportWarning - Report a warning at a token
     * 
     * @param message The warning message
     * @param token The token where the warning occurred
     * @param filename The source file name
     */
    void ReportWarning(const std::string& message, const Token& token, const std::string& filename);
    
    /**
     * HasErrors - Check if any errors were reported
     * 
     * @return True if errors were reported, false otherwise
     */
    bool HasErrors() const { return error_count_ > 0; }
    
    /**
     * GetErrorCount - Get the number of errors reported
     * 
     * @return The number of errors
     */
    unsigned GetErrorCount() const { return error_count_; }
    
    /**
     * GetWarningCount - Get the number of warnings reported
     * 
     * @return The number of warnings
     */
    unsigned GetWarningCount() const { return warning_count_; }
    
    /**
     * GetDiagnostics - Get all diagnostics
     * 
     * @return The diagnostics
     */
    const std::vector<Diagnostic>& GetDiagnostics() const { return diagnostics_; }
    
    /**
     * PrintDiagnostics - Print all diagnostics to the specified output stream
     * 
     * @param os The output stream to print to
     */
    void PrintDiagnostics(std::ostream& os = std::cerr) const;
    
private:
    std::vector<Diagnostic> diagnostics_;
    unsigned error_count_;
    unsigned warning_count_;
};

} // namespace dsLang

#endif // DSLANG_DIAGNOSTIC_H
