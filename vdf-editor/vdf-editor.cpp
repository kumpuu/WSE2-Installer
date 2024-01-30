#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdint.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include "vdf.hpp"

namespace fs = std::filesystem;

const char* s_usage = "Usage:\n vdf_shortcut_editor.exe input_file -l (list vdf content)\n vdf_shortcut_editor.exe input_file -a appid AppName exe_path (Creates .bak backup)";

inline void rem_quotes(std::string& str)
{
    if (str.back() == '"' || str.back() == '\'') { str.pop_back(); }
    if (str.front() == '"' || str.front() == '\'') { str.erase(0, 1); }
}

void construct_shortcut(vdf_entry *entry, int appid, std::string appname, std::string exe_path)
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
        new vdf_entry {vdf_type::TYPE_INT, "appid", (char*)&appid, 4},
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
        new vdf_entry {vdf_type::TYPE_STR, "FlatpakAppID", &empty, 4},

        new vdf_entry {vdf_type::TYPE_NODE, "tags"}
    };
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << s_usage;
        return 0;
    }

    fs::path in_path(argv[1]);
    if (!fs::exists(in_path)) {
        std::cout << in_path << " does not exist, fool.";
        return 0;
    }

    std::ifstream f(argv[1], std::ifstream::in | std::ifstream::binary);
    vdf_doc doc;
    vdf_read_doc(f, doc);
    f.close();

    //std::ofstream of;
    //of.open("out.vdf");
    //vdf_dump_doc(of, doc);
    //of.close();
    //auto root = tyti::vdf::read<tyti::vdf::multikey_object>(f);

    if(std::string(argv[2]) == "-l"){
        if (argc != 3) {
            std::cout << s_usage;
            return 0;
        }

        vdf_print_doc(std::cout, doc);
    }
    else if (std::string(argv[2]) == "-a") {
        if (argc != 6) {
            std::cout << s_usage;
            return 0;
        }

        int32_t appid = atoi(argv[3]);
        std::string appname(argv[4]);
        std::string exe_path(argv[5]);

        vdf_entry entry;
        size_t num_sc = doc.childs[0]->childs.size();
        entry.name = std::to_string(num_sc);
        construct_shortcut(&entry, appid, appname, exe_path);

        fs::path out_path = in_path;
        out_path.concat(".bak");
        fs::copy_file(in_path, out_path, fs::copy_options::update_existing);

        doc.childs[0]->childs.push_back(&entry);

        std::ofstream of;
        of.open(in_path);
        vdf_dump_doc(of, doc);
        of.close();
    }

    return 0;
}
