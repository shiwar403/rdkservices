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
 
#include "Monitor.h"
#include <iostream>

using namespace std;

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(Monitor, 1, 0);

    static Core::ProxyPoolType<Web::JSONBodyType<Core::JSON::ArrayType<Monitor::Data>>> jsonBodyDataFactory(2);
    static Core::ProxyPoolType<Web::JSONBodyType<Monitor::Data>> jsonBodyParamFactory(2);
    static Core::ProxyPoolType<Web::JSONBodyType<Monitor::Data::MetaData>> jsonMemoryBodyDataFactory(2);

    /* virtual */ const string Monitor::Initialize(PluginHost::IShell* service)
    {
	cout<<"START: Monitor::Initialize Monitor.cpp\n";
        _config.FromString(service->ConfigLine());

        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        Core::JSON::ArrayType<Config::Entry>::Iterator index(_config.Observables.Elements());

        // Create a list of plugins to monitor..
        _monitor->Open(service, index);

        // During the registartion, all Plugins, currently active are reported to the sink.
        service->Register(_monitor);

        // On succes return a name as a Callsign to be used in the URL, after the "service"prefix
	cout<<"END: Monitor::Initialize Monitor.cpp\n";
        return (_T(""));
    }

    /* virtual */ void Monitor::Deinitialize(PluginHost::IShell* service)
    {
	cout<<"START: Monitor::Deinitialize Monitor.cpp\n"
        _monitor->Close();

        service->Unregister(_monitor);
	cout<<"END: Monitor::Deinitialize Monitor.cpp\n";
    }

    /* virtual */ string Monitor::Information() const
    {
        // No additional info to report.
        return (nullptr);
    }

    /* virtual */ void Monitor::Inbound(Web::Request& request)
    {
        if ((request.Verb == Web::Request::HTTP_PUT) || (request.Verb == Web::Request::HTTP_POST))
            request.Body(jsonBodyParamFactory.Element());
    }

    // <GET> ../				Get all Memory Measurments
    // <GET> ../<Callsign>		Get the Memory Measurements for Callsign
    // <PUT> ../<Callsign>		Reset the Memory measurements for Callsign
    /* virtual */ Core::ProxyType<Web::Response> Monitor::Process(const Web::Request& request)
    {
	cout<<"START: Monitor::Process Monitor.cpp\n";
        ASSERT(_skipURL <= request.Path.length());

        Core::ProxyType<Web::Response> result(PluginHost::IFactories::Instance().Response());
        Core::TextSegmentIterator index(Core::TextFragment(request.Path, _skipURL, static_cast<uint32_t>(request.Path.length() - _skipURL)), false, '/');

        // If there is an entry, the first one will alwys be a '/', skip this one..
        index.Next();

        // By default, we assume everything works..
        result->ErrorCode = Web::STATUS_OK;
        result->Message = "OK";

        if (request.Verb == Web::Request::HTTP_GET) {
	    cout<<"Monitor::Process request.Verb == Web::Request::HTTP_GET Monitor.cpp\n";
            // Let's list them all....
            if (index.Next() == false) {
		cout<<"Monitor::Process index.Next() == false Monitor.cpp\n";
                if (_monitor->Length() > 0) {
		    cout<<"Monitor::Process _monitor->Length() > 0 Monitor.cpp\n";
                    Core::ProxyType<Web::JSONBodyType<Core::JSON::ArrayType<Monitor::Data>>> response(jsonBodyDataFactory.Element());

                    _monitor->Snapshot(*response);

                    result->Body(Core::proxy_cast<Web::IBody>(response));
                }
            } else {
                MetaData memoryInfo;

                // Seems we only want 1 name
                if (_monitor->Snapshot(index.Current().Text(), memoryInfo) == true) {
		    cout<<"Monitor::Process _monitor->Snapshot(index.Current().Text(), memoryInfo) == true Monitor.cpp\n";
                    Core::ProxyType<Web::JSONBodyType<Monitor::Data::MetaData>> response(jsonMemoryBodyDataFactory.Element());

                    *response = memoryInfo;

                    result->Body(Core::proxy_cast<Web::IBody>(response));
                }
            }

            result->ContentType = Web::MIME_JSON;
        } else if ((request.Verb == Web::Request::HTTP_PUT) && (index.Next() == true)) {
	    cout<<"Monitor::Process request.Verb == Web::Request::HTTP_PUT) && (index.Next() == true Monitor.cpp\n";
            MetaData memoryInfo;

            // Seems we only want 1 name
            if (_monitor->Reset(index.Current().Text(), memoryInfo) == true) {
		cout<<"Monitor::Process _monitor->Reset(index.Current().Text(), memoryInfo) == true Monitor.cpp\n";
                Core::ProxyType<Web::JSONBodyType<Monitor::Data::MetaData>> response(jsonMemoryBodyDataFactory.Element());

                *response = memoryInfo;

                result->Body(Core::proxy_cast<Web::IBody>(response));
            }

            result->ContentType = Web::MIME_JSON;
        } else if ((request.Verb == Web::Request::HTTP_POST) && (request.HasBody())) {
	    cout<<"Monitor::Process request.Verb == Web::Request::HTTP_POST) && (request.HasBody() Monitor.cpp\n";
            Core::ProxyType<const Monitor::Data> body(request.Body<const Monitor::Data>());
            string observable = body->Observable.Value();

            uint16_t restartWindow = 0;
            uint8_t restartLimit = 0;

            if (body->Restart.IsSet()) {
		cout<<"Monitor::Process body->Restart.IsSet() Monitor.cpp\n";
                restartWindow = body->Restart.Window;
                restartLimit = body->Restart.Limit;
            }
            TRACE(Trace::Information, (_T("Sets Restart Limits:[LIMIT:%d, WINDOW:%d]"), restartLimit, restartWindow));
            _monitor->Update(observable, restartWindow, restartLimit);
        } else {
            result->ErrorCode = Web::STATUS_BAD_REQUEST;
            result->Message = _T(" could not handle your request.");
        }
	cout<<"END: Monitor::Process Monitor.cpp\n";
        return (result);
    }
}
}
