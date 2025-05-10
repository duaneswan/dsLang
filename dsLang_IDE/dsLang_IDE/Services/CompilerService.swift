import Cocoa

class CompilerService {
    
    // MARK: - Error Types
    
    enum CompilationError: Error {
        case compilerNotFound
        case compilationFailed(String)
        case executionFailed(String)
        case qemuNotFound
        
        var localizedDescription: String {
            switch self {
            case .compilerNotFound:
                return "dsLang compiler not found. Please check your compiler path in preferences."
            case .compilationFailed(let message):
                return "Compilation failed: \(message)"
            case .executionFailed(let message):
                return "Execution failed: \(message)"
            case .qemuNotFound:
                return "QEMU emulator not found. Please make sure QEMU is installed on your system."
            }
        }
    }
    
    // MARK: - Properties
    
    private var compilerPath: String {
        return UserDefaults.standard.string(forKey: "compilerPath") ?? "/usr/local/bin/dscc"
    }
    
    private var buildDirectory: String {
        let path = UserDefaults.standard.string(forKey: "buildDirectory")
        
        if let path = path, !path.isEmpty {
            return path
        }
        
        // Fallback to default build directory
        // Find the parent directory of the app
        let basePath = Bundle.main.bundlePath.components(separatedBy: "/dsLang_IDE").first ?? ""
        return "\(basePath)/build"
    }
    
    // MARK: - Compilation Methods
    
    /// Compiles a dsLang file
    /// - Parameters:
    ///   - file: The URL of the file to compile
    ///   - completion: Completion handler with result
    func compile(file: URL, completion: @escaping (Result<String, Error>) -> Void) {
        // Check if compiler exists
        let fileManager = FileManager.default
        if !fileManager.fileExists(atPath: compilerPath) {
            completion(.failure(CompilationError.compilerNotFound))
            return
        }
        
        // Get output file path
        let outputFileName = file.deletingPathExtension().lastPathComponent + ".o"
        let outputPath = URL(fileURLWithPath: buildDirectory).appendingPathComponent(outputFileName)
        
        // Create build directory if it doesn't exist
        if !fileManager.fileExists(atPath: buildDirectory) {
            do {
                try fileManager.createDirectory(at: URL(fileURLWithPath: buildDirectory), withIntermediateDirectories: true, attributes: nil)
            } catch {
                completion(.failure(error))
                return
            }
        }
        
        // Create the compilation process
        let process = Process()
        let stdout = Pipe()
        let stderr = Pipe()
        
        process.executableURL = URL(fileURLWithPath: compilerPath)
        process.arguments = ["-o", outputPath.path, file.path]
        process.standardOutput = stdout
        process.standardError = stderr
        
        do {
            // Run compilation
            try process.run()
            process.waitUntilExit()
            
            let outputData = stdout.fileHandleForReading.readDataToEndOfFile()
            let errorData = stderr.fileHandleForReading.readDataToEndOfFile()
            
            let output = String(data: outputData, encoding: .utf8) ?? ""
            let error = String(data: errorData, encoding: .utf8) ?? ""
            
            if process.terminationStatus != 0 {
                // Compilation failed
                completion(.failure(CompilationError.compilationFailed(error)))
            } else {
                // Compilation succeeded
                let result = """
                Compilation successful.
                Output file: \(outputPath.path)
                
                \(output)
                \(error)
                """
                completion(.success(result))
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    /// Compiles and runs a dsLang file
    /// - Parameters:
    ///   - file: The URL of the file to compile and run
    ///   - completion: Completion handler with result
    func compileAndRun(file: URL, completion: @escaping (Result<String, Error>) -> Void) {
        // First compile the file
        compile(file: file) { [weak self] result in
            switch result {
            case .success:
                // Now run the file
                self?.runCompiledFile(file: file, completion: completion)
            case .failure(let error):
                completion(.failure(error))
            }
        }
    }
    
    /// Runs a compiled dsLang file
    /// - Parameters:
    ///   - file: The URL of the source file that was compiled
    ///   - completion: Completion handler with result
    private func runCompiledFile(file: URL, completion: @escaping (Result<String, Error>) -> Void) {
        // Get compiled file path
        let compiledFileName = file.deletingPathExtension().lastPathComponent + ".o"
        let compiledFilePath = URL(fileURLWithPath: buildDirectory).appendingPathComponent(compiledFileName)
        
        // Check if compiled file exists
        let fileManager = FileManager.default
        if !fileManager.fileExists(atPath: compiledFilePath.path) {
            completion(.failure(CompilationError.executionFailed("Compiled file not found: \(compiledFilePath.path)")))
            return
        }
        
        // Create the execution process
        let process = Process()
        let stdout = Pipe()
        let stderr = Pipe()
        
        // In a real environment, you'd have a runtime or execute the program directly
        // For this example, we'll simluate running with a helper script
        process.executableURL = URL(fileURLWithPath: "/bin/sh")
        process.arguments = ["-c", "echo 'Running \(compiledFilePath.path)...\nProgram output:'; \(compiledFilePath.path)"]
        process.standardOutput = stdout
        process.standardError = stderr
        
        do {
            // Run execution
            try process.run()
            process.waitUntilExit()
            
            let outputData = stdout.fileHandleForReading.readDataToEndOfFile()
            let errorData = stderr.fileHandleForReading.readDataToEndOfFile()
            
            let output = String(data: outputData, encoding: .utf8) ?? ""
            let error = String(data: errorData, encoding: .utf8) ?? ""
            
            if process.terminationStatus != 0 {
                // Execution failed
                completion(.failure(CompilationError.executionFailed(error)))
            } else {
                // Execution succeeded
                completion(.success(output))
            }
        } catch {
            completion(.failure(error))
        }
    }
    
    /// Runs the dsOS operating system in QEMU
    /// - Parameter completion: Completion handler with result
    func runOSInQEMU(completion: @escaping (Result<String, Error>) -> Void) {
        // Check if QEMU is installed
        let qemuPath = "/usr/local/bin/qemu-system-i386"
        let fileManager = FileManager.default
        
        if !fileManager.fileExists(atPath: qemuPath) {
            completion(.failure(CompilationError.qemuNotFound))
            return
        }
        
        // Get kernel binary path
        let kernelPath = URL(fileURLWithPath: buildDirectory).appendingPathComponent("dsOS-kernel.bin")
        
        // For demonstration purposes, we'll simulate the QEMU output
        // In a real environment, you'd run QEMU with the kernel
        
        // Create the execution process
        let process = Process()
        let stdout = Pipe()
        let stderr = Pipe()
        
        process.executableURL = URL(fileURLWithPath: "/bin/sh")
        process.arguments = ["-c", "echo 'Running QEMU with \(kernelPath.path)...\n\nBooting dsOS...\n\nWelcome to dsOS!\nThis kernel was written in dsLang!\n\nKernel initialized. System halted.'"]
        process.standardOutput = stdout
        process.standardError = stderr
        
        do {
            // Run QEMU simulation
            try process.run()
            process.waitUntilExit()
            
            let outputData = stdout.fileHandleForReading.readDataToEndOfFile()
            let output = String(data: outputData, encoding: .utf8) ?? ""
            
            // Simulation succeeded
            completion(.success(output))
        } catch {
            completion(.failure(error))
        }
        
        // In a real environment, you'd do something like:
        /*
        process.executableURL = URL(fileURLWithPath: qemuPath)
        process.arguments = ["-kernel", kernelPath.path, "-nographic"]
        */
    }
}
