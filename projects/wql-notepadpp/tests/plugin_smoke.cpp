// SPDX-License-Identifier: GPL-3.0-or-later
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "PluginInterface.h"
#include <iostream>
int main(int argc,char** argv) {
    if(argc!=2) return 1;
    auto lib=LoadLibraryA(argv[1]);
    if(!lib) {std::cerr<<"DLL load failed: "<<GetLastError();return 2;}
    for(const auto name:{"setInfo","getName","getFuncsArray","beNotified","messageProc","isUnicode"})
        if(!GetProcAddress(lib,name)) {std::cerr<<"Missing export "<<name;return 3;}
    auto getItems=reinterpret_cast<FuncItem* (__cdecl*)(int*)>(GetProcAddress(lib,"getFuncsArray"));
    int count=0;auto items=getItems(&count);
    if(count!=5) return 4;
    for(int i=0;i<3;++i) if(!items[i]._pFunc || !items[i]._pShKey || !items[i]._pShKey->_isCtrl) return 5;
    FreeLibrary(lib); std::cout<<"DLL loaded; command exports and shortcuts verified.\n";return 0;
}
