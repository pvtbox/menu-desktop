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
#pragma warning(disable : 4996)
#ifdef _WIN32
  #include "stdafx.h"
  #include <shlobj.h>
#endif
#include "ws_client.h"
#include <json/json.h>
#include <fstream>

const char* const jsonCmdKey = "cmd";
const char* const jsonPathKey = "path";
const char* const jsonPathsKey = "paths";
const char* const jsonLinksKey = "links";
const char* const jsonStatusKey = "status";
const char* const jsonContextKey = "context";

// std::ofstream // log_f;

WSClient::WSClient(WSListenerInterface* listener) : m_open(false), m_connecting(false), m_listener(listener) {
    init_();
    m_listenerInterface2 = false;
}

WSClient::WSClient(WSListenerInterface2* listener) : m_open(false), m_connecting(false), m_listener(listener) {
    init_();
    m_listenerInterface2 = true;
}

void WSClient::init_() {
    // set up access channels to only log interesting things
    m_client.clear_access_channels(websocketpp::log::alevel::all);
    m_client.set_access_channels(websocketpp::log::alevel::connect);
    m_client.set_access_channels(websocketpp::log::alevel::disconnect);
    m_client.set_access_channels(websocketpp::log::alevel::app);

    // Initialize the Asio transport policy
    m_client.init_asio();
    m_client.start_perpetual();

    // Bind the handlers we are using
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;
    m_client.set_open_handler(bind(&WSClient::on_open, this, _1));
    m_client.set_close_handler(bind(&WSClient::on_close, this, _1));
    m_client.set_fail_handler(bind(&WSClient::on_fail, this, _1));
    m_client.set_message_handler(bind(&WSClient::on_message, this, _1, _2));

    //std::ostringstream oss;
    //oss << "C:\\tmp\\plugin_";
    //oss << time(NULL);
    //oss << ".txt";

   // log_f.open(oss.str(), std::ios::out);

}

void WSClient::start(){
    m_thread = websocketpp::lib::thread(&WSClient::start_, this);
}

void WSClient::start_(){
    // log_f << "log " << "WS client started" << std::endl;
    // Create a thread to run the ASIO io_service event loop
    websocketpp::lib::thread asio_thread(&client::run, &m_client);
    while (!m_stop) {
        if (!m_connecting && !m_open) {
            connect();
        }
#ifdef _WIN32
        Sleep(2000);
#else
        sleep(2);
#endif
    }
    asio_thread.join();
    // log_f << "log " << "Run finished" << std::endl;
}

void WSClient::stop() {
    scoped_lock guard(m_lock);
    m_client.stop_perpetual();
    m_stop = true;
    m_lock.unlock();
    close_();
}

void WSClient::close_() {
    try {
        if (is_connected())
        {
            int close_code = websocketpp::close::status::normal;
            m_client.close(m_hdl, close_code, "WSClient closed");
        }
        // log_f << "log " << "WS client closed" << std::endl;
    }
    catch (const websocketpp::exception& e) {
        m_client.get_alog().write(websocketpp::log::alevel::app,
            e.what());
        // log_f << "log " << "Exception in close " << e.what() << std::endl;
    }
    m_thread.join();
}

std::string WSClient::get_port_() {
    std::string port = "";
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    BOOL result = SHGetSpecialFolderPathW(NULL, buffer, CSIDL_LOCAL_APPDATA, false);
    if (!result) {
        m_client.get_alog().write(websocketpp::log::alevel::app, 
            "No appdata folder");
        return port;
    }
    std::wstring path = L"";
    path += buffer;
    path += L"\\.pvtbox\\pvtbox.port";
#else
    std::string path = "/.pvtbox/pvtbox.port";
    path.insert(0, getenv("HOME"));
#endif
    std::ifstream file(path);
    if (!file)
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "No port file");
    else {
        file >> port;
    }
    return port;
}

void WSClient::connect() {
    std::string uri = "ws://127.0.0.1:";
    std::string port = get_port_();
    if (port.empty())
        return;

    // log_f << "log " << "found port" << std::endl;
    uri += port;
    // Create a new connection to the given URI
    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_client.get_connection(uri, ec);
    if (ec) {
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "Get Connection Error: " + ec.message());
        // log_f << "log " << "Get Connection Error: " << ec.message() << std::endl;
        return;
    }

    // Grab a handle for this connection so we can talk to it in a thread
    // safe manor after the event loop starts.
    m_hdl = con->get_handle();

    // Queue the connection. No DNS queries or network connections will be
    // made until the io_service event loop is run.
    m_client.connect(con);
    // log_f << "log " << "Connecting..." << std::endl;
    scoped_lock guard(m_lock);
    m_connecting = true;
}

void WSClient::on_open(websocketpp::connection_hdl) {
    m_client.get_alog().write(websocketpp::log::alevel::app,
        "Connection opened");
    // log_f << "log " << "Connection opened" << std::endl;

    scoped_lock guard(m_lock);
    m_open = true;
    m_connecting = false;
}

void WSClient::on_close(websocketpp::connection_hdl) {
    m_client.get_alog().write(websocketpp::log::alevel::app,
        "Connection closed");
    // log_f << "log " << "Connection closed" << std::endl;
    on_close_();
}

void WSClient::on_fail(websocketpp::connection_hdl) {
    m_client.get_alog().write(websocketpp::log::alevel::app,
        "Connection failed");
    // log_f << "log " << "Connection failed" << std::endl;
    on_close_();
}

void WSClient::on_close_() {
    std::vector<std::string> paths;
    // clear cash
    m_listener->on_message("sync_dir", "", paths, "");

    scoped_lock guard(m_lock);
    m_open = false;
    m_connecting = false;
}

std::string createJsonMessage(const std::string& cmd,
    const std::string& fullPathUtf8) {
    Json::Value msg;

    msg[jsonCmdKey] = std::string(cmd);
    if (!fullPathUtf8.empty())
        msg[jsonPathKey] = fullPathUtf8;

    return Json::FastWriter().write(msg);
}

std::string createJsonMessage2(const std::string& cmd, const std::vector<std::string> paths,
    const std::string& context) {
    Json::Value msg;

    msg[jsonCmdKey] = std::string(cmd);    
    msg[jsonContextKey] = std::string(context);
    
    Json::Value p;
    for (auto&& path: paths)
        p.append(path);
    msg[jsonPathsKey] = p;

    return Json::FastWriter().write(msg);
}

void WSClient::on_message(websocketpp::connection_hdl, message_ptr msg_ptr){
    Json::Value msg;
    Json::Reader reader;
    Json::Value defaultValue;

    std::string message = msg_ptr->get_payload();

    // log_f << "log " << "Got message " << message << std::endl;
    if (!reader.parse(message, msg)){
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "Receive Error in: " + message);
        // log_f << "log " << "Receive Error" << std::endl;
        return;
    }
    const Json::Value& cmdValue = msg.get(jsonCmdKey, defaultValue);
    if (cmdValue == defaultValue) {
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "No command in message: " + message);
        // log_f << "log " << "No command in message: " << std::endl;
        return;
    }

    std::vector<std::string> paths;
    if (msg.isMember(jsonPathsKey)) {
        for (auto itr : msg[jsonPathsKey]) {
            paths.push_back(itr.asString());
        }
    }
    std::vector<std::string> links;
    if (msg.isMember(jsonLinksKey)) {
        for (auto itr : msg[jsonLinksKey]) {
            links.push_back(itr.asString());
        }
    }
    const Json::Value& pathValue = msg.get(jsonPathKey, defaultValue);
    const Json::Value& statusValue = msg.get(jsonStatusKey, defaultValue);
    const Json::Value& contextValue = msg.get(jsonContextKey, defaultValue);
    if (contextValue == defaultValue) {
        m_listener->on_message(cmdValue.asString(), pathValue.asString(), paths, statusValue.asString());
    }
    else if(m_listenerInterface2) {
        WSListenerInterface2* listener2 = (WSListenerInterface2*)m_listener;
        listener2->on_message(cmdValue.asString(), pathValue.asString(), paths, links, statusValue.asString(), contextValue.asString());
    }
}

void WSClient::send_message(const std::string& command, const std::string& path) {
    websocketpp::lib::error_code ec;

    if (!m_open)
        return;
    // log_f << "log " << "Sending message " << command << "" << path << std::endl;
    std::string msg_str = createJsonMessage(command, path);
    m_client.send(m_hdl, msg_str, websocketpp::frame::opcode::text, ec);
    if (ec) {
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "Send Error: " + ec.message());
        // log_f << "log " << "Send Error: " << ec.message() << std::endl;
    }
}

void WSClient::send_message(const std::string& command, const std::vector<std::string> paths, const std::string& context) {
    websocketpp::lib::error_code ec;

    if (!m_open)
        return;
    // log_f << "log " << "Sending message2 " << command << "" << context << std::endl;
    std::string msg_str = createJsonMessage2(command, paths, context);
    m_client.send(m_hdl, msg_str, websocketpp::frame::opcode::text, ec);
    if (ec) {
        m_client.get_alog().write(websocketpp::log::alevel::app,
            "Send Error: " + ec.message());
        // log_f << "log " << "Send2 Error: " << ec.message() << std::endl;
    }
}

bool WSClient::is_connected() {
    return m_open;
}
