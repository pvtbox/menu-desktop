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
#include <unordered_set>
#include <unordered_map>

#include "../common/ws_client.h"


// Websocket listener
class WSListener : private WSListenerInterface
{
public:
    WSListener(std::function<void(std::string, std::string)> cb_status_changed);
    ~WSListener();
    void on_message(
            const std::string & command,
            const std::string & path,
            std::vector<std::string> & paths,
            const std::string & status);
    void addPath(const std::string & path);
    std::string getStatus(const std::string & path);

private:
    std::function<void(std::string, std::string)> cb_status_changed;
    WSClient ws_client;
    std::string sync_dir;
    std::unordered_set <std::string> deferredPaths;
    std::unordered_map <std::string, std::string> subscribedPaths;
};
