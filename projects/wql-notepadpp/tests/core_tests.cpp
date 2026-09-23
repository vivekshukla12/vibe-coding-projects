// SPDX-License-Identifier: GPL-3.0-or-later
#include "wql.h"
#include <iostream>
#include <stdexcept>
void require(bool condition,const char* why) { if(!condition) throw std::runtime_error(why); }
int main() {
try {
    const std::string q="SELECT employeeID,primaryHomeAddress{city,postalCode} AS address FROM allWorkers WHERE activeStatus=true";
    const std::string expected="SELECT\n    employeeID,\n    primaryHomeAddress{\n        city,\n        postalCode\n    } AS address\nFROM allWorkers\nWHERE activeStatus = true";
    require(wql::format(q)==expected,"nested layout");
    require(wql::format(expected)==expected,"format idempotence");
    require(wql::minify(expected)==q,"minify layout and content");
    require(wql::validate(q).empty(),"valid basic query");
    const std::string cases[]={
        "PARAMETERS prompt='A  B' SELECT worker,dependents{name,age} FROM indexedAllWorkers(dataSourceFilter=filter,isActive=true) WHERE ON dependents age>5 WHERE worker IN (Reference_ID_Type=Reference_ID) LIMIT 100",
        "SELECT location,COUNT() AS total FROM allWorkers GROUP BY location HAVING COUNT()>100 ORDER BY total DESC LIMIT 99",
        "SELECT name FROM workers WHERE name IN ('München','A  B')",
        "SELECT a -- keep this\r\nFROM x /* block\r\ncomment */ WHERE a='literal\nline'",
        "SELECT a,b{c,d{e,f} AS deeper} FROM x WHERE a=-1.5 AND b>=2",
        "SELECT a FROM x WHERE a='x, { FROM }'"
    };
    for(const auto& s:cases) {
        const auto m=wql::minify(s),f=wql::format(s);
        require(wql::minify(f)==m,"format/minify token round trip");
        require(wql::format(f)==f,"idempotence across cases");
        require(wql::validate(s).empty(),"valid syntax case");
    }
    auto windows=wql::format("SELECT a FROM x WHERE a='first\nsecond'","\r\n");
    require(windows.find("'first\nsecond'")!=std::string::npos,"literal newline preserved");
    require(windows.find("SELECT\r\n")!=std::string::npos,"document CRLF");
    require(wql::minify("SELECT a --note\nFROM x").find("--note\nFROM")!=std::string::npos,"line comment terminator");
    require(wql::minify("SELECT a FROM x WHERE a - - 1").find("- -")!=std::string::npos,"avoid creating a comment");
    const std::string invalid[]={"SELECT a b FROM x","SELECT a, FROM x","SELECT a{} FROM x","SELECT * FROM x","SELECT a","SELECT a FROM x WHERE a=","SELECT a FROM x WHERE a=1 AND","SELECT a FROM x LIMIT 0","SELECT COUNT(a) FROM x","SELECT a FROM x HAVING COUNT()>1","SELECT a FROM x WHERE a=1 WHERE ON r b=2","SELECT a{b} FROM x GROUP BY a","SELECT a{b FROM x","SELECT 'oops"};
    for(const auto& s:invalid) { if(wql::validate(s).empty()) throw std::runtime_error("missed invalid query: "+s); }
    bool blocked=false;try { wql::format("SELECT a{b FROM x"); } catch(const wql::Error&) { blocked=true; }
    require(blocked,"unbalanced edit refused");
    std::cout << "All WQL core tests passed.\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
