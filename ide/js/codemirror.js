// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/5/LICENSE

// This is a simplified version of CodeMirror for the dsLang IDE
// In a real implementation, you would include the full CodeMirror library

(function(global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
  typeof define === 'function' && define.amd ? define(factory) :
  (global = global || self, global.CodeMirror = factory());
}(this, function() {
  'use strict';

  // CONSTRUCTOR
  function CodeMirror(place, options) {
    if (!(this instanceof CodeMirror)) return new CodeMirror(place, options);

    this.options = options = options ? copyObj(options) : {};
    copyObj(defaults, options, false);

    // Set up DOM structure
    this.display = makeDisplay(place);
    this.state = {
      keyMaps: [],
      overlays: [],
      modeGen: 0,
      overwrite: false,
      delayingBlurEvent: false,
      focused: false,
      suppressEdits: false,
      pasteIncoming: -1, cutIncoming: -1,
      draggingText: false,
      highlight: new Delayed()
    };

    // Initialize with content
    if (options.value) {
      this.display.textarea.value = options.value;
      this.display.textContent.textContent = options.value;
    }

    // Set up event handlers
    this.setupEvents();

    // Display line numbers if option is set
    if (options.lineNumbers) {
      this.display.lineNumbers.style.display = "block";
      updateLineNumbers(this);
    }
  }

  // Helper functions
  function copyObj(obj, target, overwrite) {
    if (!target) target = {};
    for (var prop in obj)
      if (obj.hasOwnProperty(prop) && (overwrite !== false || !target.hasOwnProperty(prop)))
        target[prop] = obj[prop];
    return target;
  }

  function makeDisplay(place) {
    let wrapper = document.createElement("div");
    wrapper.className = "CodeMirror";
    
    let textareaWrap = document.createElement("div");
    textareaWrap.className = "CodeMirror-textarea-wrap";
    
    let textarea = document.createElement("textarea");
    textarea.setAttribute("autocorrect", "off");
    textarea.setAttribute("autocapitalize", "off");
    textarea.setAttribute("spellcheck", "false");
    textarea.style.display = "none";
    textareaWrap.appendChild(textarea);
    
    let content = document.createElement("div");
    content.className = "CodeMirror-content";
    
    let textContent = document.createElement("pre");
    textContent.className = "CodeMirror-code";
    content.appendChild(textContent);
    
    let lineNumbers = document.createElement("div");
    lineNumbers.className = "CodeMirror-linenumbers";
    lineNumbers.style.display = "none";
    
    wrapper.appendChild(textareaWrap);
    wrapper.appendChild(content);
    wrapper.appendChild(lineNumbers);
    
    if (typeof place === "string") {
      place = document.getElementById(place);
    }
    place.appendChild(wrapper);
    
    return {
      wrapper: wrapper,
      textarea: textarea,
      textContent: textContent,
      content: content,
      lineNumbers: lineNumbers
    };
  }

  // Update line numbers
  function updateLineNumbers(cm) {
    let lineCount = cm.display.textarea.value.split("\n").length;
    let lineNumbersContent = "";
    for (let i = 1; i <= lineCount; i++) {
      lineNumbersContent += i + "<br>";
    }
    cm.display.lineNumbers.innerHTML = lineNumbersContent;
  }

  // Set up event handlers
  CodeMirror.prototype.setupEvents = function() {
    let cm = this;
    
    this.display.textarea.addEventListener("input", function() {
      cm.setValue(cm.display.textarea.value);
      if (cm.options.onChange) {
        cm.options.onChange(cm);
      }
    });
    
    this.display.textContent.addEventListener("click", function() {
      cm.display.textarea.focus();
    });
    
    this.display.textarea.addEventListener("keydown", function(e) {
      if (cm.options.extraKeys) {
        let key = CodeMirror.keyName(e);
        let command = cm.options.extraKeys[key];
        if (command) {
          if (typeof command == "function") command(cm);
          e.preventDefault();
        }
      }
    });
    
    this.display.textarea.addEventListener("focus", function() {
      cm.state.focused = true;
      cm.display.wrapper.classList.add("CodeMirror-focused");
    });
    
    this.display.textarea.addEventListener("blur", function() {
      cm.state.focused = false;
      cm.display.wrapper.classList.remove("CodeMirror-focused");
    });
  };

  // Get the editor content
  CodeMirror.prototype.getValue = function() {
    return this.display.textarea.value;
  };

  // Set the editor content
  CodeMirror.prototype.setValue = function(val) {
    this.display.textarea.value = val;
    this.display.textContent.textContent = val;
    updateLineNumbers(this);
  };

  // Set the cursor position
  CodeMirror.prototype.setCursor = function(line, ch) {
    // Simple implementation, just move cursor to beginning
    this.display.textarea.focus();
  };

  // Get the cursor position
  CodeMirror.prototype.getCursor = function() {
    // Simple implementation, just pretend cursor is at 0,0
    return {line: 0, ch: 0};
  };

  // Register an event listener
  CodeMirror.prototype.on = function(type, func) {
    let callbacks = this._handlers || (this._handlers = {});
    let arr = callbacks[type] || (callbacks[type] = []);
    arr.push(func);
  };

  // Helper for key name
  CodeMirror.keyName = function(event) {
    let base = event.key || String.fromCharCode(event.charCode || event.keyCode);
    let name = base;
    if (event.ctrlKey) name = "Ctrl-" + name;
    if (event.altKey) name = "Alt-" + name;
    if (event.metaKey) name = "Cmd-" + name;
    if (event.shiftKey) name = "Shift-" + name;
    return name;
  };

  // Delayed function
  function Delayed() {
    this.id = null;
    this.prev = 0;
  }
  Delayed.prototype.set = function(ms, f) {
    clearTimeout(this.id);
    this.id = setTimeout(f, ms);
  };

  // Default options
  var defaults = {
    lineNumbers: false,
    lineWrapping: false,
    mode: null,
    theme: null
  };

  // Define mode
  CodeMirror.defineMode = function(name, mode) {
    CodeMirror.modes = CodeMirror.modes || {};
    CodeMirror.modes[name] = mode;
  };

  // Define MIME
  CodeMirror.defineMIME = function(mime, spec) {
    CodeMirror.mimeModes = CodeMirror.mimeModes || {};
    CodeMirror.mimeModes[mime] = spec;
  };

  // Export CodeMirror
  return CodeMirror;
}));
