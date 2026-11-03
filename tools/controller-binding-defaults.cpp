// Compile as a standalone C++20 utility from the repository root.
// --write exports the current built-in mappings; otherwise verifies shipped INIs.
#include "../source/includes/vrcontrollerbindings.hpp"
#include "../source/includes/vrbindingconfig.hpp"
#include <array>
#include <iostream>
#include <regex>
#include <stdexcept>

struct Action {std::string group,name,label,type,component;int hand;};
int main(int argc,char** argv) try {
    using namespace VRControllerBindings;
    const bool write=argc==2 && std::string_view(argv[1])=="--write";
    std::ifstream source("source/openxrvr.ixx");
    if(!source)throw std::runtime_error("Run from repository root");
    const std::regex hands(R"rx(!create\(inputActions,"([^"]+)","([^"]+)",(\w+),\w+,(-?\d+),"([^"]+)"\))rx");
    const std::regex game(R"rx(!game\(Action::\w+,(\w+),"([^"]+)","([^"]+)",(\w+),(-?\d+),(nullptr|"[^"]*")\))rx");
    const std::map<std::string,std::string> groups{{"inputActions","hands"},{"gameplayActions","on_foot"},
        {"drivingActions","driving"},{"menuActions","menus"},{"systemActions","system"},{"phoneActions","phone"}};
    std::vector<Action> actions;
    for(std::string line;std::getline(source,line);) {
        std::smatch match;
        if(std::regex_search(line,match,hands))
            actions.push_back({"hands",match[1],match[2],match[3],match[5],std::stoi(match[4])});
        else if(std::regex_search(line,match,game)) {
            std::string component=match[6];
            component=component=="nullptr" ? "":component.substr(1,component.size()-2);
            actions.push_back({groups.at(match[1]),match[2],match[3],match[4],component,std::stoi(match[5])});
        } else if(line.find("!create(inputActions,")!=line.npos || line.find("!game(Action::")!=line.npos)
            throw std::runtime_error("Action catalog format changed; update this exporter: "+line);
    }
    if(actions.empty())throw std::runtime_error("No actions found");
    size_t checked=0;
    for(const auto profile:{Profile::Vive,Profile::Touch,Profile::Index,Profile::Motion}) {
        const auto path=std::filesystem::path("data/plugins/VRBindings")/FileName(profile);
        auto config=VRBindingConfig::File::Load(path);
        std::ofstream out;
        if(write) {
            std::filesystem::create_directories(path.parent_path());out.open(path);
            out<<"; GTA4VR default controller bindings: "<<Path(profile)<<"\n"
                "; Restart GTA IV to apply edits. Missing keys use built-in defaults.\n"
                "; Use none (or an empty value) to unbind. Multiple paths: comma-separated.\n"
                "; Paths must be supported by this controller profile and the action type.\n"
                "; Guide: GTA4VR-notices/Controller-bindings.md (docs/controller-bindings-ini.md in source).\n";
        }
        for(const auto& group:{"hands","on_foot","driving","menus","system","phone"}) {
            if(write)out<<"\n["<<group<<"]\n";
            for(const auto& action:actions)if(action.group==group) {
                const unsigned first=action.hand<0 ? 0:action.hand,last=action.hand<0 ? 1:action.hand;
                for(unsigned hand=first;hand<=last;++hand) {
                    const auto key=action.name+(action.hand<0 ? (hand ? ".right":".left"):"");
                    const auto component=Component(profile,action.name,hand,action.component,action.group);
                    std::vector<std::string> expected;
                    if(!component.empty())expected.push_back(std::string(Hand(profile,action.name,hand) ?
                        "/user/hand/right":"/user/hand/left")+std::string(component));
                    if(write)out<<"; "<<action.label<<" ("<<action.type<<")\n"<<key<<" = "<<(expected.empty() ? "none":expected[0])<<"\n";
                    else {
                        if(!config.values.contains({group,key}))throw std::runtime_error(path.string()+": missing "+group+"."+key);
                        if(config.Resolve(group,key,{},action.hand<0 ? int(hand):-1)!=expected)
                            throw std::runtime_error(path.string()+": default differs for "+group+"."+key);
                        ++checked;
                    }
                }
            }
        }
        if(!write) {config.CheckUnused();if(!config.errors.empty())throw std::runtime_error(path.string()+": "+config.errors[0]);}
    }
    std::cout<<(write ? "Exported":"Verified")<<" four profile INIs; "<<actions.size()<<" actions, "<<checked<<" unchanged default entries\n";
    return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
