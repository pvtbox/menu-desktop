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

#ifndef PATHCHECKER_H
#define PATHCHECKER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <condition_variable>
#include "../../common/ws_client.h"

#pragma once    

class PathChecker: public WSListenerInterface {
public:
    enum FileState {
        // Order synced with Overlay
        StateError = 0,
        StateSynced,
        StateSyncing,
        StatePaused,
        StateOnline,
        StateNone
    };
    PathChecker();
    ~PathChecker();
    std::wstring SyncDirectory() const;
    bool IsPathMonitored(const wchar_t* filePath, int* state);
    void on_message(const std::string& command, 
                    const std::string& path, 
                    std::vector<std::string>& paths, 
                    const std::string& status);

private:
    FileState _StrToFileState(const std::string& str);
    std::mutex _mutex;
    std::atomic<bool> _stop;
    WSClient client_;

    // Everything here is protected by the _mutex

    /** The list of paths we need to query. The main thread fill this, and the worker thread
    * send that to the socket. */
    std::unordered_map<std::wstring, std::time_t> _pending;
    std::unordered_map<std::wstring, int> _changed_paths;

    std::unordered_map<std::wstring, FileState> _cache;
    std::wstring _sync_directory;
    bool _connected;
    std::wstring _hidden_dir = L"";

    std::thread _thread;
    void workerThreadLoop();
    void send_messages_();
    void set_sync_dir_(const std::string& path);
    void refresh_();
    void set_status_(std::vector<std::string>& paths, const std::string& status);
    void clear_paths_(const std::string& path);
    void clear_cache_();
    bool is_path_hidden_(const std::wstring& path);
    bool is_path_online_file_(const std::wstring& path, int state);
    void update_pending_(const wchar_t* path);
    void clear_pending_();
    void query_update_();
};

#endif