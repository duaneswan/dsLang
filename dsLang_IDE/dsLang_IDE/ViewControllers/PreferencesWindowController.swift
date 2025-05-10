import Cocoa

class PreferencesWindowController: NSWindowController {
    
    // MARK: - Outlets
    @IBOutlet private weak var compilerPathTextField: NSTextField!
    @IBOutlet private weak var buildDirectoryTextField: NSTextField!
    @IBOutlet private weak var themePopUpButton: NSPopUpButton!
    @IBOutlet private weak var tabSizeTextField: NSTextField!
    @IBOutlet private weak var useSpacesForTabsCheckbox: NSButton!
    @IBOutlet private weak var showLineNumbersCheckbox: NSButton!
    
    // MARK: - Properties
    private let userDefaults = UserDefaults.standard
    
    // MARK: - Lifecycle
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        // Set window title
        window?.title = "dsLang IDE Preferences"
        
        // Load current preferences
        loadPreferences()
    }
    
    // MARK: - Preference Loading/Saving
    
    private func loadPreferences() {
        // Compiler settings
        compilerPathTextField.stringValue = userDefaults.string(forKey: "compilerPath") ?? ""
        buildDirectoryTextField.stringValue = userDefaults.string(forKey: "buildDirectory") ?? ""
        
        // Editor settings
        let themeName = userDefaults.string(forKey: "editorTheme") ?? "Light"
        themePopUpButton.selectItem(withTitle: themeName)
        
        tabSizeTextField.integerValue = userDefaults.integer(forKey: "tabSize")
        useSpacesForTabsCheckbox.state = userDefaults.bool(forKey: "useSpacesForTabs") ? .on : .off
        showLineNumbersCheckbox.state = userDefaults.bool(forKey: "showLineNumbers") ? .on : .off
    }
    
    // MARK: - Actions
    
    @IBAction func browseCompilerPath(_ sender: Any) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = true
        openPanel.canChooseDirectories = false
        openPanel.allowsMultipleSelection = false
        openPanel.message = "Select dsLang Compiler"
        
        openPanel.beginSheetModal(for: window!) { [weak self] (response) in
            if response == .OK, let url = openPanel.url {
                self?.compilerPathTextField.stringValue = url.path
            }
        }
    }
    
    @IBAction func browseBuildDirectory(_ sender: Any) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = false
        openPanel.canChooseDirectories = true
        openPanel.allowsMultipleSelection = false
        openPanel.message = "Select Build Directory"
        
        openPanel.beginSheetModal(for: window!) { [weak self] (response) in
            if response == .OK, let url = openPanel.url {
                self?.buildDirectoryTextField.stringValue = url.path
            }
        }
    }
    
    @IBAction func savePreferences(_ sender: Any) {
        // Save compiler settings
        userDefaults.set(compilerPathTextField.stringValue, forKey: "compilerPath")
        userDefaults.set(buildDirectoryTextField.stringValue, forKey: "buildDirectory")
        
        // Save editor settings
        userDefaults.set(themePopUpButton.titleOfSelectedItem, forKey: "editorTheme")
        userDefaults.set(tabSizeTextField.integerValue, forKey: "tabSize")
        userDefaults.set(useSpacesForTabsCheckbox.state == .on, forKey: "useSpacesForTabs")
        userDefaults.set(showLineNumbersCheckbox.state == .on, forKey: "showLineNumbers")
        
        // Post notification for preferences changes
        NotificationCenter.default.post(name: NSNotification.Name("PreferencesDidChangeNotification"), object: nil)
        
        // Close window
        window?.close()
    }
    
    @IBAction func cancelPreferences(_ sender: Any) {
        // Discard changes and close window
        window?.close()
    }
    
    @IBAction func restoreDefaults(_ sender: Any) {
        // Set default compiler path
        let basePath = Bundle.main.bundlePath.components(separatedBy: "/dsLang_IDE").first ?? ""
        compilerPathTextField.stringValue = "\(basePath)/build/dscc"
        
        // Set default build directory
        buildDirectoryTextField.stringValue = "\(basePath)/build"
        
        // Set default editor settings
        themePopUpButton.selectItem(withTitle: "Light")
        tabSizeTextField.integerValue = 4
        useSpacesForTabsCheckbox.state = .on
        showLineNumbersCheckbox.state = .on
    }
}

// MARK: - Theme Options

extension PreferencesWindowController {
    
    enum EditorTheme: String, CaseIterable {
        case light = "Light"
        case dark = "Dark"
        case monochrome = "Monochrome"
        
        var colors: (background: NSColor, text: NSColor, selection: NSColor) {
            switch self {
            case .light:
                return (NSColor.white, NSColor.black, NSColor(calibratedRed: 0.8, green: 0.8, blue: 1.0, alpha: 0.5))
            case .dark:
                return (NSColor(calibratedWhite: 0.15, alpha: 1.0), NSColor.white, NSColor(calibratedRed: 0.3, green: 0.3, blue: 0.6, alpha: 0.5))
            case .monochrome:
                return (NSColor(calibratedWhite: 0.95, alpha: 1.0), NSColor.darkGray, NSColor(calibratedWhite: 0.8, alpha: 0.5))
            }
        }
    }
    
    func getSelectedTheme() -> EditorTheme {
        let themeName = themePopUpButton.titleOfSelectedItem ?? "Light"
        return EditorTheme(rawValue: themeName) ?? .light
    }
}
