// CodeMirror mode for dsLang

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
  "use strict";

  CodeMirror.defineMode("dsLang", function(config) {
    var indentUnit = config.indentUnit;
    var statementIndent = config.statementIndent;
    var keywords = buildRE([
      "if", "else", "while", "for", "do", "return",
      "break", "continue", "struct", "enum", "int",
      "char", "bool", "long", "short", "unsigned",
      "void", "const", "extern", "static", "true", "false", "null"
    ]);
    
    var types = buildRE([
      "int", "char", "bool", "long", "short", "unsigned",
      "void", "struct", "enum"
    ]);
    
    var operators = /[+\-*&%=<>!?|\/]/;
    var identifiers = /[A-Za-z_][A-Za-z0-9_]*/;
    
    function buildRE(words) {
      return new RegExp("^(?:" + words.join("|") + ")\\b");
    }
    
    function tokenBase(stream, state) {
      // Handle comments
      if (stream.match("//")) {
        stream.skipToEnd();
        return "comment";
      }
      
      if (stream.match("/*")) {
        state.tokenize = tokenComment;
        return tokenComment(stream, state);
      }
      
      // Handle string literals
      if (stream.match('"')) {
        state.tokenize = tokenString;
        return tokenString(stream, state);
      }
      
      // Handle character literals
      if (stream.match("'")) {
        state.tokenize = tokenChar;
        return tokenChar(stream, state);
      }
      
      // Handle numbers
      if (stream.match(/^-?[0-9\.]+/))
        return "number";
      
      if (stream.match(/^0x[0-9a-fA-F]+/))
        return "number";
      
      // Handle operators
      if (stream.match(operators))
        return "operator";
      
      // Handle delimiters
      if (stream.match(/[;,(){}\[\]\.]/))
        return "punctuation";
      
      // Handle identifiers and keywords
      if (stream.match(identifiers)) {
        var word = stream.current();
        if (keywords.test(word))
          return "keyword";
        if (types.test(word))
          return "type";
        return "variable";
      }
      
      // Handle everything else
      stream.next();
      return null;
    }
    
    function tokenComment(stream, state) {
      var maybeEnd = false, ch;
      while (ch = stream.next()) {
        if (maybeEnd && ch == "/") {
          state.tokenize = tokenBase;
          break;
        }
        maybeEnd = (ch == "*");
      }
      return "comment";
    }
    
    function tokenString(stream, state) {
      var escaped = false, ch;
      while ((ch = stream.next()) != null) {
        if (ch == '"' && !escaped) {
          state.tokenize = tokenBase;
          break;
        }
        escaped = !escaped && ch == "\\";
      }
      return "string";
    }
    
    function tokenChar(stream, state) {
      var escaped = false, ch;
      while ((ch = stream.next()) != null) {
        if (ch == "'" && !escaped) {
          state.tokenize = tokenBase;
          break;
        }
        escaped = !escaped && ch == "\\";
      }
      return "string-2";
    }
    
    // Interface
    return {
      startState: function() {
        return {
          tokenize: tokenBase,
          context: null,
          indented: 0,
          startOfLine: true
        };
      },
      
      token: function(stream, state) {
        if (stream.sol()) {
          state.startOfLine = true;
          state.indented = stream.indentation();
        }
        
        if (stream.eatSpace()) return null;
        
        var style = state.tokenize(stream, state);
        state.startOfLine = false;
        return style;
      },
      
      indent: function(state, textAfter) {
        if (state.tokenize != tokenBase) return CodeMirror.Pass;
        
        var ctx = state.context;
        var firstChar = textAfter && textAfter.charAt(0);
        if (ctx && firstChar == ctx.type) {
          return ctx.indented;
        }
        
        var closing = firstChar == "}";
        if (ctx && closing) ctx = ctx.prev;
        
        return ctx ? ctx.indented + indentUnit : 0;
      },
      
      electricInput: /^\s*[{}]$/,
      lineComment: "//",
      blockCommentStart: "/*",
      blockCommentEnd: "*/",
      fold: "brace"
    };
  });
  
  CodeMirror.defineMIME("text/x-dslang", "dsLang");
  // For legacy modes
  CodeMirror.defineMIME("application/x-dslang", "dsLang");
});
