# dsLang IDE

A native macOS IDE for the dsLang programming language.

## Overview

dsLang IDE is a powerful, native macOS application built with Swift and Cocoa, designed specifically for developing programs in the dsLang programming language. It provides a modern development environment with essential features like syntax highlighting, file navigation, and integrated compilation.

## Features

- **Native macOS Experience**: Built with Swift and Cocoa for optimal performance and native look and feel.
- **Code Editor**: Advanced text editing with dsLang-specific syntax highlighting, line numbers, and customizable editor preferences.
- **Project Navigation**: File browser sidebar for easy project navigation and file management.
- **Integrated Compilation**: Compile and run dsLang programs directly from the IDE.
- **Console Output**: Dedicated console for viewing compilation results and program output.
- **OS Development Support**: Built-in support for dsOS development with QEMU integration.
- **Customizable Preferences**: Configurable settings for compiler paths, build directories, and editor appearance.

## Requirements

- macOS 13.0 or later
- Xcode 14.0 or later (for development)
- dsLang compiler installed

## Building the Project

1. Open `dsLang_IDE.xcodeproj` in Xcode.
2. Configure the signing options to match your developer account.
3. Build the project (Cmd+B) and run (Cmd+R).

## Usage

### Creating a New File

1. Select `File > New` from the menu or press `Cmd+N`.
2. The editor will open with a new file containing a template for a dsLang program.
3. Save the file with a `.ds` extension.

### Opening a File

1. Select `File > Open` from the menu or press `Cmd+O`.
2. Browse to and select a dsLang source file to open.

### Compiling and Running

1. With a dsLang file open in the editor, click the `Compile` button or press `Cmd+Shift+B`.
2. To run the compiled program, click the `Run` button or press `Cmd+R`.
3. The console panel will show compilation results and program output.

### Running in QEMU

If you're developing for dsOS, you can run your kernel in QEMU by clicking the `Run OS` button.

### Customizing Preferences

1. Open `Preferences` from the menu or press `Cmd+,`.
2. Configure compiler path, build directory, editor theme, and other settings.

## Project Structure

- **AppDelegate.swift**: Application lifecycle management.
- **ViewController.swift**: Main view controller for the editor interface.
- **EditorView.swift**: Custom view for code editing with syntax highlighting.
- **SidebarViewController.swift**: File browser and project navigation.
- **ConsoleViewController.swift**: Output console for compiler and runtime messages.
- **CompilerService.swift**: Service for compiling and running dsLang programs.
- **DSLangSyntaxHighlighter.swift**: Syntax highlighting for dsLang code.
- **DocumentController.swift**: Document management system.
- **PreferencesWindowController.swift**: Settings management.

## Development

### Adding Features

The project follows standard MVC architecture:

- **Models**: Document state and compiler interactions are managed by service classes.
- **Views**: Custom views for code editing and UI elements.
- **Controllers**: View controllers for different parts of the application.

When adding new features, follow the established patterns and use Swift best practices.

### Building for Distribution

1. Select the "Archive" option from the Product menu.
2. Follow the Xcode distribution process to create a signed application.

## License

This project is distributed under the MIT License.
