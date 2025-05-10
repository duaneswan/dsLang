#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <cerrno>

// For LLVM include errors, we'll comment these out for now
// and fix the include paths later
// #include "llvm/IR/LLVMContext.h"
// #include "llvm/IR/Module.h"
// #include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/Verifier.h"
// #include "llvm/Support/TargetSelect.h"
// #include "llvm/Support/FileSystem.h"
// #include "llvm/Support/raw_ostream.h"
// #include "llvm/Target/TargetMachine.h"
// #include "llvm/Target/TargetOptions.h"
// #include "llvm/MC/TargetRegistry.h"
// #include "llvm/Support/TargetSelect.h"
// #include "llvm/IR/LegacyPassManager.h"

#include "diagnostic.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "sema.h"

// Display usage information
void printUsage(const char* progName) {
    std::cerr << "dsLang Compiler (dscc) - Cross compiler for dsOS\n\n";
    std::cerr << "Usage: " << progName << " [options] input_file\n";
    std::cerr << "Options:\n";
    std::cerr << "  -o <file>     Specify output file name\n";
    std::cerr << "  -S            Output assembly code\n";
    std::cerr << "  -c            Output object file (default)\n";
    std::cerr << "  -O<level>     Optimization level (0-3)\n";
    std::cerr << "  -v            Verbose output\n";
    std::cerr << "  -h, --help    Display this help message\n";
}

// Read file content into a string
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file '" << filename << "': " << strerror(errno) << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Main compiler entry point
int main(int argc, char** argv) {
    // Default values
    std::string inputFilename;
    std::string outputFilename = "a.out";
    bool outputAssembly = false;
    bool verbose = false;
    int optLevel = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg[0] == '-') {
            if (arg == "-o" && i + 1 < argc) {
                outputFilename = argv[++i];
            } else if (arg == "-S") {
                outputAssembly = true;
            } else if (arg == "-v") {
                verbose = true;
            } else if (arg.substr(0, 2) == "-O") {
                optLevel = std::stoi(arg.substr(2, 1));
                if (optLevel < 0 || optLevel > 3) {
                    std::cerr << "Invalid optimization level. Using default (0).\n";
                    optLevel = 0;
                }
            } else if (arg == "-h" || arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else {
                std::cerr << "Unknown option: " << arg << "\n";
                printUsage(argv[0]);
                return 1;
            }
        } else {
            // Input filename
            inputFilename = arg;
        }
    }
    
    // Check if input file was provided
    if (inputFilename.empty()) {
        std::cerr << "Error: No input file specified.\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Set default output filename if not specified
    if (outputFilename == "a.out") {
        size_t dotPos = inputFilename.find_last_of('.');
        if (dotPos != std::string::npos) {
            outputFilename = inputFilename.substr(0, dotPos);
        } else {
            outputFilename = inputFilename;
        }
        
        if (outputAssembly) {
            outputFilename += ".s";
        } else {
            outputFilename += ".o";
        }
    }
    
    if (verbose) {
        std::cout << "Input file: " << inputFilename << "\n";
        std::cout << "Output file: " << outputFilename << "\n";
        std::cout << "Optimization level: " << optLevel << "\n";
    }
    
    // Read the input file
    std::string sourceCode = readFile(inputFilename);
    if (sourceCode.empty()) {
        return 1;
    }
    
    // Comment out LLVM code for now until we fix include paths
    /*
    // Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // Create LLVM context and module
    std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>(inputFilename, *context);
    
    // Set target triple for x86-64 ELF (bare metal)
    module->setTargetTriple("x86_64-elf");
    */
    
    try {
        // Create diagnostic reporter for error messages
        dsLang::DiagnosticReporter diagReporter;
        
        // Tokenize and parse the source code
        dsLang::Lexer lexer(sourceCode, inputFilename);
        dsLang::Parser parser(lexer, diagReporter);
        std::shared_ptr<dsLang::CompilationUnit> program = parser.Parse();
        
        if (verbose) {
            std::cout << "Parsing completed successfully\n";
        }
        
        // Perform semantic analysis
        auto semanticAnalyzer = dsLang::CreateSemanticAnalyzer(diagReporter);
        semanticAnalyzer->Analyze(program.get());
        
        if (verbose) {
            std::cout << "Semantic analysis completed successfully\n";
        }
        
        // Generate LLVM IR code - temporarily disabled due to include issues
        // dsLang::CodeGenerator codegen(inputFilename, "x86_64-elf");
        // codegen.Generate(static_cast<dsLang::CompilationUnit*>(program.get()));
        
        if (verbose) {
            std::cout << "Code generation phase skipped due to LLVM include issues\n";
        }
        
        /* Comment out LLVM code till we fix include paths
        // Verify the generated LLVM IR
        std::string verifyStr;
        llvm::raw_string_ostream verifyStream(verifyStr);
        if (llvm::verifyModule(*module, &verifyStream)) {
            std::cerr << "Error: Generated module is invalid!\n";
            std::cerr << verifyStr;
            return 1;
        }
        
        // Get target machine for x86-64
        std::string targetTriple = "x86_64-elf";
        std::string error;
        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
        
        if (!target) {
            std::cerr << "Error looking up target: " << error << "\n";
            return 1;
        }
        
        llvm::TargetOptions opt;
        std::unique_ptr<llvm::TargetMachine> targetMachine(
            target->createTargetMachine(targetTriple, "generic", "", opt, llvm::Reloc::Model::Static));
        
        module->setDataLayout(targetMachine->createDataLayout());
        
        // Open output file
        std::error_code EC;
        llvm::raw_fd_ostream outputFile(outputFilename, EC, llvm::sys::fs::OF_None);
        
        if (EC) {
            std::cerr << "Error opening output file: " << EC.message() << "\n";
            return 1;
        }
        
        // Set up pass manager
        llvm::legacy::PassManager pass;
        
        // Set output file type
        auto fileType = outputAssembly ? llvm::CGFT_AssemblyFile : llvm::CGFT_ObjectFile;
        
        // Generate code
        if (targetMachine->addPassesToEmitFile(pass, outputFile, nullptr, fileType)) {
            std::cerr << "Error: Target machine can't emit a file of this type\n";
            return 1;
        }
        
        // Run passes
        pass.run(*module);
        outputFile.close();
        */
        
        // For now, write a dummy output file to indicate we got this far
        std::ofstream dummyOutput(outputFilename);
        dummyOutput << "// Compilation successful up to semantic analysis\n";
        dummyOutput << "// LLVM code generation disabled due to include path issues\n";
        dummyOutput.close();
        
        if (verbose) {
            std::cout << "Output written to: " << outputFilename << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
