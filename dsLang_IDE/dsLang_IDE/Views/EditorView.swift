import Cocoa

class EditorView: NSView {
    
    // MARK: - Properties
    
    // Text storage and layout
    private var textView: NSTextView!
    private var scrollView: NSScrollView!
    private var textStorage: NSTextStorage!
    private var layoutManager: NSLayoutManager!
    private var textContainer: NSTextContainer!
    
    // Line number view
    private var lineNumberView: LineNumberView?
    
    // Syntax highlighter
    private let syntaxHighlighter = DSLangSyntaxHighlighter()
    
    // Editor preferences
    var tabSize: Int = 4
    var useSpacesForTabs: Bool = true
    var showLineNumbers: Bool = true {
        didSet {
            lineNumberView?.isHidden = !showLineNumbers
        }
    }
    
    // Text content
    var text: String {
        get {
            return textView.string
        }
        set {
            textView.string = newValue
            highlightSyntax()
        }
    }
    
    // Callback for text changes
    var textDidChangeHandler: ((String) -> Void)?
    
    // MARK: - Initialization
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        setupTextView()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        setupTextView()
    }
    
    private func setupTextView() {
        // Create the text storage with attributed string
        textStorage = NSTextStorage()
        
        // Create the layout manager
        layoutManager = NSLayoutManager()
        textStorage.addLayoutManager(layoutManager)
        
        // Create the text container with size that tracks view frame
        textContainer = NSTextContainer(containerSize: frame.size)
        textContainer.widthTracksTextView = true
        textContainer.heightTracksTextView = true
        layoutManager.addTextContainer(textContainer)
        
        // Create scroll view
        scrollView = NSScrollView(frame: bounds)
        scrollView.borderType = .noBorder
        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = true
        scrollView.autoresizingMask = [.width, .height]
        
        // Create text view
        textView = NSTextView(frame: bounds, textContainer: textContainer)
        textView.autoresizingMask = [.width, .height]
        textView.isEditable = true
        textView.isRichText = false
        textView.allowsUndo = true
        textView.isAutomaticQuoteSubstitutionEnabled = false
        textView.isAutomaticLinkDetectionEnabled = false
        textView.isAutomaticDashSubstitutionEnabled = false
        textView.enabledTextCheckingTypes = 0  // Disable spell checking
        textView.isAutomaticSpellingCorrectionEnabled = false
        textView.isAutomaticTextReplacementEnabled = false
        textView.usesFontPanel = false
        textView.backgroundColor = NSColor.textBackgroundColor
        textView.textColor = NSColor.textColor
        textView.font = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
        
        // Set text view as document view of scroll view
        scrollView.documentView = textView
        
        // Add scroll view to this view
        addSubview(scrollView)
        
        // Add line number view if enabled
        if showLineNumbers {
            setupLineNumberView()
        }
        
        // Set up notification for text changes
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(textDidChange(_:)),
            name: NSText.didChangeNotification,
            object: textView
        )
        
        // Set up tab handling
        setupTabHandling()
    }
    
    private func setupLineNumberView() {
        let lineNumberView = LineNumberView(scrollView: scrollView)
        scrollView.verticalRulerView = lineNumberView
        scrollView.hasVerticalRuler = true
        scrollView.rulersVisible = true
        self.lineNumberView = lineNumberView
    }
    
    private func setupTabHandling() {
        // Add a key event monitor to handle tabs
        NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self = self else { return event }
            guard let characters = event.charactersIgnoringModifiers else { return event }
            
            // Handle tab key press
            if characters == "\t" {
                if self.useSpacesForTabs {
                    // Insert spaces instead of tab
                    let spaces = String(repeating: " ", count: self.tabSize)
                    self.textView.insertText(spaces, replacementRange: self.textView.selectedRange())
                    return nil
                }
            }
            
            return event
        }
    }
    
    // MARK: - Syntax Highlighting
    
    private func highlightSyntax() {
        guard let textStorage = textStorage else { return }
        let text = textView.string
        
        // Reset attributes
        let range = NSRange(location: 0, length: text.count)
        textStorage.setAttributes([
            .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular),
            .foregroundColor: NSColor.textColor
        ], range: range)
        
        // Apply syntax highlighting
        syntaxHighlighter.highlightSyntax(in: textStorage, for: text)
    }
    
    // MARK: - Notifications
    
    @objc private func textDidChange(_ notification: Notification) {
        highlightSyntax()
        textDidChangeHandler?(textView.string)
    }
    
    // MARK: - Memory Management
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
}

// MARK: - Line Number View

class LineNumberView: NSRulerView {
    
    private var font: NSFont = NSFont.monospacedSystemFont(ofSize: 11, weight: .regular)
    
    init(scrollView: NSScrollView) {
        super.init(scrollView: scrollView, orientation: .verticalRuler)
        self.clientView = scrollView.documentView
        self.ruleThickness = 40.0
    }
    
    required init(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    override func drawHashMarksAndLabels(in rect: NSRect) {
        guard let textView = self.clientView as? NSTextView else { return }
        guard let layoutManager = textView.layoutManager else { return }
        guard let textContainer = textView.textContainer else { return }
        
        let visibleRect = scrollView?.contentView.bounds ?? .zero
        let relativePoint = convert(NSPoint.zero, from: scrollView?.contentView)
        
        // Set the ruler color
        NSColor.secondaryLabelColor.set()
        
        let text = textView.string as NSString
        let lineRange = text.lineRange(for: NSRange(location: 0, length: 0))
        var glyphRange = layoutManager.glyphRange(forCharacterRange: lineRange, actualCharacterRange: nil)
        
        var lineNumber = 1
        
        while NSLocationInRange(glyphRange.location, layoutManager.glyphRange(for: textContainer)) {
            var lineRect = layoutManager.boundingRect(forGlyphRange: glyphRange, in: textContainer)
            lineRect.origin.x = 0
            lineRect.origin.y += relativePoint.y
            
            if lineRect.intersects(visibleRect) {
                let lineNumberString = "\(lineNumber)" as NSString
                let attributes: [NSAttributedString.Key: Any] = [
                    .font: font,
                    .foregroundColor: NSColor.secondaryLabelColor
                ]
                
                let stringSize = lineNumberString.size(withAttributes: attributes)
                let x = ruleThickness - stringSize.width - 5
                let y = lineRect.minY
                
                lineNumberString.draw(at: NSPoint(x: x, y: y), withAttributes: attributes)
            }
            
            if let newlineLocation = text.range(of: "\n", options: [], range: NSRange(location: NSMaxRange(lineRange), length: text.length - NSMaxRange(lineRange))).location {
                lineRange.location = newlineLocation + 1
                lineRange.length = text.length - lineRange.location
            } else {
                break
            }
            
            glyphRange = layoutManager.glyphRange(forCharacterRange: lineRange, actualCharacterRange: nil)
            lineNumber += 1
        }
    }
    
    override var requiredThickness: CGFloat {
        return 40.0
    }
}
