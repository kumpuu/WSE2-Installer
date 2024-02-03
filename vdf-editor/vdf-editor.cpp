#include <windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include "vdf.hpp"

namespace fs = std::filesystem;

const char* s_usage = "Usage:\n vdf_shortcut_editor.exe input_file -l (list vdf content)\n vdf_shortcut_editor.exe input_file -a appid AppName exe_path (Creates .bak backup, then inserts shortcut)\n";

inline void rem_quotes(std::string& str)
{
    if (str.back() == '"' || str.back() == '\'') { str.pop_back(); }
    if (str.front() == '"' || str.front() == '\'') { str.erase(0, 1); }
}

void construct_shortcut(vdf_entry *entry, int* appid, std::string appname, std::string exe_path)
{
    rem_quotes(appname);

    size_t c_appname_size = appname.length() + 1;
    static char* c_appname = new char[c_appname_size];
    std::memcpy(c_appname, appname.c_str(), c_appname_size);

    rem_quotes(exe_path);

    fs::path p(exe_path);
    p.remove_filename();
    std::string dir = p.string();

    size_t c_dir_size = dir.length() + 1;
    static char* c_dir = new char[c_dir_size];
    std::memcpy(c_dir, dir.c_str(), c_dir_size);

    exe_path = '"' + exe_path + '"';
    size_t c_exe_path_size = exe_path.length() + 1;
    static char* c_exe_path = new char[c_exe_path_size];
    std::memcpy(c_exe_path, exe_path.c_str(), c_exe_path_size);

    static char empty = '\0';
    static int zero = 0;
    static int one = 1;

    entry->childs = std::vector<vdf_entry*> {
        new vdf_entry {vdf_type::TYPE_INT, "appid", (char*)appid, 4},
        new vdf_entry {vdf_type::TYPE_STR, "AppName", c_appname, c_appname_size},
        new vdf_entry {vdf_type::TYPE_STR, "Exe", c_exe_path, c_exe_path_size},
        new vdf_entry {vdf_type::TYPE_STR, "StartDir", c_dir, c_dir_size},

        new vdf_entry {vdf_type::TYPE_STR, "icon", &empty, 1},
        new vdf_entry {vdf_type::TYPE_STR, "ShortcutPath", &empty, 1},
        new vdf_entry {vdf_type::TYPE_STR, "LaunchOptions", &empty, 1},

        new vdf_entry {vdf_type::TYPE_INT, "IsHidden", (char*)&zero, 4},
        new vdf_entry {vdf_type::TYPE_INT, "AllowDesktopConfig", (char*)&one, 4},
        new vdf_entry {vdf_type::TYPE_INT, "AllowOverlay", (char*)&one, 4},
        new vdf_entry {vdf_type::TYPE_INT, "OpenVR", (char*)&zero, 4},

        new vdf_entry {vdf_type::TYPE_INT, "Devkit", (char*)&zero, 4},
        new vdf_entry {vdf_type::TYPE_STR, "DevkitGameID", &empty, 1},
        new vdf_entry {vdf_type::TYPE_INT, "DevkitOverrideAppID", (char*)&zero, 4},

        new vdf_entry {vdf_type::TYPE_INT, "LastPlayTime", (char*)&zero, 4},
        new vdf_entry {vdf_type::TYPE_STR, "FlatpakAppID", &empty, 1},

        new vdf_entry {vdf_type::TYPE_NODE, "tags"}
    };
}

bool compareFiles(const std::wstring& p1, const std::wstring& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false; //file problem
    }

    if (f1.tellg() != f2.tellg()) {
        return false; //size mismatch
    }

    //seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
        std::istreambuf_iterator<char>(),
        std::istreambuf_iterator<char>(f2.rdbuf()));
}

void convert_args_u8(int argc, wchar_t* argv[], std::vector<std::string>& args)
{
    for (size_t i = 0; i < argc; i++)
    {

        int size = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)argv[i], -1, NULL, 0, NULL, NULL);

        std::string s(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)argv[i], -1, &s[0], size, NULL, NULL);
        s.resize(size - 1);

        args.push_back(s);
    }
}

int wmain(int argc, wchar_t* argv[])
{
    //_setmode(_fileno(stdout), _O_U8TEXT);
    //we get args as wchar_t* (ansi?) but we need utf8... Thats the theory
    std::vector<std::string> args;
    convert_args_u8(argc, argv, args);

    if (argc < 3) {
        std::cout << s_usage;
        return 0;
    }

    fs::path in_path(argv[1]);
    if (!fs::exists(in_path)) {
        std::cout << args[1] << " does not exist, fool.";
        return 0;
    }

    std::ifstream f(argv[1], std::ifstream::in | std::ifstream::binary);
    vdf_doc doc;
    vdf_read_doc(f, doc);
    f.close();

    if(args[2] == "-l") {
        if (argc != 3) {
            std::cout << s_usage;
            return 0;
        }

        vdf_print_doc(std::cout, doc);
    }
    else if (args[2] == "-a") {
        if (argc != 6) {
            std::cout << s_usage;
            return 0;
        }

        //sanity check to see if reading->dumping produces the same binary
        fs::path bakbak_path = in_path;
        bakbak_path.concat(".bakbak");

        std::ofstream of;
        of.open(bakbak_path, std::ofstream::trunc);
        vdf_dump_doc(of, doc);
        of.close();

        if ( !compareFiles(in_path.wstring(), bakbak_path.wstring()) ) {
            std::cout << "Something went wrong reading the .vdf! Aborting.";
            return 1;
        }

        //insert new shortcut
        int32_t appid = atoi(args[3].c_str());
        std::string& appname  = args[4];
        std::string& exe_path = args[5];

        vdf_entry entry;
        size_t num_sc = doc.childs[0]->childs.size();
        entry.name = std::to_string(num_sc);
        construct_shortcut(&entry, &appid, appname, exe_path);

        std::vector<std::string> ignores{"LastPlayTime"};
        for (vdf_entry* child : doc.childs[0]->childs)
        {
            if (vdf_cmp_nodes(child, &entry, ignores))
            { 
                std::cout << "Shortcut already exists.";
                return 0;
            }
        }

        fs::path bak_path = in_path;
        bak_path.concat(".bak");
        fs::rename(bakbak_path, bak_path);
        
        doc.childs[0]->childs.push_back(&entry);

        of.open(in_path);
        vdf_dump_doc(of, doc);
        of.close();
        std::cout << "Successfully added shortcut.";
    }

    return 0;
}
