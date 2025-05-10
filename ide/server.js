/**
 * dsLang IDE Server
 * 
 * This is a simple Express server to handle file operations,
 * compilation, and running of dsLang programs.
 */

const express = require('express');
const path = require('path');
const fs = require('fs');
const { exec } = require('child_process');
const util = require('util');
const bodyParser = require('body-parser');

const app = express();
const PORT = process.env.PORT || 3000;
const execPromise = util.promisify(exec);

// Base directories
const IDE_ROOT = __dirname;
const PROJECT_ROOT = path.join(__dirname, '..');

// Compiler paths
const COMPILER_PATH = path.join(PROJECT_ROOT, 'build', 'dscc');
const BUILD_DIR = path.join(PROJECT_ROOT, 'build');

// Serve static files from IDE directory
app.use(express.static(IDE_ROOT));
app.use(bodyParser.json());
app.use(bodyParser.text());

// Get list of files
app.get('/api/files', (req, res) => {
    try {
        let dirPath = path.join(PROJECT_ROOT, req.query.path || '');
        
        // Normalize the path to prevent directory traversal attacks
        dirPath = path.normalize(dirPath);
        
        // Make sure the path is still within our project root
        if (!dirPath.startsWith(PROJECT_ROOT)) {
            return res.status(403).json({ error: 'Access denied: Path outside of project' });
        }
        
        const items = fs.readdirSync(dirPath, { withFileTypes: true })
            .map(item => ({
                name: item.name,
                isDirectory: item.isDirectory(),
                path: path.relative(PROJECT_ROOT, path.join(dirPath, item.name))
            }));
        
        res.json(items);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Get file content
app.get('/api/file', (req, res) => {
    try {
        let filePath = path.join(PROJECT_ROOT, req.query.path || '');
        
        // Normalize the path to prevent directory traversal attacks
        filePath = path.normalize(filePath);
        
        // Make sure the path is still within our project root
        if (!filePath.startsWith(PROJECT_ROOT)) {
            return res.status(403).json({ error: 'Access denied: Path outside of project' });
        }
        
        // Check if the file exists
        if (!fs.existsSync(filePath) || !fs.statSync(filePath).isFile()) {
            return res.status(404).json({ error: 'File not found' });
        }
        
        // Read the file content
        const content = fs.readFileSync(filePath, 'utf8');
        res.send(content);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Save file content
app.post('/api/file', (req, res) => {
    try {
        let filePath = path.join(PROJECT_ROOT, req.query.path || '');
        
        // Normalize the path to prevent directory traversal attacks
        filePath = path.normalize(filePath);
        
        // Make sure the path is still within our project root
        if (!filePath.startsWith(PROJECT_ROOT)) {
            return res.status(403).json({ error: 'Access denied: Path outside of project' });
        }
        
        // Ensure the directory exists
        const dirPath = path.dirname(filePath);
        if (!fs.existsSync(dirPath)) {
            fs.mkdirSync(dirPath, { recursive: true });
        }
        
        // Write the file content
        fs.writeFileSync(filePath, req.body);
        res.json({ success: true });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Compile a dsLang file
app.post('/api/compile', async (req, res) => {
    try {
        let filePath = path.join(PROJECT_ROOT, req.query.path || '');
        
        // Normalize the path to prevent directory traversal attacks
        filePath = path.normalize(filePath);
        
        // Make sure the path is still within our project root
        if (!filePath.startsWith(PROJECT_ROOT)) {
            return res.status(403).json({ error: 'Access denied: Path outside of project' });
        }
        
        // Check if the file exists
        if (!fs.existsSync(filePath) || !fs.statSync(filePath).isFile()) {
            return res.status(404).json({ error: 'File not found' });
        }
        
        // Generate output filename
        const outputPath = path.join(BUILD_DIR, path.basename(filePath, '.ds') + '.o');
        
        // Compile using dscc
        const cmd = `${COMPILER_PATH} -o "${outputPath}" "${filePath}"`;
        
        try {
            const { stdout, stderr } = await execPromise(cmd);
            res.json({
                success: true,
                output: stdout,
                error: stderr,
                outputFile: path.relative(PROJECT_ROOT, outputPath)
            });
        } catch (error) {
            res.json({
                success: false,
                output: error.stdout || '',
                error: error.stderr || error.message
            });
        }
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Run a compiled dsLang program
app.post('/api/run', async (req, res) => {
    try {
        let filePath = path.join(PROJECT_ROOT, req.query.path || '');
        
        // Normalize the path to prevent directory traversal attacks
        filePath = path.normalize(filePath);
        
        // Make sure the path is still within our project root
        if (!filePath.startsWith(PROJECT_ROOT)) {
            return res.status(403).json({ error: 'Access denied: Path outside of project' });
        }
        
        // Check if the file exists
        if (!fs.existsSync(filePath) || !fs.statSync(filePath).isFile()) {
            return res.status(404).json({ error: 'File not found' });
        }
        
        // For demonstration, we'll just simulate running the program
        const outputPath = filePath;
        res.json({
            success: true,
            output: `Simulated output for ${path.basename(filePath)}\nHello from dsLang!`,
            error: ''
        });
        
        // In a real implementation, we would execute the compiled program
        // const cmd = `"${outputPath}"`;
        // try {
        //     const { stdout, stderr } = await execPromise(cmd);
        //     res.json({
        //         success: true,
        //         output: stdout,
        //         error: stderr
        //     });
        // } catch (error) {
        //     res.json({
        //         success: false,
        //         output: error.stdout || '',
        //         error: error.stderr || error.message
        //     });
        // }
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Run the OS in QEMU
app.post('/api/run-os', async (req, res) => {
    try {
        // Path to the kernel binary
        const kernelPath = path.join(BUILD_DIR, 'dsOS-kernel.bin');
        
        // Command to run QEMU
        const cmd = `qemu-system-i386 -kernel "${kernelPath}" -nographic`;
        
        // For demonstration, we'll just simulate the OS output
        res.json({
            success: true,
            output: `Simulated QEMU output\n\nBooting dsOS...\n\nWelcome to dsOS!\nThis kernel was written in dsLang!\n\nKernel initialized. System halted.`,
            error: ''
        });
        
        // In a real implementation, we would run QEMU
        // try {
        //     const { stdout, stderr } = await execPromise(cmd);
        //     res.json({
        //         success: true,
        //         output: stdout,
        //         error: stderr
        //     });
        // } catch (error) {
        //     res.json({
        //         success: false,
        //         output: error.stdout || '',
        //         error: error.stderr || error.message
        //     });
        // }
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Start the server
app.listen(PORT, () => {
    console.log(`dsLang IDE server running on http://localhost:${PORT}`);
});
