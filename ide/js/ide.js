// dsLang IDE JavaScript

document.addEventListener('DOMContentLoaded', function() {
    // Constants
    const DEFAULT_FILE = 'main.ds';
    const DEFAULT_CONTENT = '/* dsLang program */\n\n// This is a simple dsLang program\n\nint main() {\n    puts("Hello, world from dsLang!");\n    return 0;\n}\n';
    
    // References to DOM elements
    const editor = document.getElementById('editor');
    const output = document.getElementById('output');
    const fileList = document.getElementById('file-list');
    const cursorPosition = document.getElementById('cursor-position');
    const compilerStatus = document.getElementById('compiler-status');
    const clearOutputBtn = document.getElementById('clear-output');
    
    // Button references
    const newFileBtn = document.getElementById('new-file');
    const openFileBtn = document.getElementById('open-file');
    const saveFileBtn = document.getElementById('save-file');
    const compileBtn = document.getElementById('compile');
    const runBtn = document.getElementById('run');
    const runOSBtn = document.getElementById('run-os');
    
    // Dialog elements
    const fileDialog = document.getElementById('file-dialog');
    const fileBrowser = document.getElementById('file-browser');
    const closeDialog = document.querySelector('.close');
    const dialogCancel = document.getElementById('dialog-cancel');
    const dialogConfirm = document.getElementById('dialog-confirm');
    
    // State
    let currentFile = DEFAULT_FILE;
    let files = {};
    let editorInstance = null;
    let projectRoot = '';
    let modified = false;
    
    // Initialize CodeMirror
    function initCodeMirror() {
        // Fetch CodeMirror library or use the one already included
        if (typeof CodeMirror !== 'undefined') {
            setupEditor();
        } else {
            // Load CodeMirror dynamically if not included
            loadScript('https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.65.2/codemirror.min.js', function() {
                setupEditor();
            });
        }
    }
    
    // Set up the code editor
    function setupEditor() {
        editorInstance = CodeMirror(editor, {
            value: DEFAULT_CONTENT,
            mode: 'dsLang',
            theme: 'dslang',
            lineNumbers: true,
            autoCloseBrackets: true,
            matchBrackets: true,
            indentUnit: 4,
            tabSize: 4,
            indentWithTabs: false,
            extraKeys: {
                "Tab": function(cm) {
                    if (cm.somethingSelected()) {
                        cm.indentSelection("add");
                    } else {
                        cm.replaceSelection("    ", "end");
                    }
                }
            }
        });
        
        // Save the default file
        files[currentFile] = DEFAULT_CONTENT;
        updateFileList();
        
        // Set up editor change events
        editorInstance.on('change', function(instance) {
            modified = true;
            files[currentFile] = instance.getValue();
        });
        
        editorInstance.on('cursorActivity', function(instance) {
            const cursor = instance.getCursor();
            cursorPosition.textContent = `Line: ${cursor.line + 1}, Col: ${cursor.ch + 1}`;
        });
        
        // Initial cursor position
        cursorPosition.textContent = 'Line: 1, Col: 1';
    }
    
    // Helper function to load scripts dynamically
    function loadScript(url, callback) {
        const script = document.createElement('script');
        script.src = url;
        script.onload = callback;
        document.head.appendChild(script);
    }
    
    // Update the file list in the sidebar
    function updateFileList() {
        fileList.innerHTML = '';
        Object.keys(files).sort().forEach(file => {
            const li = document.createElement('li');
            li.textContent = file;
            li.addEventListener('click', () => openFile(file));
            if (file === currentFile) {
                li.classList.add('active');
            }
            fileList.appendChild(li);
        });
    }
    
    // Open a file in the editor
    function openFile(fileName) {
        if (modified && confirm('You have unsaved changes. Do you want to save them?')) {
            saveFile();
        }
        
        currentFile = fileName;
        editorInstance.setValue(files[fileName] || '');
        modified = false;
        updateFileList();
    }
    
    // Create a new file
    function createNewFile() {
        const fileName = prompt('Enter file name:', 'newfile.ds');
        if (fileName) {
            if (files[fileName]) {
                alert('File already exists!');
                return;
            }
            
            files[fileName] = '';
            currentFile = fileName;
            editorInstance.setValue('');
            updateFileList();
            modified = false;
        }
    }
    
    // Save the current file
    function saveFile() {
        files[currentFile] = editorInstance.getValue();
        modified = false;
        
        // In a real implementation, this would save to the server or local filesystem
        // For this demo, we'll just show a notification
        addOutputMessage(`File "${currentFile}" saved.`);
        
        // Generate a downloadable file
        const blob = new Blob([files[currentFile]], {type: 'text/plain'});
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.style.display = 'none';
        a.href = url;
        a.download = currentFile;
        
        document.body.appendChild(a);
        a.click();
        
        setTimeout(() => {
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        }, 100);
    }
    
    // Compile the current file
    function compileFile() {
        addOutputMessage(`Compiling "${currentFile}"...`);
        compilerStatus.textContent = 'Compiling...';
        
        // Simulate compilation
        setTimeout(() => {
            // For demo purposes, just check if the code contains "main"
            const code = editorInstance.getValue();
            if (code.includes('main()')) {
                addOutputMessage('Compilation successful.');
                compilerStatus.textContent = 'Ready';
            } else {
                addOutputMessage('Error: No main function found.');
                compilerStatus.textContent = 'Error';
            }
        }, 1000);
    }
    
    // Run the compiled program
    function runProgram() {
        addOutputMessage(`Running "${currentFile}"...`);
        
        // Simulate running the program
        setTimeout(() => {
            const code = editorInstance.getValue();
            
            // Extract strings in puts() calls for demo output
            const regex = /puts\s*\(\s*"([^"]*)"\s*\)/g;
            let match;
            let found = false;
            
            while ((match = regex.exec(code)) !== null) {
                addOutputMessage(`> ${match[1]}`);
                found = true;
            }
            
            if (!found) {
                addOutputMessage('> Program executed but produced no output.');
            }
            
            addOutputMessage('Program execution completed.');
        }, 800);
    }
    
    // Run the OS in QEMU
    function runOS() {
        addOutputMessage('Starting QEMU...');
        addOutputMessage('Loading dsOS kernel...');
        
        // Simulate OS booting
        setTimeout(() => {
            addOutputMessage('Booting dsOS...');
        }, 500);
        
        setTimeout(() => {
            addOutputMessage('Welcome to dsOS!');
            addOutputMessage('This kernel was written in dsLang!');
        }, 1500);
    }
    
    // Add a message to the output console
    function addOutputMessage(message) {
        const timestamp = new Date().toLocaleTimeString();
        const line = document.createElement('div');
        line.classList.add('output-line');
        line.textContent = `[${timestamp}] ${message}`;
        output.appendChild(line);
        output.scrollTop = output.scrollHeight;
    }
    
    // Clear the output console
    function clearOutput() {
        output.innerHTML = '';
    }
    
    // Open file dialog
    function showFileDialog() {
        fileBrowser.innerHTML = '';
        
        // Add files to the browser
        Object.keys(files).sort().forEach(file => {
            const fileItem = document.createElement('div');
            fileItem.classList.add('file-item');
            fileItem.textContent = file;
            fileItem.dataset.filename = file;
            
            fileItem.addEventListener('click', (e) => {
                const selected = document.querySelector('.file-item.selected');
                if (selected) {
                    selected.classList.remove('selected');
                }
                e.currentTarget.classList.add('selected');
            });
            
            fileItem.addEventListener('dblclick', (e) => {
                openFile(e.currentTarget.dataset.filename);
                fileDialog.style.display = 'none';
            });
            
            fileBrowser.appendChild(fileItem);
        });
        
        fileDialog.style.display = 'block';
    }
    
    // Initialize the IDE
    function init() {
        initCodeMirror();
        
        // Set up event listeners for buttons
        newFileBtn.addEventListener('click', createNewFile);
        openFileBtn.addEventListener('click', showFileDialog);
        saveFileBtn.addEventListener('click', saveFile);
        compileBtn.addEventListener('click', compileFile);
        runBtn.addEventListener('click', runProgram);
        runOSBtn.addEventListener('click', runOS);
        clearOutputBtn.addEventListener('click', clearOutput);
        
        // Dialog event listeners
        closeDialog.addEventListener('click', () => {
            fileDialog.style.display = 'none';
        });
        
        dialogCancel.addEventListener('click', () => {
            fileDialog.style.display = 'none';
        });
        
        dialogConfirm.addEventListener('click', () => {
            const selected = document.querySelector('.file-item.selected');
            if (selected) {
                openFile(selected.dataset.filename);
            }
            fileDialog.style.display = 'none';
        });
        
        // Close dialog when clicking outside
        window.addEventListener('click', (e) => {
            if (e.target === fileDialog) {
                fileDialog.style.display = 'none';
            }
        });
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            // Ctrl+S to save
            if (e.ctrlKey && e.key === 's') {
                e.preventDefault();
                saveFile();
            }
            
            // Ctrl+N for new file
            if (e.ctrlKey && e.key === 'n') {
                e.preventDefault();
                createNewFile();
            }
            
            // F9 to compile
            if (e.key === 'F9') {
                e.preventDefault();
                compileFile();
            }
            
            // F5 to run
            if (e.key === 'F5') {
                e.preventDefault();
                runProgram();
            }
        });
        
        // Welcome message
        addOutputMessage('Welcome to dsLang IDE - Ready to compile dsLang programs.');
        addOutputMessage(`Loaded ${currentFile}`);
    }
    
    // Start the IDE
    init();
});
