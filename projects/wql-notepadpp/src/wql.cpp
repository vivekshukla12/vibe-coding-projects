// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 Vivek Shukla
#include "wql.h"
#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
namespace wql {
namespace {
enum Kind { Word, String, Symbol, LineComment, BlockComment };
struct Token { Kind kind; std::string text; size_t pos; };
using Tokens = std::vector<Token>;
bool word(unsigned char c) { return std::isalnum(c) || c == '_' || c == '$' || c >= 128; }
std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}
bool comment(const Token& t) { return t.kind == LineComment || t.kind == BlockComment; }
Tokens lex(const std::string& s) {
    Tokens t;
    for (size_t i = 0; i < s.size();) {
        if (std::isspace(static_cast<unsigned char>(s[i]))) { ++i; continue; }
        size_t b = i;
        Kind k = Symbol;
        if (s.compare(i, 2, "--") == 0) {
            k = LineComment;
            while (i < s.size() && s[i] != '\r' && s[i] != '\n') ++i;
        } else if (s.compare(i, 2, "/*") == 0) {
            k = BlockComment;
            size_t end = s.find("*/", i + 2);
            if (end == std::string::npos) throw Error(b, "Unclosed block comment.");
            i = end + 2;
        } else if (s[i] == '\'' || s[i] == '"') {
            k = String; char quote = s[i++]; bool closed = false;
            while (i < s.size()) {
                // Preserve escape-like input verbatim; validation flags it separately.
                if (s[i] == '\\' && i + 1 < s.size()) { i += 2; continue; }
                if (s[i++] == quote) {
                    if (i < s.size() && s[i] == quote) { ++i; continue; }
                    closed = true; break;
                }
            }
            if (!closed) throw Error(b, "Unclosed quoted value.");
        } else if (word(static_cast<unsigned char>(s[i]))) {
            k = Word;
            while (i < s.size() && word(static_cast<unsigned char>(s[i]))) ++i;
        } else {
            ++i;
            const std::set<std::string> pairs = {"<=", ">=", "!=", "<>", "||", "::"};
            if (i < s.size() && pairs.count(s.substr(b, 2))) ++i;
        }
        t.push_back({k, s.substr(b, i - b), b});
    }
    return t;
}
void balanced(const Tokens& t) {
    std::vector<Token> stack;
    for (const auto& x : t) {
        if (x.kind != Symbol) continue;
        if (x.text == "(" || x.text == "{") {
            if (stack.size() >= 128) throw Error(x.pos, "Nesting exceeds 128 levels.");
            stack.push_back(x);
        } else if (x.text == ")" || x.text == "}") {
            if (stack.empty() || stack.back().text != (x.text == ")" ? "(" : "{"))
                throw Error(x.pos, "Mismatched closing delimiter.");
            stack.pop_back();
        }
    }
    if (!stack.empty()) throw Error(stack.back().pos, "Unclosed delimiter.");
}
bool same(const Tokens& a, const Tokens& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].kind != b[i].kind || a[i].text != b[i].text) return false;
    return true;
}
bool needsSpace(const Token& a, const Token& b) {
    if ((a.kind == Word || a.kind == String) && (b.kind == Word || b.kind == String)) return true;
    if (comment(a) || comment(b)) return true;
    const std::set<std::string> pairs = {"--", "/*", "*/", "<=", ">=", "!=", "<>", "||", "::"};
    if (pairs.count(a.text + b.text)) return true;
    if ((a.text == "}" || a.text == ")") && b.kind == Word) return true;
    return false;
}
struct Clause { size_t at, length; int rank; std::string name; };
std::vector<Clause> clauses(const Tokens& t) {
    std::vector<Clause> out; int depth = 0;
    for (size_t i = 0; i < t.size(); ++i) {
        if (t[i].kind == Symbol) {
            if (t[i].text == "(" || t[i].text == "{") ++depth;
            if (t[i].text == ")" || t[i].text == "}") --depth;
        }
        if (depth || t[i].kind != Word) continue;
        std::string n = upper(t[i].text); int rank = -1; size_t length = 1;
        if (n == "PARAMETERS") rank = 0;
        if (n == "SELECT") rank = 1;
        if (n == "FROM") rank = 2;
        if (n == "WHERE") {
            rank = 4;
            if (i + 1 < t.size() && upper(t[i+1].text) == "ON") { n = "WHERE ON"; rank = 3; length = 2; }
        }
        if ((n == "GROUP" || n == "ORDER") && i+1 < t.size() && upper(t[i+1].text) == "BY") {
            rank = n == "GROUP" ? 5 : 7; n += " BY"; length = 2;
        }
        if (n == "HAVING") rank = 6;
        if (n == "LIMIT") rank = 8;
        if (rank >= 0) { out.push_back({i, length, rank, n}); i += length - 1; }
    }
    return out;
}
bool comparison(const std::string& s) { return s == "=" || s == "!=" || s == "<" || s == ">" || s == "<=" || s == ">=" || s == "<>"; }
std::string transform(const std::string& s, bool pretty, const std::string& eol) {
    auto t = lex(s); balanced(t);
    if (t.empty()) return s;
    auto cs = clauses(t);
    std::string out, clause;
    struct Frame { std::string close; bool multiline; int parentIndent; };
    std::vector<Frame> stack;
    int indent = 0;
    bool nextLine = false;
    for (size_t i = 0; i < t.size(); ++i) {
        const auto& x = t[i]; const Token* prev = i ? &t[i-1] : nullptr;
        auto ci = std::find_if(cs.begin(), cs.end(), [&](const Clause& c){ return c.at == i; });
        bool closing = x.kind == Symbol && (x.text == "}" || x.text == ")");
        bool closeLine = false;
        if (closing && !stack.empty()) {
            auto f = stack.back(); stack.pop_back();
            indent = f.parentIndent; closeLine = f.multiline && prev && prev->text != "{" && prev->text != "(";
        }
        std::string gap;
        if (pretty) {
            if (ci != cs.end()) { clause = ci->name; indent = 0; if (i) gap = eol; }
            else if (nextLine || closeLine) gap = eol + std::string(indent * 4, ' ');
            else if (prev && (needsSpace(*prev, x) || prev->text == "," || comparison(prev->text) || comparison(x.text))) gap = " ";
            if (x.text == "(" && prev && upper(prev->text) == "IN") gap = " ";
            if (x.text == "," || x.text == ";" || (closing && !closeLine)) gap.clear();
        } else if (prev && needsSpace(*prev, x)) gap = " ";
        if (prev && prev->kind == LineComment) gap = eol + (pretty ? std::string(indent * 4, ' ') : "");
        // Keep all existing token contents unchanged, including newlines in literals/comments.
        out += gap; out += x.text; nextLine = false;
        if (!pretty) continue;
        if (ci != cs.end() && (clause == "SELECT" || clause == "PARAMETERS")) { indent = 1; nextLine = true; }
        if (x.kind == Symbol && (x.text == "{" || x.text == "(")) {
            bool multiline = x.text == "{" || (prev && upper(prev->text) == "IN");
            stack.push_back({x.text == "{" ? "}" : ")", multiline, indent});
            if (multiline) { ++indent; nextLine = true; }
        }
        if (x.text == "," && ((!stack.empty() && stack.back().multiline) || (stack.empty() && (clause == "SELECT" || clause == "PARAMETERS")))) nextLine = true;
        if (x.kind == LineComment) nextLine = true;
    }
    if (!same(t, lex(out))) throw Error(0, "Transformation could change tokens; document left unchanged.");
    return out;
}
} // namespace
std::string format(const std::string& s, const std::string& eol) { return transform(s, true, eol); }
std::string minify(const std::string& s, const std::string& eol) { return transform(s, false, eol); }
std::vector<Issue> validate(const std::string& source) {
    Tokens t; std::vector<Issue> issues;
    auto add = [&](size_t p, const std::string& m){ if (issues.size() < 50) issues.push_back({p, m}); };
    try { t = lex(source); balanced(t); }
    catch (const Error& e) { return {{e.offset, e.what()}}; }
    t.erase(std::remove_if(t.begin(), t.end(), comment), t.end());
    if (!t.empty() && t.back().text == ";") t.pop_back();
    if (t.empty()) return {{0, "Enter a WQL SELECT query."}};
    auto cs = clauses(t); std::set<int> seen; int previousRank = -1;
    if (cs.empty() || cs.front().at != 0 || (cs.front().rank != 0 && cs.front().rank != 1)) add(0, "Expected SELECT, optionally preceded by PARAMETERS.");
    size_t selectBegin = t.size(), selectEnd = t.size(); bool hasRbo = false;
    for (size_t c = 0; c < cs.size(); ++c) {
        auto x = cs[c]; size_t b = x.at + x.length, e = c+1 < cs.size() ? cs[c+1].at : t.size();
        if (x.rank < previousRank) add(t[x.at].pos, "Clause is out of order: " + x.name + ".");
        if (seen.count(x.rank) && x.rank != 3) add(t[x.at].pos, "Repeated clause: " + x.name + ".");
        seen.insert(x.rank); previousRank = x.rank;
        if (b == e) add(t[x.at].pos, "Missing content after " + x.name + ".");
        if (x.rank == 1) { selectBegin = b; selectEnd = e; }
        if (x.rank == 8 && b < e) {
            const auto& value = t[b].text;
            bool digits = !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char k){return std::isdigit(k);});
            if (!digits || e != b+1 || value.size() > 6 || value == "0" || std::all_of(value.begin(), value.end(), [](char k){return k == '0';}))
                add(t[b].pos, "LIMIT must be an integer from 1 to 999999.");
        }
    }
    if (!seen.count(1)) add(0, "Missing SELECT clause.");
    if (!seen.count(2)) add(source.size(), "Missing FROM clause.");
    if (seen.count(6) && !seen.count(5)) add(0, "HAVING requires GROUP BY.");
    // Parse projection lists to catch missing commas without guessing tenant field names.
    std::function<void(size_t&,size_t,bool)> projection;
    projection = [&](size_t& i, size_t end, bool nested) {
        while (i < end && !(nested && t[i].text == "}")) {
            size_t start = i;
            if (t[i].text == "*") { add(t[i].pos, "SELECT * is not supported by WQL."); ++i; }
            else if (t[i].kind != Word) { add(t[i].pos, "Expected a field or aggregate expression."); ++i; }
            else {
                std::string name = upper(t[i++].text);
                if (i < end && t[i].text == "(") {
                    size_t a = ++i; int depth = 1;
                    while (i < end && depth) { if (t[i].text == "(") ++depth; if (t[i].text == ")") --depth; if (depth) ++i; }
                    if (name == "COUNT" && i > a && !(i == a+2 && upper(t[a].text) == "DISTINCT" && t[a+1].kind == Word)) add(t[a].pos, "Use COUNT() or COUNT(DISTINCT field).");
                    if ((name == "AVG" || name == "SUM" || name == "MIN" || name == "MAX") && !(i == a+1 && t[a].kind == Word)) add(t[start].pos, "Aggregate requires one field argument.");
                    if (i < end) ++i;
                }
                if (i < end && t[i].text == "{") {
                    hasRbo = true; ++i;
                    if (i < end && t[i].text == "}") add(t[i].pos, "Related field selection is empty.");
                    projection(i, end, true);
                    if (i < end && t[i].text == "}") ++i;
                }
                if (i < end && upper(t[i].text) == "AS") {
                    ++i;
                    if (i == end || t[i].kind != Word) add(t[i-1].pos, "Expected an alias after AS.");
                    else ++i;
                }
            }
            if (i == end || (nested && t[i].text == "}")) break;
            if (t[i].text != ",") { add(t[i].pos, "Expected a comma between selected fields (or AS before an alias)."); if (i == start) ++i; }
            else { ++i; if (i == end || (nested && t[i].text == "}")) add(t[i-1].pos, "Trailing comma in field selection."); }
        }
    };
    if (selectBegin < selectEnd) { size_t i = selectBegin; projection(i, selectEnd, false); }
    if (hasRbo && seen.count(5)) add(t[selectBegin].pos, "Related field projections cannot be combined with GROUP BY.");
    for (size_t i = 0; i < t.size(); ++i) {
        auto u = upper(t[i].text);
        if (t[i].kind == String && t[i].text.find('\\') != std::string::npos) add(t[i].pos, "Check this quoted value: WQL does not support escape-character syntax.");
        if (t[i].kind == Symbol && t[i].text == ";") add(t[i].pos, "Validate one query at a time.");
        if (comparison(t[i].text) || u == "AND" || u == "OR" || u == "IN" || u == "CONTAINS" || u == "STARTSWITH" || u == "ENDSWITH") {
            if (i+1 == t.size() || t[i+1].text == ")" || t[i+1].text == "," || comparison(t[i+1].text) || std::any_of(cs.begin(), cs.end(), [&](const Clause& c){return c.at == i+1;}))
                add(t[i].pos, "Incomplete condition or missing value after " + t[i].text + ".");
        }
    }
    std::sort(issues.begin(), issues.end(), [](const Issue& a, const Issue& b){return a.offset < b.offset;});
    return issues;
}
} // namespace wql
