import Cocoa

class DocumentController: NSDocumentController {
    
    override init() {
        super.init()
        
        // Register for notifications
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(fileDidChange(_:)),
            name: NSNotification.Name("FileDidChangeNotification"),
            object: nil
        )
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
    
    // MARK: - Document Types
    
    override func defaultType(for url: URL) -> String? {
        if url.pathExtension.lowercased() == "ds" {
            return "dsLangSourceFile"
        }
        return super.defaultType(for: url)
    }
    
    // MARK: - Document Creation
    
    override func makeUntitledDocument(ofType typeName: String) throws -> NSDocument {
        let document = try super.makeUntitledDocument(ofType: typeName)
        
        // For dsLang documents, set initial content
        if typeName == "dsLangSourceFile" {
            if let dsDocument = document as? DSLangDocument {
                let templateContent = """
                /* 
                 * dsLang Example Program
                 */
                
                // Define a simple function
                define function greet with parameters name of type String returns String
                    define variable greeting as String = "Hello, " + name + "!"
                    return greeting
                end
                
                // Define a basic record (struct)
                define record User
                    define userId as Int
                    define name as String
                    define active as Bool
                end
                
                // Main program entry point
                define function main with parameters args of type Array returns Int
                    define variable user as User
                    user.userId = 1
                    user.name = "World"
                    user.active = true
                    
                    define variable message as String = greet(user.name)
                    print(message)
                    
                    return 0
                end
                """
                
                dsDocument.setContent(templateContent)
            }
        }
        
        return document
    }
    
    // MARK: - Document Saving
    
    override func saveDocument(_ sender: Any?) {
        guard let document = currentDocument else {
            return super.saveDocument(sender)
        }
        
        // If the document hasn't been saved before, use saveDocumentAs
        if document.fileURL == nil {
            saveDocumentAs(sender)
        } else {
            super.saveDocument(sender)
        }
    }
    
    // MARK: - Document Opening
    
    override func openDocument(_ sender: Any?) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = true
        openPanel.canChooseDirectories = false
        openPanel.allowsMultipleSelection = false
        openPanel.allowedFileTypes = ["ds"]
        openPanel.message = "Choose a dsLang file to open"
        
        openPanel.beginSheetModal(for: NSApp.mainWindow ?? NSApp.keyWindow ?? NSWindow()) { [weak self] (response) in
            if response == .OK, let url = openPanel.url {
                self?.openDocument(withContentsOf: url, display: true) { (document, documentWasAlreadyOpen, error) in
                    if let error = error {
                        self?.presentError(error)
                    }
                }
            }
        }
    }
    
    // MARK: - Notifications
    
    @objc private func fileDidChange(_ notification: Notification) {
        if let url = notification.object as? URL {
            // Check if this file is already open in a document
            if let document = self.document(for: url) {
                // Reload the document
                document.revert(self)
            }
        }
    }
}

// MARK: - DSLangDocument

class DSLangDocument: NSDocument {
    
    // Content of the document
    private var content: String = ""
    
    // MARK: - NSDocument
    
    override func makeWindowControllers() {
        let storyboard = NSStoryboard(name: "Main", bundle: nil)
        if let windowController = storyboard.instantiateController(withIdentifier: "DocumentWindowController") as? NSWindowController {
            self.addWindowController(windowController)
            
            // Set document in the view controller
            if let viewController = windowController.contentViewController as? ViewController {
                viewController.representedObject = fileURL
            }
        }
    }
    
    override func data(ofType typeName: String) throws -> Data {
        // Return data representation of document
        guard let data = content.data(using: .utf8) else {
            throw NSError(domain: NSCocoaErrorDomain, code: NSFileWriteUnknownError, userInfo: nil)
        }
        return data
    }
    
    override func read(from data: Data, ofType typeName: String) throws {
        // Read document from data
        guard let newContent = String(data: data, encoding: .utf8) else {
            throw NSError(domain: NSCocoaErrorDomain, code: NSFileReadUnknownError, userInfo: nil)
        }
        content = newContent
        
        // If window controllers are already set up, update them
        if !windowControllers.isEmpty {
            if let viewController = windowControllers.first?.contentViewController as? ViewController {
                viewController.representedObject = fileURL
            }
        }
    }
    
    override class var autosavesInPlace: Bool {
        return true
    }
    
    // MARK: - Custom Methods
    
    func getContent() -> String {
        return content
    }
    
    func setContent(_ newContent: String) {
        content = newContent
        updateChangeCount(.changeDone)
    }
}
