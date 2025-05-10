import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {
    
    var preferencesWindowController: PreferencesWindowController?
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Set up any initial application state here
        setupDefaultPreferences()
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        // Save any unsaved changes or application state
    }
    
    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
    
    // Set up default preferences for the application
    private func setupDefaultPreferences() {
        let defaults = UserDefaults.standard
        
        // Set default compiler path if not set
        if defaults.string(forKey: "compilerPath") == nil {
            let basePath = Bundle.main.bundlePath.components(separatedBy: "/dsLang_IDE").first ?? ""
            let defaultCompilerPath = "\(basePath)/build/dscc"
            defaults.set(defaultCompilerPath, forKey: "compilerPath")
        }
        
        // Set default themes and editor preferences
        if defaults.string(forKey: "editorTheme") == nil {
            defaults.set("Light", forKey: "editorTheme")
        }
        
        if defaults.integer(forKey: "tabSize") == 0 {
            defaults.set(4, forKey: "tabSize")
        }
        
        if defaults.object(forKey: "useSpacesForTabs") == nil {
            defaults.set(true, forKey: "useSpacesForTabs")
        }
        
        if defaults.object(forKey: "showLineNumbers") == nil {
            defaults.set(true, forKey: "showLineNumbers")
        }
    }
    
    // MARK: - Menu Action Methods
    
    @IBAction func showPreferences(_ sender: Any) {
        if preferencesWindowController == nil {
            preferencesWindowController = PreferencesWindowController(windowNibName: "PreferencesWindow")
        }
        preferencesWindowController?.showWindow(self)
    }
    
    @IBAction func newDocument(_ sender: Any) {
        // Create a new document
        let documentController = NSDocumentController.shared
        do {
            try documentController.openUntitledDocumentAndDisplay(true)
        } catch {
            showAlert(title: "Error", message: "Could not create new document: \(error.localizedDescription)")
        }
    }
    
    @IBAction func compile(_ sender: Any) {
        // Get the current document and compile it
        if let mainViewController = NSApplication.shared.mainWindow?.contentViewController as? ViewController {
            mainViewController.compile(sender)
        }
    }
    
    @IBAction func run(_ sender: Any) {
        // Get the current document and run it
        if let mainViewController = NSApplication.shared.mainWindow?.contentViewController as? ViewController {
            mainViewController.run(sender)
        }
    }
    
    @IBAction func runInQEMU(_ sender: Any) {
        // Run the compiled OS in QEMU
        if let mainViewController = NSApplication.shared.mainWindow?.contentViewController as? ViewController {
            mainViewController.runInQEMU(sender)
        }
    }
    
    // Helper method to show alerts
    private func showAlert(title: String, message: String) {
        let alert = NSAlert()
        alert.messageText = title
        alert.informativeText = message
        alert.alertStyle = .warning
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
}
