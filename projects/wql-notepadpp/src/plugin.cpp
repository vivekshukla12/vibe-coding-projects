// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Vivek Shukla
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "PluginInterface.h"
#include "wql.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <cwchar>
namespace {
NppData npp;
const wchar_t* title = L"WQL Tools";
constexpr size_t maxBytes = 10 * 1024 * 1024;
ShortcutKey shortcuts[] = {{true,true,true,'W'}, {true,true,true,'M'}, {true,true,true,'V'}};
FuncItem items[5];
HWND editor() {
    int which = 0;
    SendMessage(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&which));
    return which ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
}
LRESULT sci(HWND h, UINT m, WPARAM w = 0, LPARAM l = 0) { return SendMessage(h, m, w, l); }
std::wstring wide(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(n, L' ');
    if (n) MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &out[0], n);
    return out;
}
struct Input { HWND view; Sci_Position start, end; bool selected; std::string text, eol; };
Input input() {
    HWND h = editor();
    if (sci(h, SCI_GETSELECTIONS) != 1 || sci(h, SCI_SELECTIONISRECTANGLE))
        throw std::runtime_error("Use a single ordinary selection, or clear the selection to process the whole document.");
    Sci_Position a = sci(h, SCI_GETSELECTIONSTART), b = sci(h, SCI_GETSELECTIONEND);
    bool selected = a != b;
    if (!selected) { a = 0; b = sci(h, SCI_GETLENGTH); }
    if (b < a || static_cast<size_t>(b-a) > maxBytes) throw std::runtime_error("Select a query smaller than 10 MiB.");
    std::vector<char> bytes(static_cast<size_t>(b-a)+1, 0);
    Sci_TextRangeFull range{{a,b}, bytes.data()};
    sci(h, SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&range));
    int mode = static_cast<int>(sci(h, SCI_GETEOLMODE));
    return {h, a, b, selected, std::string(bytes.data(), static_cast<size_t>(b-a)), mode == SC_EOL_CRLF ? "\r\n" : mode == SC_EOL_CR ? "\r" : "\n"};
}
void show(const std::string& text, UINT flags = MB_OK | MB_ICONINFORMATION) {
    MessageBoxW(npp._nppHandle, wide(text).c_str(), title, flags);
}
void transform(bool pretty) {
    try {
        auto in = input();
        if (sci(in.view, SCI_GETREADONLY)) throw std::runtime_error("This document is read-only.");
        auto out = pretty ? wql::format(in.text,in.eol) : wql::minify(in.text,in.eol);
        if (out == in.text) return;
        sci(in.view, SCI_BEGINUNDOACTION);
        sci(in.view, SCI_SETTARGETSTART, static_cast<WPARAM>(in.start));
        sci(in.view, SCI_SETTARGETEND, static_cast<WPARAM>(in.end));
        sci(in.view, SCI_REPLACETARGET, out.size(), reinterpret_cast<LPARAM>(out.data()));
        sci(in.view, SCI_ENDUNDOACTION);
        sci(in.view, SCI_SETSEL, static_cast<WPARAM>(in.start), static_cast<LPARAM>(in.start + out.size()));
    } catch (const std::exception& e) { show(std::string(e.what()) + "\n\nNo changes made.", MB_OK | MB_ICONWARNING); }
    catch (...) { show("Unexpected error. No query correction was performed.", MB_OK | MB_ICONERROR); }
}
void formatCommand() { transform(true); }
void minifyCommand() { transform(false); }
void validateCommand() {
    try {
        auto in = input(); auto issues = wql::validate(in.text);
        if (issues.empty()) {
            show("No issues found by the implemented local checks.\n\nThis is not full Workday validation. Field names, data sources, types, security and tenant-specific rules are not checked.");
            return;
        }
        std::ostringstream message;
        message << "Local validation: " << issues.size() << " potential issue(s).\n\n";
        for (size_t i=0; i < std::min<size_t>(issues.size(),20); ++i) {
            auto pos = in.start + static_cast<Sci_Position>(issues[i].offset);
            auto line = sci(in.view, SCI_LINEFROMPOSITION, static_cast<WPARAM>(pos));
            auto col = sci(in.view, SCI_GETCOLUMN, static_cast<WPARAM>(pos));
            message << "Line " << line+1 << ", column " << col+1 << ": " << issues[i].message << "\n";
        }
        if (issues.size() > 20) message << "Additional issues omitted.\n";
        message << "\nQuery unchanged. The cursor will move to the first issue.\nThese are local diagnostics, not Workday server messages.";
        show(message.str(), MB_OK | MB_ICONWARNING);
        sci(in.view, SCI_GOTOPOS, static_cast<WPARAM>(in.start + issues.front().offset));
        sci(in.view, SCI_GRABFOCUS);
    } catch (const std::exception& e) { show(e.what(), MB_OK | MB_ICONERROR); }
    catch (...) { show("Unexpected validation error.", MB_OK | MB_ICONERROR); }
}
void colorsHelp() {
    show("Import one bundled syntax file:\n\nLanguage > User Defined Language > Define your language > Import\n\nChoose WQL-Light.xml or WQL-Dark.xml, then select Language > WQL. Files ending in .wql use it automatically. Import only one theme; remove the old WQL definition when switching.\n\nThe DLL's formatting and validation commands work with any file extension.");
}
void about() { show("WQL Tools 0.1.0\nCopyright (c) 2026 Vivek Shukla\nGPL-3.0-or-later\n\nFormat, Minify and Validate locally. No network requests or AI service.\n\nShortcuts: Settings > Shortcut Mapper > Plugin commands.\nDefault: Ctrl+Alt+Shift+W / M / V. Resolve any shortcut conflicts there."); }
void setup() {
    const wchar_t* names[] = {L"Format WQL",L"Minify WQL",L"Validate WQL",L"Syntax colors - installation help",L"About WQL Tools"};
    PFUNCPLUGINCMD callbacks[] = {formatCommand,minifyCommand,validateCommand,colorsHelp,about};
    for(int i=0;i<5;++i) { std::wcsncpy(items[i]._itemName,names[i],menuItemSize-1); items[i]._pFunc=callbacks[i]; items[i]._pShKey=i<3?&shortcuts[i]:nullptr; }
}
}
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return title; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) { setup(); *count=5; return items; }
extern "C" __declspec(dllexport) void beNotified(SCNotification*) {}
extern "C" __declspec(dllexport) LRESULT messageProc(UINT,WPARAM,LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
BOOL APIENTRY DllMain(HMODULE,DWORD,LPVOID) { return TRUE; }
