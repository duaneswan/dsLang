import Cocoa

class ConsoleViewController: NSViewController {
    
    // MARK: - Outlets
    @IBOutlet private weak var textView: NSTextView!
    @IBOutlet private weak var scrollView: NSScrollView!
    @IBOutlet private weak var clearButton: NSButton!
    
    // MARK: - Properties
    private let errorAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemRed,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let warningAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemOrange,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let standardAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.textColor,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    // MARK: - View Lifecycle
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Configure text view
        setupTextView()
        
        // Set up initial console message
        showWelcomeMessage()
    }
    
    // MARK: - Setup Methods
    
    private func setupTextView() {
        // Configure the text view
        textView.isEditable = false
        textView.isSelectable = true
        textView.font = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
        textView.textColor = NSColor.textColor
        textView.backgroundColor = NSColor(calibratedWhite: 0.95, alpha: 1.0)
        textView.isAutomaticQuoteSubstitutionEnabled = false
        textView.isAutomaticLinkDetectionEnabled = false
        textView.isAutomaticDashSubstitutionEnabled = false
        textView.isAutomaticSpellingCorrectionEnabled = false
        textView.isAutomaticTextReplacementEnabled = false
        textView.enabledTextCheckingTypes = 0  // Disable spell checking
        
        // Set up scroll view if needed
        if scrollView == nil {
            // Create a scroll view if it wasn't injected via IB
            let scrollView = NSScrollView(frame: view.bounds)
            scrollView.hasVerticalScroller = true
            scrollView.hasHorizontalScroller = true
            scrollView.autoresizingMask = [.width, .height]
            scrollView.borderType = .noBorder
            scrollView.documentView = textView
            
            view.addSubview(scrollView)
            self.scrollView = scrollView
        }
        
        // Set up clear button if needed
        if clearButton == nil {
            let clearButton = NSButton(frame: NSRect(x: view.bounds.width - 80, y: view.bounds.height - 30, width: 70, height: 24))
            clearButton.title = "Clear"
            clearButton.bezelStyle = .rounded
            clearButton.autoresizingMask = [.minX, .minY]
            clearButton.target = self
            clearButton.action = #selector(clearConsole(_:))
            
            view.addSubview(clearButton)
            self.clearButton = clearButton
        }
    }
    
    private func showWelcomeMessage() {
        let welcomeMessage = """
        === dsLang IDE Console ===
        Ready to compile and run dsLang programs.
        
        """
        
        appendText(welcomeMessage)
    }
    
    // MARK: - Public Methods
    
    /// Clears the console view
    @IBAction func clearConsole(_ sender: Any? = nil) {
        textView.string = ""
    }
    
    /// Appends text to the console with default formatting
    func appendText(_ text: String) {
        // Create attributed string with standard attributes
        let attributedString = NSAttributedString(string: text, attributes: standardAttributes)
        appendAttributedText(attributedString)
    }
    
    /// Appends error text to the console (red text)
    func appendErrorText(_ text: String) {
        // Create attributed string with error attributes
        let attributedString = NSAttributedString(string: text, attributes: errorAttributes)
        appendAttributedText(attributedString)
    }
    
    /// Appends warning text to the console (orange text)
    func appendWarningText(_ text: String) {
        // Create attributed string with warning attributes
        let attributedString = NSAttributedString(string: text, attributes: warningAttributes)
        appendAttributedText(attributedString)
    }
    
    /// Appends text with syntax highlighting for compiler output
    func appendCompilerOutput(_ text: String) {
        // Process compiler output to highlight errors and warnings
        let lines = text.components(separatedBy: .newlines)
        
        for line in lines {
            if line.contains("error:") {
                appendErrorText(line + "\n")
            } else if line.contains("warning:") {
                appendWarningText(line + "\n")
            } else {
                appendText(line + "\n")
            }
        }
    }
    
    // MARK: - Private Methods
    
    private func appendAttributedText(_ attributedString: NSAttributedString) {
        // Get the current text storage
        let textStorage = textView.textStorage!
        
        // Start editing session
        textStorage.beginEditing()
        
        // Append the new attributed string
        textStorage.append(attributedString)
        
        // End editing session
        textStorage.endEditing()
        
        // Scroll to the end
        scrollToEnd()
    }
    
    private func scrollToEnd() {
        if textView.string.count > 0 {
            let range = NSRange(location: textView.string.count - 1, length: 1)
            textView.scrollRangeToVisible(range)
        }
    }
}
