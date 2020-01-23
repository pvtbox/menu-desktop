/**
*  
*  Pvtbox. Fast and secure file transfer & sync directly across your devices. 
*  Copyright © 2020  Pb Private Cloud Solutions Ltd. 
*  
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*  
**/
#include <iostream>
#include "WSListener.h"

// Websocket listener
WSListener::WSListener(std::function<void(std::string, std::string)> cb_status_changed)
    : ws_client(this), cb_status_changed(cb_status_changed), sync_dir("")
{
    std::cout << "call WSListener constructor" << std::endl;
    ws_client.start();
}


WSListener::~WSListener()
{
    std::cout << "call WSListener destructor" << std::endl;
    ws_client.stop();
}


void WSListener::on_message(
    const std::string & command,
    const std::string & path,
    std::vector<std::string> & paths,
    const std::string & status)
{
    if (command == "sync_dir")
    {
        std::string prev_sync_dir = sync_dir;
        sync_dir = path;

        if (prev_sync_dir == "" && sync_dir != "")
        {
            std::cout << "client has connected" << std::endl;
            for ( auto deferredPath: deferredPaths)
            {
                if (std::strstr(deferredPath.c_str(), (sync_dir + "/").c_str()) == deferredPath.c_str())
                {
                    // Send message to subscribe if path inside sync dir
                    std::cout << "send subscribe for: " << deferredPath << std::endl;
                    ws_client.send_message("status_subscribe", deferredPath);
                }
            }
            deferredPaths.clear();
        }
        else if (prev_sync_dir != "" && sync_dir == "")
        {
            std::cout << "client has disconnected" << std::endl;
            deferredPaths.clear();
            for (auto subscribed_path : subscribedPaths)
            {
                deferredPaths.insert(subscribed_path.first);
            }
            subscribedPaths.clear();
            for (auto p : deferredPaths)
            {
                cb_status_changed(p, "");
            }
        }
        else if (prev_sync_dir != "" && sync_dir != "")
        {
            std::cout << "sync dir has changed" << std::endl;
        }
    }
    else if (command == "status")
    {
        for (auto p : paths)
        {
            subscribedPaths[p] = status;
            cb_status_changed(p, status);
        }
    }
    else
    {
        //
    }
}


void WSListener::addPath(const std::string & path)
{
    std::string current_dir = path.substr(0, path.find_last_of("/"));

    if (sync_dir == "")
    {
        deferredPaths.insert(path);
    }
    else
    {
        // Send message to subscribe if path inside sync dir
        if (std::strstr(path.c_str(), (sync_dir + "/").c_str()) == path.c_str())
        {
            std::cout << "send subscribe for: " << path << std::endl;
            ws_client.send_message("status_subscribe", path);
        }
    }
}


std::string WSListener::getStatus(const std::string & path)
{
    auto subscribed_path = subscribedPaths.find(path);
    return subscribed_path != subscribedPaths.end() ? subscribed_path->second : "";
}
