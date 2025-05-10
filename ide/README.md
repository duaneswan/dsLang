# dsLang IDE

This is a simple web-based IDE for the dsLang programming language. It provides features for editing, compiling, and running dsLang programs, as well as running the dsOS operating system in QEMU.

## Features

- Syntax highlighting for dsLang
- Code editing with basic IDE features
- File management (create, open, save)
- Compilation using the dsLang compiler
- Run compiled dsLang programs
- Run dsOS in QEMU
- Console output for compilation and execution results

## Getting Started

### Prerequisites

- Node.js and npm installed
- dsLang compiler built and available at `../build/dscc`

### Installation

1. Navigate to the `ide` directory:

```bash
cd ide
```

2. Install dependencies:

```bash
npm install
```

3. Start the IDE server:

```bash
npm start
```

4. Open a web browser and navigate to `http://localhost:3000`

## Usage

1. The IDE will open with a default `main.ds` file.
2. You can create new files by clicking the "New" button.
3. Save your work using the "Save" button or Ctrl+S.
4. Compile your dsLang program by clicking "Compile" or pressing F9.
5. Run your compiled program by clicking "Run" or pressing F5.
6. Run the dsOS operating system in QEMU by clicking "Run OS".

## File Structure

- `index.html`: Main IDE interface
- `css/`: CSS stylesheets
  - `style.css`: Main styles for the IDE
  - `codemirror.css`: Styles for the CodeMirror editor
- `js/`: JavaScript files
  - `ide.js`: Main IDE functionality
  - `codemirror.js`: Simplified CodeMirror editor implementation
  - `dsLang-mode.js`: Syntax highlighting for dsLang
- `server.js`: Node.js server for file operations and compilation
- `package.json`: npm package configuration

## Development

To run the IDE in development mode with automatic reloading:

```bash
npm run dev
```

## Integration with dsLang Compiler

The IDE integrates with the dsLang compiler through the server API. The server sends compilation requests to the dsLang compiler executable and captures the output for display in the IDE.

## License

This project is distributed under the MIT License.
