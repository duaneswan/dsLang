import Cocoa

class ViewController: NSViewController {
    
    // MARK: - Outlets
    @IBOutlet weak var splitView: NSSplitView!
    @IBOutlet weak var editorContainerView: NSView!
    @IBOutlet weak var sidebarContainerView: NSView!
    @IBOutlet weak var consoleContainerView: NSView!
    
    // Child view controllers
    private var sidebarViewController: SidebarViewController?
    private var consoleViewController: ConsoleViewController?
    
    // Editor view
    private var editorView: EditorView?
    
    // Services
    private let compilerService = CompilerService()
    
    // Current document state
    private var currentDocument: URL?
    private var documentContent: String = ""
    private var documentIsModified: Bool = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Set up the editor view
        setupEditorView()
        
        // Set up child view controllers
        setupSidebarViewController()
        setupConsoleViewController()
        
        // Register for notifications
        registerForNotifications()
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
            if let url = representedObject as? URL {
                loadDocument(at: url)
            }
        }
    }
    
    // MARK: - Setup Methods
    
    private func setupEditorView() {
        editorView = EditorView(frame: editorContainerView.bounds)
        
        if let editorView = editorView {
            editorView.autoresizingMask = [.width, .height]
            editorContainerView.addSubview(editorView)
            
            // Set up editor preferences
            let preferences = UserDefaults.standard
            let tabSize = preferences.integer(forKey: "tabSize")
            let useSpacesForTabs = preferences.bool(forKey: "useSpacesForTabs")
            let showLineNumbers = preferences.bool(forKey: "showLineNumbers")
            
            editorView.tabSize = tabSize
            editorView.useSpacesForTabs = useSpacesForTabs
            editorView.showLineNumbers = showLineNumbers
            
            // Set up callback for text changes
            editorView.textDidChangeHandler = { [weak self] newText in
                self?.documentContent = newText
                self?.documentIsModified = true
                // Update window title to show modified status
                if let window = self?.view.window {
                    if let title = window.title, !title.hasSuffix(" *") {
                        window.title = title + " *"
                    }
                }
            }
        }
    }
    
    private func setupSidebarViewController() {
        let storyboard = NSStoryboard(name: "Main", bundle: nil)
        sidebarViewController = storyboard.instantiateController(withIdentifier: "SidebarViewController") as? SidebarViewController
        
        if let sidebarVC = sidebarViewController {
            addChild(sidebarVC)
            sidebarContainerView.addSubview(sidebarVC.view)
            sidebarVC.view.frame = sidebarContainerView.bounds
            sidebarVC.view.autoresizingMask = [.width, .height]
            
            // Set up callback for file selection
            sidebarVC.fileSelectedHandler = { [weak self] url in
                self?.loadDocument(at: url)
            }
        }
    }
    
    private func setupConsoleViewController() {
        let storyboard = NSStoryboard(name: "Main", bundle: nil)
        consoleViewController = storyboard.instantiateController(withIdentifier: "ConsoleViewController") as? ConsoleViewController
        
        if let consoleVC = consoleViewController {
            addChild(consoleVC)
            consoleContainerView.addSubview(consoleVC.view)
            consoleVC.view.frame = consoleContainerView.bounds
            consoleVC.view.autoresizingMask = [.width, .height]
        }
    }
    
    private func registerForNotifications() {
        // Listen for document updates
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(documentDidUpdate(_:)),
            name: NSNotification.Name("DocumentDidUpdateNotification"),
            object: nil
        )
    }
    
    // MARK: - Document Handling
    
    private func loadDocument(at url: URL) {
        do {
            let content = try String(contentsOf: url, encoding: .utf8)
            documentContent = content
            currentDocument = url
            editorView?.text = content
            documentIsModified = false
            
            // Update window title
            if let window = view.window {
                window.title = url.lastPathComponent
            }
            
            // Update sidebar selection
            sidebarViewController?.selectFile(url)
        } catch {
            showAlert(title: "Error", message: "Could not load document: \(error.localizedDescription)")
        }
    }
    
    private func saveDocument() -> Bool {
        guard let url = currentDocument else {
            return saveDocumentAs()
        }
        
        do {
            try documentContent.write(to: url, atomically: true, encoding: .utf8)
            documentIsModified = false
            
            // Update window title to remove modified status
            if let window = view.window {
                window.title = url.lastPathComponent
            }
            
            return true
        } catch {
            showAlert(title: "Error", message: "Could not save document: \(error.localizedDescription)")
            return false
        }
    }
    
    private func saveDocumentAs() -> Bool {
        let savePanel = NSSavePanel()
        savePanel.allowedFileTypes = ["ds"]
        savePanel.canCreateDirectories = true
        savePanel.isExtensionHidden = false
        savePanel.title = "Save dsLang File"
        savePanel.message = "Choose a location to save your dsLang file."
        
        let response = savePanel.runModal()
        
        if response == .OK, let url = savePanel.url {
            currentDocument = url
            do {
                try documentContent.write(to: url, atomically: true, encoding: .utf8)
                documentIsModified = false
                
                // Update window title
                if let window = view.window {
                    window.title = url.lastPathComponent
                }
                
                // Update sidebar
                sidebarViewController?.refreshFileList()
                
                return true
            } catch {
                showAlert(title: "Error", message: "Could not save document: \(error.localizedDescription)")
                return false
            }
        }
        
        return false
    }
    
    @objc private func documentDidUpdate(_ notification: Notification) {
        if let url = notification.object as? URL, url == currentDocument {
            loadDocument(at: url)
        }
    }
    
    // MARK: - Compile and Run Methods
    
    @IBAction func compile(_ sender: Any) {
        guard saveBeforeCompileOrRun() else { return }
        
        guard let url = currentDocument else {
            showAlert(title: "Error", message: "No document to compile.")
            return
        }
        
        // Show console if hidden
        if let splitView = splitView, splitView.isSubviewCollapsed(consoleContainerView) {
            splitView.setPosition(splitView.frame.height - 200, ofDividerAt: 1)
        }
        
        // Clear console and show compiling message
        consoleViewController?.clearConsole()
        consoleViewController?.appendText("Compiling \(url.lastPathComponent)...\n")
        
        // Compile the document
        compilerService.compile(file: url) { [weak self] result in
            DispatchQueue.main.async {
                switch result {
                case .success(let output):
                    self?.consoleViewController?.appendText("Compilation successful.\n")
                    self?.consoleViewController?.appendText(output)
                case .failure(let error):
                    self?.consoleViewController?.appendText("Compilation failed.\n")
                    self?.consoleViewController?.appendText(error.localizedDescription)
                }
            }
        }
    }
    
    @IBAction func run(_ sender: Any) {
        guard saveBeforeCompileOrRun() else { return }
        
        guard let url = currentDocument else {
            showAlert(title: "Error", message: "No document to run.")
            return
        }
        
        // Show console if hidden
        if let splitView = splitView, splitView.isSubviewCollapsed(consoleContainerView) {
            splitView.setPosition(splitView.frame.height - 200, ofDividerAt: 1)
        }
        
        // Clear console and show running message
        consoleViewController?.clearConsole()
        consoleViewController?.appendText("Running \(url.lastPathComponent)...\n")
        
        // Compile and run the document
        compilerService.compileAndRun(file: url) { [weak self] result in
            DispatchQueue.main.async {
                switch result {
                case .success(let output):
                    self?.consoleViewController?.appendText("Program output:\n")
                    self?.consoleViewController?.appendText(output)
                case .failure(let error):
                    self?.consoleViewController?.appendText("Execution failed.\n")
                    self?.consoleViewController?.appendText(error.localizedDescription)
                }
            }
        }
    }
    
    @IBAction func runInQEMU(_ sender: Any) {
        // Show console if hidden
        if let splitView = splitView, splitView.isSubviewCollapsed(consoleContainerView) {
            splitView.setPosition(splitView.frame.height - 200, ofDividerAt: 1)
        }
        
        // Clear console and show QEMU message
        consoleViewController?.clearConsole()
        consoleViewController?.appendText("Starting dsOS in QEMU...\n")
        
        // Run the OS in QEMU
        compilerService.runOSInQEMU { [weak self] result in
            DispatchQueue.main.async {
                switch result {
                case .success(let output):
                    self?.consoleViewController?.appendText("QEMU output:\n")
                    self?.consoleViewController?.appendText(output)
                case .failure(let error):
                    self?.consoleViewController?.appendText("QEMU execution failed.\n")
                    self?.consoleViewController?.appendText(error.localizedDescription)
                }
            }
        }
    }
    
    // MARK: - File Menu Actions
    
    @IBAction func saveDocument(_ sender: Any) {
        _ = saveDocument()
    }
    
    @IBAction func saveDocumentAs(_ sender: Any) {
        _ = saveDocumentAs()
    }
    
    // MARK: - Helper Methods
    
    private func saveBeforeCompileOrRun() -> Bool {
        if documentIsModified {
            let alert = NSAlert()
            alert.messageText = "Save changes"
            alert.informativeText = "Do you want to save the changes before compiling/running?"
            alert.alertStyle = .warning
            alert.addButton(withTitle: "Save")
            alert.addButton(withTitle: "Cancel")
            alert.addButton(withTitle: "Don't Save")
            
            let response = alert.runModal()
            
            switch response {
            case .alertFirstButtonReturn: // Save
                return saveDocument()
            case .alertSecondButtonReturn: // Cancel
                return false
            case .alertThirdButtonReturn: // Don't Save
                return true
            default:
                return false
            }
        }
        
        return true
    }
    
    private func showAlert(title: String, message: String) {
        let alert = NSAlert()
        alert.messageText = title
        alert.informativeText = message
        alert.alertStyle = .warning
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
}
