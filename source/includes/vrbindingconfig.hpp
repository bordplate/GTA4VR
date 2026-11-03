#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace VRBindingConfig {
inline std::string Trim(std::string_view text) {
    const auto first=text.find_first_not_of(" \t\r\n");
    if(first==text.npos)return {};
    return std::string(text.substr(first,text.find_last_not_of(" \t\r\n")-first+1));
}
inline std::string Lower(std::string text) {
    for(auto& c:text)if(c>='A' && c<='Z')c+= 'a'-'A';
    return text;
}
inline bool InputPath(std::string_view path,int hand=-1) {
    const std::string_view left="/user/hand/left/input/",right="/user/hand/right/input/";
    const auto prefix=path.starts_with(left) ? left:path.starts_with(right) ? right:std::string_view{};
    if(prefix.empty() || path.size()==prefix.size() || path.back()=='/' || path.find("//")!=path.npos)return false;
    if((hand==0 && prefix!=left) || (hand==1 && prefix!=right))return false;
    return std::all_of(path.begin(),path.end(),[](char c){
        return (c>='a' && c<='z') || (c>='0' && c<='9') || c=='/' || c=='_' || c=='-' || c=='.';
    });
}
inline std::optional<std::vector<std::string>> Paths(std::string_view text,int hand=-1) {
    std::vector<std::string> paths;
    const auto value=Trim(text);
    if(value.empty() || Lower(value)=="none")return paths;
    size_t at=0;
    do {
        const auto end=value.find(',',at);
        const auto path=Trim(std::string_view(value).substr(at,end==value.npos ? value.npos:end-at));
        if(!InputPath(path,hand))return std::nullopt;
        if(std::find(paths.begin(),paths.end(),path)==paths.end())paths.push_back(path);
        if(end==value.npos)break;
        at=end+1;
    }while(at<=value.size());
    return paths;
}
struct File {
    using Key=std::pair<std::string,std::string>;
    std::map<Key,std::string> values;
    std::set<Key> used;
    std::vector<std::string> errors;
    static File Parse(std::istream& input) {
        File file;std::string line,section;unsigned number=0;
        while(std::getline(input,line)) {
            ++number;
            if(number==1 && line.starts_with("\xEF\xBB\xBF"))line.erase(0,3);
            const auto comment=std::min({line.find(';'),line.find('#'),line.find("//")});
            line=Trim(std::string_view(line).substr(0,comment));
            if(line.empty())continue;
            if(line.front()=='[') {
                if(line.back()!=']' || line.size()<3) {
                    file.errors.push_back("Invalid section at line "+std::to_string(number));section.clear();continue;
                }
                section=Lower(Trim(std::string_view(line).substr(1,line.size()-2)));continue;
            }
            const auto equal=line.find('=');
            const auto key=Lower(Trim(std::string_view(line).substr(0,equal)));
            if(section.empty() || equal==line.npos || key.empty()) {
                file.errors.push_back("Invalid binding at line "+std::to_string(number));continue;
            }
            if(!file.values.emplace(Key{section,key},Trim(std::string_view(line).substr(equal+1))).second)
                file.errors.push_back("Duplicate binding ["+section+"] "+key+"; first entry retained");
        }
        return file;
    }
    static File Load(const std::filesystem::path& path) {
        std::ifstream input(path);
        if(input)return Parse(input);
        File file;std::error_code ec;
        if(std::filesystem::exists(path,ec) || ec)file.errors.push_back("Cannot read bindings file");
        return file;
    }
    std::vector<std::string> Resolve(std::string_view section,std::string_view key,
        const std::vector<std::string>& fallback,int hand=-1) {
        const Key id{Lower(std::string(section)),Lower(std::string(key))};
        used.insert(id);
        const auto found=values.find(id);
        if(found==values.end())return fallback;
        if(const auto parsed=Paths(found->second,hand))return *parsed;
        errors.push_back("Invalid input path ["+id.first+"] "+id.second+"; default retained");
        return fallback;
    }
    void CheckUnused() {
        for(const auto& [key,value]:values)if(!used.contains(key))
            errors.push_back("Unknown binding ["+key.first+"] "+key.second);
    }
};
}
