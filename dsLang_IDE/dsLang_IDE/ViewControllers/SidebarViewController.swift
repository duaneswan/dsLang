import Cocoa

class SidebarViewController: NSViewController {
    
    // MARK: - Outlets
    @IBOutlet weak var outlineView: NSOutlineView!
    @IBOutlet weak var pathControl: NSPathControl!
    
    // MARK: - Properties
    
    // Root item for the outline view
    private var rootItems: [FileSystemItem] = []
    
    // Current working directory
    private var workingDirectory: URL?
    
    // Callback for when a file is selected
    var fileSelectedHandler: ((URL) -> Void)?
    
    // Context menu
    private var contextMenu: NSMenu?
    
    // MARK: - View Lifecycle
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Set up the outline view
        setupOutlineView()
        
        // Set up the context menu
        setupContextMenu()
        
        // Set initial directory
        setInitialDirectory()
    }
    
    // MARK: - Setup Methods
    
    private func setupOutlineView() {
        outlineView.dataSource = self
        outlineView.delegate = self
        outlineView.allowsMultipleSelection = false
        outlineView.rowHeight = 20.0
        
        // Register for double click
        outlineView.target = self
        outlineView.action = #selector(outlineViewItemClicked(_:))
        outlineView.doubleAction = #selector(outlineViewItemDoubleClicked(_:))
    }
    
    private func setupContextMenu() {
        let menu = NSMenu()
        menu.addItem(NSMenuItem(title: "New File", action: #selector(newFile(_:)), keyEquivalent: "n"))
        menu.addItem(NSMenuItem(title: "New Folder", action: #selector(newFolder(_:)), keyEquivalent: "f"))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Open in Finder", action: #selector(openInFinder(_:)), keyEquivalent: ""))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Delete", action: #selector(delete(_:)), keyEquivalent: ""))
        menu.addItem(NSMenuItem(title: "Rename", action: #selector(rename(_:)), keyEquivalent: ""))
        
        contextMenu = menu
        outlineView.menu = menu
    }
    
    private func setInitialDirectory() {
        // Default to current user's home directory
        var initialDirectory = FileManager.default.homeDirectoryForCurrentUser
        
        // Check if we have a default directory in user preferences
        if let savedDirectoryPath = UserDefaults.standard.string(forKey: "lastWorkingDirectory") {
            let savedDirectory = URL(fileURLWithPath: savedDirectoryPath)
            var isDirectory: ObjCBool = false
            
            if FileManager.default.fileExists(atPath: savedDirectoryPath, isDirectory: &isDirectory) && isDirectory.boolValue {
                initialDirectory = savedDirectory
            }
        }
        
        setWorkingDirectory(initialDirectory)
    }
    
    // MARK: - File System Methods
    
    func setWorkingDirectory(_ directory: URL) {
        workingDirectory = directory
        pathControl.url = directory
        
        // Save to user defaults
        UserDefaults.standard.set(directory.path, forKey: "lastWorkingDirectory")
        
        // Load directory contents
        refreshFileList()
    }
    
    private func loadDirectoryContents(_ directory: URL) -> [FileSystemItem] {
        let fileManager = FileManager.default
        var items: [FileSystemItem] = []
        
        do {
            let contents = try fileManager.contentsOfDirectory(at: directory, includingPropertiesForKeys: [.isDirectoryKey], options: [.skipsHiddenFiles])
            
            for url in contents {
                let isDirectory = (try? url.resourceValues(forKeys: [.isDirectoryKey]))?.isDirectory ?? false
                let item = FileSystemItem(url: url, isDirectory: isDirectory)
                
                if isDirectory {
                    // Preload children for directories
                    item.children = loadDirectoryContents(url)
                }
                
                items.append(item)
            }
            
            // Sort: directories first, then alphabetically
            items.sort { (item1, item2) -> Bool in
                if item1.isDirectory && !item2.isDirectory {
                    return true
                } else if !item1.isDirectory && item2.isDirectory {
                    return false
                } else {
                    return item1.name.localizedStandardCompare(item2.name) == .orderedAscending
                }
            }
        } catch {
            print("Error loading directory contents: \(error)")
        }
        
        return items
    }
    
    @objc func refreshFileList() {
        guard let directory = workingDirectory else { return }
        
        // Load new directory contents
        rootItems = loadDirectoryContents(directory)
        
        // Reload the outline view
        outlineView.reloadData()
        
        // Expand the root items
        for i in 0..<rootItems.count {
            if rootItems[i].isDirectory {
                outlineView.expandItem(rootItems[i])
            }
        }
    }
    
    // MARK: - File Selection
    
    @objc private func outlineViewItemClicked(_ sender: Any) {
        // Handle single click
        let clickedRow = outlineView.clickedRow
        guard clickedRow >= 0 else { return }
        
        guard let item = outlineView.item(atRow: clickedRow) as? FileSystemItem else { return }
        
        // If it's a directory, expand/collapse it
        if item.isDirectory {
            if outlineView.isItemExpanded(item) {
                outlineView.collapseItem(item)
            } else {
                outlineView.expandItem(item)
            }
        }
    }
    
    @objc private func outlineViewItemDoubleClicked(_ sender: Any) {
        // Handle double click
        let clickedRow = outlineView.clickedRow
        guard clickedRow >= 0 else { return }
        
        guard let item = outlineView.item(atRow: clickedRow) as? FileSystemItem else { return }
        
        // If it's a file, open it
        if !item.isDirectory {
            fileSelectedHandler?(item.url)
        }
    }
    
    // Select a specific file in the outline view
    func selectFile(_ file: URL) {
        // Find the item corresponding to the file
        let item = findItem(for: file, in: rootItems)
        
        if let item = item {
            // Expand parent directories
            expandParents(for: item)
            
            // Get row for the item
            let row = outlineView.row(forItem: item)
            if row >= 0 {
                outlineView.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
            }
        }
    }
    
    private func findItem(for url: URL, in items: [FileSystemItem]) -> FileSystemItem? {
        for item in items {
            if item.url == url {
                return item
            }
            
            if item.isDirectory {
                if let found = findItem(for: url, in: item.children) {
                    return found
                }
            }
        }
        
        return nil
    }
    
    private func expandParents(for item: FileSystemItem) {
        var parent = item.parent
        while let p = parent {
            outlineView.expandItem(p)
            parent = p.parent
        }
    }
    
    // MARK: - Context Menu Actions
    
    @IBAction func newFile(_ sender: Any) {
        let selectedItem = getSelectedItemOrCurrentDirectory()
        guard let parentDirectory = getDirectoryURL(for: selectedItem) else { return }
        
        let alert = NSAlert()
        alert.messageText = "Create New File"
        alert.informativeText = "Enter a name for the new file:"
        
        let textField = NSTextField(frame: NSRect(x: 0, y: 0, width: 300, height: 24))
        textField.placeholderString = "filename.ds"
        alert.accessoryView = textField
        
        alert.addButton(withTitle: "Create")
        alert.addButton(withTitle: "Cancel")
        
        alert.beginSheetModal(for: view.window!) { [weak self] (response) in
            if response == .alertFirstButtonReturn {
                let fileName = textField.stringValue.trimmingCharacters(in: .whitespaces)
                guard !fileName.isEmpty else { return }
                
                let fileURL = parentDirectory.appendingPathComponent(fileName)
                
                // Create empty file
                do {
                    try "".write(to: fileURL, atomically: true, encoding: .utf8)
                    self?.refreshFileList()
                    self?.fileSelectedHandler?(fileURL)
                } catch {
                    self?.showError("Could not create file: \(error.localizedDescription)")
                }
            }
        }
    }
    
    @IBAction func newFolder(_ sender: Any) {
        let selectedItem = getSelectedItemOrCurrentDirectory()
        guard let parentDirectory = getDirectoryURL(for: selectedItem) else { return }
        
        let alert = NSAlert()
        alert.messageText = "Create New Folder"
        alert.informativeText = "Enter a name for the new folder:"
        
        let textField = NSTextField(frame: NSRect(x: 0, y: 0, width: 300, height: 24))
        textField.placeholderString = "foldername"
        alert.accessoryView = textField
        
        alert.addButton(withTitle: "Create")
        alert.addButton(withTitle: "Cancel")
        
        alert.beginSheetModal(for: view.window!) { [weak self] (response) in
            if response == .alertFirstButtonReturn {
                let folderName = textField.stringValue.trimmingCharacters(in: .whitespaces)
                guard !folderName.isEmpty else { return }
                
                let folderURL = parentDirectory.appendingPathComponent(folderName)
                
                // Create directory
                do {
                    try FileManager.default.createDirectory(at: folderURL, withIntermediateDirectories: true, attributes: nil)
                    self?.refreshFileList()
                } catch {
                    self?.showError("Could not create folder: \(error.localizedDescription)")
                }
            }
        }
    }
    
    @IBAction func openInFinder(_ sender: Any) {
        guard let selectedItem = getSelectedItem() else { return }
        NSWorkspace.shared.open(selectedItem.url)
    }
    
    @IBAction func delete(_ sender: Any) {
        guard let selectedItem = getSelectedItem() else { return }
        
        let alert = NSAlert()
        alert.messageText = "Delete \(selectedItem.name)"
        alert.informativeText = "Are you sure you want to delete this \(selectedItem.isDirectory ? "folder" : "file")? This cannot be undone."
        alert.alertStyle = .warning
        
        alert.addButton(withTitle: "Delete")
        alert.addButton(withTitle: "Cancel")
        
        alert.beginSheetModal(for: view.window!) { [weak self] (response) in
            if response == .alertFirstButtonReturn {
                do {
                    try FileManager.default.removeItem(at: selectedItem.url)
                    self?.refreshFileList()
                } catch {
                    self?.showError("Could not delete item: \(error.localizedDescription)")
                }
            }
        }
    }
    
    @IBAction func rename(_ sender: Any) {
        guard let selectedItem = getSelectedItem() else { return }
        
        let alert = NSAlert()
        alert.messageText = "Rename \(selectedItem.name)"
        alert.informativeText = "Enter a new name:"
        
        let textField = NSTextField(frame: NSRect(x: 0, y: 0, width: 300, height: 24))
        textField.stringValue = selectedItem.name
        alert.accessoryView = textField
        
        alert.addButton(withTitle: "Rename")
        alert.addButton(withTitle: "Cancel")
        
        alert.beginSheetModal(for: view.window!) { [weak self] (response) in
            if response == .alertFirstButtonReturn {
                let newName = textField.stringValue.trimmingCharacters(in: .whitespaces)
                guard !newName.isEmpty else { return }
                
                let newURL = selectedItem.url.deletingLastPathComponent().appendingPathComponent(newName)
                
                do {
                    try FileManager.default.moveItem(at: selectedItem.url, to: newURL)
                    self?.refreshFileList()
                } catch {
                    self?.showError("Could not rename item: \(error.localizedDescription)")
                }
            }
        }
    }
    
    // MARK: - Helper Methods
    
    private func getSelectedItem() -> FileSystemItem? {
        let selectedRow = outlineView.selectedRow
        guard selectedRow >= 0 else { return nil }
        
        return outlineView.item(atRow: selectedRow) as? FileSystemItem
    }
    
    private func getSelectedItemOrCurrentDirectory() -> FileSystemItem? {
        if let selectedItem = getSelectedItem() {
            return selectedItem
        } else if let workingDirectory = workingDirectory {
            // Create a virtual root item
            return FileSystemItem(url: workingDirectory, isDirectory: true)
        }
        
        return nil
    }
    
    private func getDirectoryURL(for item: FileSystemItem?) -> URL? {
        guard let item = item else { return workingDirectory }
        
        if item.isDirectory {
            return item.url
        } else {
            return item.url.deletingLastPathComponent()
        }
    }
    
    private func showError(_ message: String) {
        let alert = NSAlert()
        alert.messageText = "Error"
        alert.informativeText = message
        alert.alertStyle = .warning
        alert.addButton(withTitle: "OK")
        
        alert.beginSheetModal(for: view.window!, completionHandler: nil)
    }
}

// MARK: - NSOutlineViewDataSource

extension SidebarViewController: NSOutlineViewDataSource {
    
    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        if item == nil {
            return rootItems.count
        } else if let fileItem = item as? FileSystemItem {
            return fileItem.children.count
        }
        
        return 0
    }
    
    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
        if item == nil {
            return rootItems[index]
        } else if let fileItem = item as? FileSystemItem {
            let child = fileItem.children[index]
            child.parent = fileItem
            return child
        }
        
        fatalError("Invalid item")
    }
    
    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        if let fileItem = item as? FileSystemItem {
            return fileItem.isDirectory && !fileItem.children.isEmpty
        }
        
        return false
    }
}

// MARK: - NSOutlineViewDelegate

extension SidebarViewController: NSOutlineViewDelegate {
    
    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {
        guard let fileItem = item as? FileSystemItem else { return nil }
        
        var cellView = outlineView.makeView(withIdentifier: NSUserInterfaceItemIdentifier("FileCell"), owner: self) as? NSTableCellView
        
        if cellView == nil {
            cellView = NSTableCellView()
            cellView?.identifier = NSUserInterfaceItemIdentifier("FileCell")
            
            let textField = NSTextField()
            textField.isBezeled = false
            textField.drawsBackground = false
            textField.isEditable = false
            textField.isSelectable = false
            textField.translatesAutoresizingMaskIntoConstraints = false
            
            let imageView = NSImageView()
            imageView.translatesAutoresizingMaskIntoConstraints = false
            
            cellView?.addSubview(textField)
            cellView?.addSubview(imageView)
            
            cellView?.textField = textField
            cellView?.imageView = imageView
            
            NSLayoutConstraint.activate([
                imageView.leadingAnchor.constraint(equalTo: cellView!.leadingAnchor, constant: 2),
                imageView.centerYAnchor.constraint(equalTo: cellView!.centerYAnchor),
                imageView.widthAnchor.constraint(equalToConstant: 16),
                imageView.heightAnchor.constraint(equalToConstant: 16),
                
                textField.leadingAnchor.constraint(equalTo: imageView.trailingAnchor, constant: 4),
                textField.trailingAnchor.constraint(equalTo: cellView!.trailingAnchor, constant: -4),
                textField.centerYAnchor.constraint(equalTo: cellView!.centerYAnchor)
            ])
        }
        
        // Set up cell
        cellView?.textField?.stringValue = fileItem.name
        
        if fileItem.isDirectory {
            cellView?.imageView?.image = NSImage(named: "NSFolder")
        } else {
            // Check for dsLang file
            if fileItem.name.hasSuffix(".ds") {
                cellView?.imageView?.image = NSImage(named: "NSFileTypeForUnknownData")
            } else {
                cellView?.imageView?.image = NSImage(named: "NSFileTypeForDocument")
            }
        }
        
        return cellView
    }
    
    func outlineViewSelectionDidChange(_ notification: Notification) {
        if let selectedItem = getSelectedItem(), !selectedItem.isDirectory {
            fileSelectedHandler?(selectedItem.url)
        }
    }
}

// MARK: - NSPathControlDelegate

extension SidebarViewController: NSPathControlDelegate {
    
    func pathControl(_ pathControl: NSPathControl, willDisplay openPanel: NSOpenPanel) {
        openPanel.canChooseFiles = false
        openPanel.canChooseDirectories = true
        openPanel.allowsMultipleSelection = false
    }
    
    func pathControl(_ pathControl: NSPathControl, willPopUp menu: NSMenu) {
        // Can customize the menu if needed
    }
    
    @IBAction func pathControlClicked(_ sender: NSPathControl) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = false
        openPanel.canChooseDirectories = true
        openPanel.allowsMultipleSelection = false
        
        openPanel.begin { [weak self] (result) in
            if result == .OK, let url = openPanel.url {
                self?.setWorkingDirectory(url)
            }
        }
    }
}

// MARK: - FileSystemItem

class FileSystemItem: NSObject {
    let url: URL
    let name: String
    let isDirectory: Bool
    var children: [FileSystemItem] = []
    weak var parent: FileSystemItem?
    
    init(url: URL, isDirectory: Bool) {
        self.url = url
        self.name = url.lastPathComponent
        self.isDirectory = isDirectory
        super.init()
    }
}
