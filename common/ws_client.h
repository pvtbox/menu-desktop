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
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/client.hpp>

// extern std::ofstream // log_f;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class WSListenerInterface {
public:
    virtual ~WSListenerInterface() {};
    virtual void on_message(
        const std::string& command,
        const std::string& path,
        std::vector<std::string>& paths,
        const std::string& status) = 0;
};

class WSListenerInterface2 :public WSListenerInterface {
public:
    virtual ~WSListenerInterface2() {};
    virtual void on_message(
        const std::string& command,
        const std::string& path,
        std::vector<std::string>& paths,
        std::vector<std::string>& links,
        const std::string& status,
        const std::string& context) = 0;
};


class WSClient {
public:
    typedef websocketpp::client<websocketpp::config::asio_client> client;
    typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;

    WSClient(WSListenerInterface* listener);
    WSClient(WSListenerInterface2* listener);
    void start();
    void stop();
    void send_message(const std::string& command, const std::string& path);
    void send_message(const std::string& command, const std::vector<std::string> paths, const std::string& context);
    bool is_connected();

protected:
    void connect();
    void on_open(websocketpp::connection_hdl);
    void on_close(websocketpp::connection_hdl);
    void on_fail(websocketpp::connection_hdl);
    void on_message(websocketpp::connection_hdl, message_ptr msg_ptr);

private:
    client m_client;
    websocketpp::connection_hdl m_hdl;
    websocketpp::lib::mutex m_lock;
    bool m_open;
    bool m_connecting;
    WSListenerInterface* m_listener;
    bool m_listenerInterface2;
    websocketpp::lib::thread m_thread;
    mutable bool m_stop = false;
    void start_();
    void on_close_();
    void close_();
    std::string get_port_();
    void init_();
};
