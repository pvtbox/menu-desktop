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
#include "stdafx.h"

#include "RemotePathChecker.h"
#include "StringUtil.h"

#include <shlobj.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <cassert>
#include <valarray>
#include <fstream>
#include <stack>

using namespace std;

std::string ws2utf8(const std::wstring& wstr) {
    if (wstr.empty())
        return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), int(wstr.size()), 0, 0, NULL, NULL);
    std::valarray<char> buf(size + 1);
    size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), int(wstr.size()), &buf[0], size, NULL, NULL);
    if (!size)
        throw std::runtime_error("Failed to convert to UTF8");

    buf[size] = '\0';
    return std::string(&buf[0]);
}

std::wstring utf82ws(const std::string& str) {
    if (str.empty())
        return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[size];
    // std::valarray<char> buf(size + 1);
    size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, size);
    if (!size)
        throw std::runtime_error("Failed to convert from UTF8");

    return wstr;
}

bool is_dir(const std::wstring& path) {
    DWORD dwAttrib = GetFileAttributesW(path.data());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// This code is run in a thread
void PathChecker::workerThreadLoop() {
    bool sync_empty;
    int tick = 0;
    std::unique_lock<std::mutex> lock(_mutex);
    lock.unlock();
    while (!_stop) {
        Sleep(50);

        lock.lock();
        _connected = client_.is_connected();
        sync_empty = _sync_directory.empty();
        lock.unlock();

        if (!_connected || sync_empty) {
            lock.lock();
            clear_pending_();
            lock.unlock();
            continue;
        }

        ++tick %= 100;
        if (!tick) {
            // query update once in 5 seconds
            query_update_();
        }

        send_messages_();
    }
}

void PathChecker::send_messages_() {
    std::unordered_map<std::wstring, std::time_t> pending;
    std::unique_lock<std::mutex> lock(_mutex);
    swap(pending, _pending);
    lock.unlock();

    for (auto p_it = pending.begin(); p_it != pending.end() && !_stop; ++p_it) {
        auto filePath = p_it->first;
        
        if (StringUtil::isContainedIn(filePath, _sync_directory)) {
            client_.send_message("status_subscribe", ws2utf8(filePath));
        }
    }
    pending.clear();
}

PathChecker::PathChecker()
    : _stop(false)
    , _connected(false)
    , _thread([this] { this->workerThreadLoop(); })
    , client_(this)
    , _sync_directory()
{
    client_.start();
}

PathChecker::~PathChecker()
{
    _stop = true;
    client_.stop();
    _thread.join();
 }

std::wstring PathChecker::SyncDirectory() const
{
    return _sync_directory;
}

bool PathChecker::IsPathMonitored(const wchar_t* filePath, int* state)
{
    // log_f << "checker log " << "IsPathMonitored " << ws2utf8(filePath) << " cache size " << _cache.size() << std::endl;
    assert(state); assert(filePath);

    std::unique_lock<std::mutex> lock(_mutex);
     if (!_connected || _sync_directory.empty()) {
        update_pending_(filePath);
        return false;
    }

     auto path = std::wstring(filePath);
     if (is_path_hidden_(path))
        return false;

    auto it = _cache.find(path);
    if (it != _cache.end()) {
        // The path is in our cache, and we'll get updates pushed if the status changes.
        *state = it->second;
        // log_f << "checker log " << "IsPathMonitored " << ws2utf8(filePath) << " state " << *state << std::endl;
        return true;
    }

    update_pending_(filePath);

    lock.unlock();
    return false;
}

void PathChecker::update_pending_(const wchar_t* path) {
    std::time_t now = time(NULL);
    auto p_it = _pending.find(path);
    if (p_it == _pending.end()) {
        p_it = _pending.insert(make_pair(path, now)).first;
    }
    else {
        p_it->second = now;
    }
}

void PathChecker::clear_pending_() {
    std::time_t now = time(NULL);
    for (auto p_it = _pending.begin(); p_it != _pending.end(); ) {
        // clear all older 1 minute from now
        if (now - p_it->second > 60) {
            p_it = _pending.erase(p_it);
            
        }
        else
            ++p_it;
    }
}

void PathChecker::query_update_() {
    for (auto p_it = _changed_paths.begin(); p_it != _changed_paths.end(); ) {
        // query update path
        std::wstring path = p_it->first;
        // log_f << "checker log " << "update queried for path " << ws2utf8(path) << std::endl;
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, path.data(), NULL);
        
        {   std::unique_lock<std::mutex> lock(_mutex);
        p_it->second--;
        if (!(p_it->second)) {
            p_it = _changed_paths.erase(p_it);
        }
        else
            ++p_it;
        }
    }
}

PathChecker::FileState PathChecker::_StrToFileState(const std::string& str)
{
    if (str == "syncing") {
        return StateSyncing;
    }
    else if (str == "synced") {
        return StateSynced;
    }
    else if (str == "paused") {
        return StatePaused;
    }
    else if (str == "error") {
        return StateError;
    }

    return StateNone;
}

void PathChecker::on_message(const std::string& command, const std::string& path, std::vector<std::string>& paths, const std::string& status) {
    if (command == "sync_dir")
        set_sync_dir_(path);
    else if (command == "status")
        set_status_(paths, status);
    else if (command == "clear")
        clear_paths_(path);
    else if (command == "refresh")
        refresh_();
}

void PathChecker::set_sync_dir_(const std::string& path) {
    std::wstring new_sync_dir = utf82ws(path); //StringUtil::toUtf16(path.c_str());
        
    if (_sync_directory != new_sync_dir)
        clear_cache_();

    std::unique_lock<std::mutex> lock(_mutex);
    _sync_directory = new_sync_dir;
    _hidden_dir = _sync_directory + L"\\.pvtbox";
    lock.unlock();
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, _sync_directory.data(), NULL);
}

void PathChecker::refresh_() {
    if (!_sync_directory.empty()) {
        DWORD_PTR dwResult;

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
            0, SMTO_ABORTIFHUNG, 5000, &dwResult);
    }
}

bool PathChecker::is_path_hidden_(const std::wstring& path) {
    // log_f << "checker log " << "path " << ws2utf8(path) << " hidden dir " << ws2utf8(_hidden_dir) << std::endl;
    if (!path.compare(_hidden_dir) || StringUtil::isContainedIn(path, _hidden_dir))
        return true;

    if (StringUtil::ends_with(path, L"\\desktop.ini"))
        return true;

    if (StringUtil::ends_with(path, L".download"))
        return true;

    return false;
}

void PathChecker::set_status_(std::vector<std::string>& paths, const std::string& status) {
    wstring responsePath;

    auto state = _StrToFileState(status);
    // log_f << "checker log " << "status " << status << std::endl;
    for (auto itr: paths) {
        // log_f << "checker log " << "path " << itr << std::endl;
        responsePath = utf82ws(itr); //StringUtil::toUtf16(itr.c_str());
        
        bool updateView = false;
        {   std::unique_lock<std::mutex> lock(_mutex);

        auto it = _cache.find(responsePath);
        if (it == _cache.end()) {
            it = _cache.insert(make_pair(responsePath, StateNone)).first;
        }

        updateView = it->second != state;
        // log_f << "checker log " << "update view " << it->second << " " << state << std::endl;
        it->second = state;
        }
        if (updateView) {
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, responsePath.data(), NULL);

            {   std::unique_lock<std::mutex> lock(_mutex);
            // save path to query update later
            auto p_it = _changed_paths.find(responsePath);
            if (p_it == _changed_paths.end()) {
                p_it = _changed_paths.insert(make_pair(responsePath, 0)).first;
            }
            // query update 3 times more
            p_it->second = 3;
            }
        }
    }
    paths.clear();
}

void PathChecker::clear_paths_(const std::string& path) {
    // log_f << "checker log " << "clear paths starting from " << path << std::endl;
    if (path.empty()) {
        set_sync_dir_(path);
        clear_cache_();
        return;
    }

    wstring responsePath = utf82ws(path); //StringUtil::toUtf16(path.c_str());
    vector<wstring> removedPaths;
    {   std::unique_lock<std::mutex> lock(_mutex);
    // Remove any item from the cache
    for (auto it = _cache.begin(); it != _cache.end(); ) {
        if (StringUtil::isContainedIn(it->first, responsePath)) {
            removedPaths.emplace_back(move(it->first));
            it = _cache.erase(it);
        }
        else {
            ++it;
        }
    }
    }
    for (auto& p : removedPaths){
        int pos = p.rfind(L'\\');
        if(pos == wstring::npos)
            continue;

        wstring path_dir = p.substr(0, pos - 1);
        // log_f << "checker log " << "update dir" << ws2utf8(path_dir) << std::endl;
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSHNOWAIT, path_dir.data(), NULL);
    }
}

void PathChecker::clear_cache_() {
    // log_f << "checker log " << "clear cache" << std::endl;
    std::unique_lock<std::mutex> lock(_mutex);

    // Swap to make a copy of the cache under the mutex and clear the one stored.
    std::unordered_map<std::wstring, FileState> cache;
    swap(cache, _cache);
    lock.unlock();
    // Let explorer know about each invalidated cache entry that needs to get its icon removed.
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        LONG event_id = is_dir(it->first) ? SHCNE_UPDATEDIR : SHCNE_UPDATEITEM;
        SHChangeNotify(event_id, SHCNF_PATH | SHCNF_FLUSHNOWAIT, it->first.data(), NULL);
    }
}

