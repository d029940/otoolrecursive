/****************************************************************************
**
** Copyright (C) 2020 Manfred Kern. All rights reserved.
** Contact: manfred.kern@gmail.com
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** 3. All advertising materials mentioning features or use of this software
**    must display the following acknowledgement:
**    This product includes software developed by the the organization.
**
** 4. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>

using namespace std;

void containsLibs(const string &dylib);

const string OTOOL{"otool -L "};
const string HOMEBREW{"/opt/homebrew"}; // TODO: read from environment
const string MACPORTS{"/opt/local"};    // TODO: read from environment
const string XCODE{"/Applications/Xcode.app"};
const string SYSTEM_LIBS{"/System/Library/"};
const string LOCAL_LIBS{"/usr/local"};
set<string> packageLibs;
const string RPATH{"@rpath"};
set<string> rpathLibs;
const string LOADER_PATH{"@loader_path"};
set<string> loaderLibs;
const string EXECUTABLE_PATH{"@executable_path"};
set<string> executableLibs;
set<string> otherLibs;

vector<string> options{}; // command line options
    // (-v verbose, -p package libs -r rpath libs, -e executable libs, -l loader_path libs, -a all libs)

string trim(const string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

///
/// \brief firstWord - Assumption str starts with non whitespace character
/// \param str String where to extract first word (ending by \0)
///
void firstWord(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (isspace(str[i])) {
            str[i] = '\0';
        }
    };
    str[len] = '\0';
}

///
/// \brief startsWith
/// \param str - Complete string
/// \param pre - partial string to compare with str from the start
/// \return true, if str starts with pre, otherwise false
///
bool startsWith(const string &str, const string &pre)
{
    return str.compare(0, pre.size(), pre) == 0 ? true : false;
}

void extractLibs(const vector<string> &dylibs)
{
    for (const string &dylib : dylibs) {
        if (startsWith(dylib, HOMEBREW) || startsWith(dylib, MACPORTS)) {
            packageLibs.insert(dylib);
        } else if (startsWith(dylib, RPATH)) {
            rpathLibs.insert(dylib);
        } else if (startsWith(dylib, LOADER_PATH)) {
            loaderLibs.insert(dylib);
        } else if (startsWith(dylib, EXECUTABLE_PATH)) {
            executableLibs.insert(dylib);
        } else {
            otherLibs.insert(dylib);
        }
    }
}

void printLibs()
{
    cout << "----------------------------------------------------------------" << endl;
    cout << "*** libs from package manager ***" << endl;
    for (const string &dylib : packageLibs) {
        cout << dylib << endl;
    }

    cout << "----------------------------------------------------------------" << endl;
    cout << "*** @rpath ***" << endl;
    for (const string &dylib : rpathLibs) {
        cout << dylib << endl;
    }

    cout << "----------------------------------------------------------------" << endl;
    cout << "*** @loader_path ***" << endl;
    for (const string &dylib : loaderLibs) {
        cout << dylib << endl;
    }

    cout << "----------------------------------------------------------------" << endl;
    cout << "*** @executabe_path ***" << endl;
    for (const string &dylib : executableLibs) {
        cout << dylib << endl;
    }

    cout << "----------------------------------------------------------------" << endl;
    cout << "*** OTHER libs ***" << endl;
    for (const string &dylib : otherLibs) {
        cout << dylib << endl;
    }
    cout << "----------------------------------------------------------------" << endl;
}

void containsLibs(const string &dylib)
{
    string cmd{OTOOL};
    cmd.append(dylib);
    if (std::count(options.begin(), options.end(), "v")) {
        cout << "Cmd = " << cmd << endl;
    }

    FILE *fp;
    char path[PATH_MAX];

    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Cannot open pipe to otool\n";
        return;
    }

    vector<string> dylibs;

    while (fgets(path, PATH_MAX, fp) != NULL)
        if (path[0] == '\t') {
            firstWord(path + 1);
            // exclude libs fro further processing
            if ((strcmp(dylib.c_str(), path + 1) != 0) && // no duplicates
                !startsWith(path + 1, XCODE) &&           // no XCODE libs
                !startsWith(path + 1, SYSTEM_LIBS)) {     // no system libs
                dylibs.push_back(path + 1);
                if (!startsWith(path + 1, "/usr")) {
                    containsLibs(path + 1);
                }
            }
        }
    pclose(fp);
    extractLibs(dylibs);
}

int main(int argc, char *argv[])
{
    // Parsing command line arguments
    string libFileName{};
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && strlen(argv[i]) == 2) {
            options.push_back(argv[i] + 1);

        } else if (filesystem::exists(argv[i])) {
            libFileName = argv[i];
        } else {
            std::cerr << "usage: otoolrecursive [-vprelo]  <dynlib file>\n";
            return EXIT_FAILURE;
        }
    }

    containsLibs(libFileName);

    if (std::count(options.begin(), options.end(), "v")) {
        printLibs();
    }

    bool allLibs{true};
    if (std::count(options.begin(), options.end(), "p")) {
        for (const string &dylib : packageLibs) {
            cout << dylib << endl;
        }
        allLibs = false;
    }
    if (std::count(options.begin(), options.end(), "r")) {
        for (const string &dylib : rpathLibs) {
            cout << dylib << endl;
        }
        allLibs = false;
    }

    if (std::count(options.begin(), options.end(), "e")) {
        for (const string &dylib : executableLibs) {
            cout << dylib << endl;
        }
        allLibs = false;
    }

    if (std::count(options.begin(), options.end(), "l")) {
        for (const string &dylib : loaderLibs) {
            cout << dylib << endl;
        }
        allLibs = false;
    }

    if (std::count(options.begin(), options.end(), "o")) {
        for (const string &dylib : otherLibs) {
            cout << dylib << endl;
        }
        allLibs = false;
    }

    if (allLibs) {
        for (const string &dylib : packageLibs) {
            cout << dylib << endl;
        }
        for (const string &dylib : rpathLibs) {
            cout << dylib << endl;
        }
        for (const string &dylib : executableLibs) {
            cout << dylib << endl;
        }
        for (const string &dylib : loaderLibs) {
            cout << dylib << endl;
        }
        for (const string &dylib : otherLibs) {
            cout << dylib << endl;
        }
    }

    return EXIT_SUCCESS;
}
