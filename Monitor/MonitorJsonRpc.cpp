/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "Module.h"
#include "Monitor.h"
#include <interfaces/json/JsonData_Monitor.h>
#include <iostream>

using namespace std;
namespace WPEFramework {

namespace Plugin {

    using namespace JsonData::Monitor;

    // Registration
    //

    void Monitor::RegisterAll()
    {
	cout<<"START: Monitor::RegisterAll MonitorJsonRpc.cpp\n";
        Register<RestartlimitsParamsData,void>(_T("restartlimits"), &Monitor::endpoint_restartlimits, this);
        Register<ResetstatsParamsData,InfoInfo>(_T("resetstats"), &Monitor::endpoint_resetstats, this);
        Property<Core::JSON::ArrayType<InfoInfo>>(_T("status"), &Monitor::get_status, nullptr, this);
	cout<<"END: Monitor::RegisterAll MonitorJsonRpc.cpp\n";
    }

    void Monitor::UnregisterAll()
    {
	cout<<"START: Monitor::UnregisterAll MonitorJsonRpc.cpp\n";
        Unregister(_T("resetstats"));
        Unregister(_T("restartlimits"));
        Unregister(_T("status"));
	cout<<"END: Monitor::UnregisterAll MonitorJsonRpc.cpp\n";
    }

    // API implementation
    //

    // Method: restartlimits - Sets new restart limits for a plugin
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Monitor::endpoint_restartlimits(const RestartlimitsParamsData& params)
    {
	cout<<"START: Monitor::endpoint_restartlimits MonitorJsonRpc.cpp\n";
        const string& callsign = params.Callsign.Value();
        _monitor->Update(
            callsign,
            params.Restart.Window.Value(), params.Restart.Limit.Value());
	cout<<"END: Monitor::endpoint_restartlimits MonitorJsonRpc.cpp\n";
        return Core::ERROR_NONE;
    }

    // Method: resetstats - Resets memory and process statistics for a single plugin watched by the Monitor
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Monitor::endpoint_resetstats(const ResetstatsParamsData& params, InfoInfo& response)
    {
	cout<<"START: Monitor::endpoint_resetstats MonitorJsonRpc.cpp\n"; 
        const string& callsign = params.Callsign.Value();

        Core::JSON::ArrayType<InfoInfo> info;
        _monitor->Snapshot(callsign, &info);
        if (info.Length() == 1) {
            cout<<"START: info.Length() == 1 MonitorJsonRpc.cpp\n";
            _monitor->Reset(callsign);
            response = info[0];
        }
	cout<<"END: Monitor::endpoint_resetstats MonitorJsonRpc.cpp\n";
        return Core::ERROR_NONE;
    }

    // Property: status - The memory and process statistics either for a single plugin or all plugins watched by the Monitor
    // Return codes:
    //  - ERROR_NONE: Success
    uint32_t Monitor::get_status(const string& index, Core::JSON::ArrayType<InfoInfo>& response) const
    {
	cout<<"START: Monitor::get_status MonitorJsonRpc.cpp\n";
        const string& callsign = index;
        _monitor->Snapshot(callsign, &response);
	cout<<"END: Monitor::get_status MonitorJsonRpc.cpp\n";
        return Core::ERROR_NONE;
    }

    // Event: action - Signals action taken by the monitor
    void Monitor::event_action(const string& callsign, const string& action, const string& reason)
    {
	cout<<"START: Monitor::event_action MonitorJsonRpc.cpp\n";
        ActionParamsData params;
        params.Callsign = callsign;
        params.Action = action;
        params.Reason = reason;
	cout<<"END: Monitor::event_action MonitorJsonRpc.cpp\n";
        Notify(_T("action"), params);
    }
} // namespace Plugin
}

