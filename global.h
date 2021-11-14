#ifndef GLOBAL_H
#define GLOBAL_H
#include "json.hpp"
using nljson=nlohmann::json;
#define _getSetting(name) (global::profile[name].is_string()?QString::fromStdString(global::profile[name].get<std::string>()):(_getSettingPreset(name)))
#define _getSettingPreset(name) (global::profilePreset[name].is_string()?QString::fromStdString(global::profilePreset[name].get<std::string>()):QString(""))
#define getSetting(name) (_getSetting(name))

#define fillSetting(name,property) global::profile[name]=QString(property).toStdString()

class global
{
public:
    static nljson profile;
    static nljson profilePreset;
};

#endif // GLOBAL_H
