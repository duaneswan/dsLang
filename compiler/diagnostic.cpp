/**
 * diagnostic.cpp - Diagnostic Reporting Implementation for dsLang
 * 
 * This file implements the DiagnosticReporter class which is responsible for
 * collecting and reporting errors and warnings during compilation.
 */

#include "diagnostic.h"
#include <sstream>

namespace dsLang {

/**
 * ToString - Convert the diagnostic to a string
 */
std::string Diagnostic::ToString() const {
    std::ostringstream oss;
    
    // Format: filename:line:column: level: message
    oss << filename_ << ":" << line_ << ":" << column_ << ": ";
    
    switch (level_) {
        case Level::ERROR:
            oss << "error: ";
            break;
        case Level::WARNING:
            oss << "warning: ";
            break;
        case Level::NOTE:
            oss << "note: ";
            break;
    }
    
    oss << message_;
    
    return oss.str();
}

/**
 * Report - Report a diagnostic
 */
void DiagnosticReporter::Report(Diagnostic::Level level, const std::string& message,
                                const std::string& filename, unsigned line, unsigned column) {
    diagnostics_.emplace_back(level, message, filename, line, column);
    
    if (level == Diagnostic::Level::ERROR) {
        error_count_++;
    } else if (level == Diagnostic::Level::WARNING) {
        warning_count_++;
    }
    
    // Immediately print the diagnostic to stderr
    std::cerr << diagnostics_.back().ToString() << std::endl;
}

/**
 * ReportError - Report an error
 */
void DiagnosticReporter::ReportError(const std::string& message,
                                    const std::string& filename, unsigned line, unsigned column) {
    Report(Diagnostic::Level::ERROR, message, filename, line, column);
}

/**
 * ReportError - Report an error at a token
 */
void DiagnosticReporter::ReportError(const std::string& message, const Token& token,
                                     const std::string& filename) {
    ReportError(message, filename, token.GetLine(), token.GetColumn());
    
    // If the token has some lexeme text, show it in the error
    if (!token.GetLexeme().empty()) {
        std::string token_text = "'" + token.GetLexeme() + "'";
        Report(Diagnostic::Level::NOTE, "token text: " + token_text, 
               filename, token.GetLine(), token.GetColumn());
    }
}

/**
 * ReportWarning - Report a warning
 */
void DiagnosticReporter::ReportWarning(const std::string& message,
                                       const std::string& filename, unsigned line, unsigned column) {
    Report(Diagnostic::Level::WARNING, message, filename, line, column);
}

/**
 * ReportWarning - Report a warning at a token
 */
void DiagnosticReporter::ReportWarning(const std::string& message, const Token& token,
                                       const std::string& filename) {
    ReportWarning(message, filename, token.GetLine(), token.GetColumn());
}

/**
 * PrintDiagnostics - Print all diagnostics to the specified output stream
 */
void DiagnosticReporter::PrintDiagnostics(std::ostream& os) const {
    for (const auto& diagnostic : diagnostics_) {
        os << diagnostic.ToString() << std::endl;
    }
    
    // Print summary
    if (error_count_ > 0 || warning_count_ > 0) {
        os << "\n";
        if (error_count_ > 0) {
            os << error_count_ << " error" << (error_count_ > 1 ? "s" : "");
            if (warning_count_ > 0) {
                os << " and ";
            }
        }
        
        if (warning_count_ > 0) {
            os << warning_count_ << " warning" << (warning_count_ > 1 ? "s" : "");
        }
        
        os << " generated." << std::endl;
    }
}

} // namespace dsLang
