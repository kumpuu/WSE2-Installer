#include <cstring>
#include <ios>
#include <vector>
#include <string>
#include <stdint.h>

/*Reading, writing, printing binary .vdf files*/

enum vdf_type
{
    TYPE_NODE = 0,
    TYPE_STR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_PTR,
    TYPE_WSTRING,
    TYPE_COLOR,
    TYPE_UINT64,
    TYPE_NUMTYPES,
};

const char* vdf_type_names[9] =
{
    "Node",
    "String",
    "Int",
    "Float",
    "Pointer",
    "WString",
    "Color",
    "Uint64",
    "Numtypes"
};

struct vdf_entry
{
    vdf_type ty = vdf_type::TYPE_NODE;
    std::string name = "";

    char* buf = NULL;
    size_t buf_len = 0;

    std::vector<vdf_entry*> childs;
};

struct vdf_doc {
    std::vector<vdf_entry*> childs;
};

void vdf_read_entry(std::ifstream &stream, vdf_entry &entry)
{
    int x = stream.tellg();
    char ty;
    ty = stream.get();

    entry.ty = (vdf_type)ty;

    if(ty == vdf_type::TYPE_NUMTYPES){
        return;
    }

    std::getline(stream, entry.name, '\0');

    //std::cout << char(ty + '0') << " " << entry.name << "\n";

    if(ty == vdf_type::TYPE_NODE){
        while(true) {
            x = stream.tellg();
            vdf_entry *child = new vdf_entry();
            vdf_read_entry(stream, *child);
            int y = stream.tellg();

            if(child->ty == vdf_type::TYPE_NUMTYPES){
                delete child;
                break;
            }

            entry.childs.push_back(child);
        }
    }
    else if(ty == vdf_type::TYPE_STR){
        std::string s;
        std::getline(stream, s, '\0');
        entry.buf = (char*)malloc(s.length() + 1);
        std::memcpy(entry.buf, s.c_str(), s.length() + 1);
        entry.buf_len = s.length() + 1;
    }
    else if(ty == vdf_type::TYPE_INT || ty == vdf_type::TYPE_FLOAT || ty == vdf_type::TYPE_PTR || ty == vdf_type::TYPE_COLOR){
        entry.buf = (char*)malloc(4);
        entry.buf_len = 4;
        stream.read(entry.buf, 4);
    }
    else if(ty == vdf_type::TYPE_WSTRING){
        signed short len;
        stream.read((char*)&len, 2);
        entry.buf = (char*)malloc(len*2 + 2);
        entry.buf[len*2]     = '\0';
        entry.buf[len*2 + 1] = '\0';
        entry.buf_len = len*2 + 2;
    }
    else if(ty == vdf_type::TYPE_UINT64){
        entry.buf = (char*)malloc(8);
        stream.read(entry.buf, 8);
        entry.buf_len = 8;
    }
}

vdf_doc vdf_read_doc(std::ifstream &stream, vdf_doc &doc)
{
    int x = stream.tellg();
    while(stream.peek() != EOF) {
        vdf_entry *entry = new vdf_entry();
        vdf_read_entry(stream, *entry);
        doc.childs.push_back(entry);
        x = stream.tellg();
    }
    return doc;
}

void vdf_dump_entry(std::ofstream &stream, vdf_entry *entry)
{
    vdf_type ty = entry->ty;
    stream.write((char*)&ty, 1);

    if(ty == vdf_type::TYPE_NUMTYPES){return;}

    stream.write(entry->name.c_str(), entry->name.length() + 1);

    if(ty == vdf_type::TYPE_NODE){
        for(auto child : entry->childs){
            vdf_dump_entry(stream, child);
        }
        stream.put(char(8));
    }
    else {
        stream.write(entry->buf, entry->buf_len);
    }
}

void vdf_dump_doc(std::ofstream &stream, vdf_doc &doc)
{
    for(auto child : doc.childs){
        vdf_dump_entry(stream, child);
    }
}

void vdf_print_entry(std::ostream &stream, vdf_entry* entry, std::string indent)
{
    vdf_type ty = entry->ty;

    stream << indent << vdf_type_names[ty] << " '" << entry->name << "' = ";

    if (ty == vdf_type::TYPE_NODE) {
        stream << "{\n";
        for (auto child : entry->childs) {
            vdf_print_entry(stream, child, indent + "  ");
        }
        stream << indent << "}";
    }
    else if(ty == vdf_type::TYPE_STR){
        stream << "'" << (char*)entry->buf << "'";
    }
    else if (ty == vdf_type::TYPE_INT) {
        stream << *(int32_t*)entry->buf;
    }
    else if (ty == vdf_type::TYPE_FLOAT) {
        stream << *(float*)entry->buf;
    }
    else if (ty == vdf_type::TYPE_PTR) {
        stream << *(int32_t*)entry->buf;
    }
    else if (ty == vdf_type::TYPE_WSTRING) {
        std::wcout << *(wchar_t*)entry->buf;
    }
    else if (ty == vdf_type::TYPE_COLOR) {
        stream << *(int32_t*)entry->buf;
    }
    else if (ty == vdf_type::TYPE_UINT64) {
        stream << *(int64_t*)entry->buf;;
    }

    stream << std::endl;
}

void vdf_print_doc(std::ostream& stream, vdf_doc& doc)
{
    stream << "document:\n";
    for (auto child : doc.childs) {
        vdf_print_entry(stream, child, "  ");
    }
}