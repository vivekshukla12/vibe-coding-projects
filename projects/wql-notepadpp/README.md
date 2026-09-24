# WQL Tools for Notepad++

Version 0.1.0 — a native Windows plugin for formatting, minifying and checking
Workday Query Language. Runs offline with no PythonScript or AI dependency.

## Commands

| Command | Default shortcut | Behavior |
| --- | --- | --- |
| Format WQL | Ctrl+Alt+Shift+W | Indent selected fields and related object braces; separate clauses. |
| Minify WQL | Ctrl+Alt+Shift+M | Remove optional whitespace while preserving literals and comments. |
| Validate WQL | Ctrl+Alt+Shift+V | Report local syntax diagnostics with line/column locations. |

Commands use a single ordinary selection, or the whole document when nothing
is selected. Format and Minify preserve keyword case, identifiers, literals,
comments, order and logic. One Undo restores the previous text. Multiple and
rectangular selections are rejected to avoid editing the wrong range. Edits
are refused for read-only documents, unclosed strings, unbalanced delimiters,
queries over 10 MiB, or nesting beyond 128 levels.

Assign different keys in **Settings > Shortcut Mapper > Plugin commands**.
Built-in commands or other plugins may already use these combinations. Select
the command and check the conflict message, then modify or clear the conflicting
assignment.

The Format default changed from **Ctrl+Alt+Shift+F** to **Ctrl+Alt+Shift+W**
after a conflict with a built-in command was reported. Existing installations
may retain saved shortcut assignments after replacing the DLL; if Format still
uses F, change it to W in Shortcut Mapper. If you already changed it to W, no
further shortcut change is needed.

## Download and install

1. Open the repository's **Actions > Build WQL Tools** and select a successful
   run. Download the artifact matching your **Notepad++ architecture**:
   `WqlTools-x64` for 64-bit or `WqlTools-Win32` for 32-bit. GitHub artifact
   downloads require signing in. This is a manual third-party installation;
   the plugin has not been submitted to Plugins Admin.
2. Extract the ZIP. Close Notepad++.
3. Copy the enclosed `WqlTools` folder into Notepad++'s `plugins` directory,
   normally `C:\Program Files\Notepad++\plugins\`. The final path must be
   `plugins\WqlTools\WqlTools.dll`. For portable installations use their own
   `plugins` directory. A protected installation directory may request admin access.
4. Start Notepad++. **Plugins > WQL Tools** should appear.
5. For colors: **Language > User Defined Language > Define your language... >
   Import...**, then select `WqlTools\syntax\WQL-Light.xml` or `WQL-Dark.xml`.
   Import only one. Choose **Language > WQL** for an open query. `.wql` files
   select it automatically. Both include brace folding. If you installed the
   earlier prototype's WQL definition, remove that definition before importing.

The DLL and syntax definition are separate parts of this package. Formatting
and validation work with any extension. To switch themes, remove the existing
WQL definition in the same dialog, then import the other file. Colors can be
customized in that dialog.

Supported build targets: x64 and Win32, Notepad++ 8.5 or newer. ARM64 is not
included. The native builds use the static MSVC runtime. Download artifacts only
from successful build runs. This first release still needs a manual visual
check in your actual Notepad++ setup.

## Validation scope

Checks include balanced quotes/brackets, required SELECT/FROM, clause order,
repeated clauses, missing commas and empty related field selections, SELECT *,
common aggregate argument mistakes, dangling operators, HAVING without GROUP BY,
related projections combined with GROUP BY, and LIMIT bounds.

This is a conservative partial validator, not a complete Workday parser. A
successful check means **no issues found by implemented local checks**. It
cannot verify tenant aliases, types, data sources, security, relationships or
all valid/invalid expression combinations. Diagnostics are locally generated,
not quotations of Workday server messages. Validation never fixes query text.

Minify retains a newline after `--` comments and retains embedded newlines in
quoted values and block comments. It will therefore not always produce one
physical line. SQL-style comments and escape-like string input are preserved
as text; preservation does not imply that Workday accepts that syntax.

WQL checks were informed by the existing project's syntax baseline:
https://github.com/vivekshukla12/workday-ai-skills/blob/main/skills/wql-formatter/references/wql-syntax.md

## Build and test

On Windows install Visual Studio 2022 with Desktop development with C++ and CMake:

```powershell
cmake -S projects/wql-notepadpp -B build/wql -A x64
cmake --build build/wql --config Release
ctest --test-dir build/wql -C Release --output-on-failure
cmake --install build/wql --config Release --prefix dist
```

Use `-A Win32` in a separate build directory for 32-bit Notepad++.
On Linux/macOS, CMake builds the platform-independent core and its tests only.
CI also loads each Windows DLL and checks the exported commands and shortcuts.

Manual acceptance: open `examples/sample.wql`, apply colors, Format, Undo,
Minify, and Validate. Repeat with a selected query and in Notepad++'s second
view. Verify an invalid query reports a line/column and leaves the text intact.

## License

**GPL-3.0-or-later** applies to this project, including its original source and
syntax definitions. See [LICENSE](LICENSE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Bundled headers retain their
original notices. The repository-root MIT license applies to other projects
only where they do not specify a different license.
