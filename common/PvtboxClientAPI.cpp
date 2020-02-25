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
#include "PvtboxClientAPI.h"

#ifdef _WIN32
#include <windows.h>
#include <Lmcons.h>
#include <valarray>
#include <iomanip>
#include <memory>
#include "../windows/FileLogger.h"
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <json/json.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <sstream>
#include <vector>

const char* const jsonCmdKey = "cmd";
const char* const jsonPathKey = "path";
const char* const jsonPathsKey = "paths";

//=============================================================================
// Clobal variable to store socket connection error flag
int socket_connection_error = 0;
//=============================================================================
// Clobal variable to store app start command_line
#ifdef _WIN32
std::wstring commandLine = {};
#else
std::string commandLine = "pvtbox";
#endif

//=============================================================================
// Guard for the nanomsg string
struct NnStringGuard {
    char* str;

    NnStringGuard()
        : str(0) {
    }

    ~NnStringGuard() {
        if (str)
            nn_freemsg(str);
    }
};

//=============================================================================
// Guard for the nanomsg socket with some utility functions
struct NanomsgSoket {
    //=============================================================================
    // Constructor
    NanomsgSoket(int domain,
                 int protocol)
        : socket_(nn_socket(domain, protocol)) {

        if (socket_ < 0)
            throw std::runtime_error("Can't ctreate socket");
    }

    //=============================================================================
    // Destructor
    ~NanomsgSoket() {
        if (socket_ >= 0)
            nn_close(socket_);
    }

    //=============================================================================
    // Initialize socket with parameters
    void connect(const char* url,
                 const int timeout) {
        if (url[0] == '\0'){
            socket_connection_error = 1;
            throw std::runtime_error("Can't connect socket");
        }
        if (nn_connect(socket_, url) < 0){
            throw std::runtime_error("Can't connect socket");
        }

        const bool succeed = (0 == nn_setsockopt(socket_,
                                                 NN_SOL_SOCKET,
                                                 NN_SNDTIMEO,
                                                 &timeout,
                                                 sizeof(timeout)))
            && (0 == nn_setsockopt(socket_,
                                   NN_SOL_SOCKET,
                                   NN_RCVTIMEO,
                                   &timeout,
                                   sizeof(timeout)));

        if (!succeed)
            throw std::runtime_error("Can't set socket options");
    }

    //=============================================================================
    // Sent custom message to socket
    void send(const std::string& message) {
        if (message.size() != size_t(nn_send(socket_,
                                             message.c_str(),
                                             message.size(),
                                             0))){ 
            socket_connection_error = 1;
            throw std::runtime_error("Can't send message " + message);
        }
    }

    //=============================================================================
    // Receive message from socket
    std::string receive() {
        NnStringGuard receiveBuffer;
        const int bytesReceived = nn_recv(socket_,
                                          &receiveBuffer.str,
                                          NN_MSG,
                                          0);
        if (bytesReceived == 0)
            throw std::runtime_error("Can't receive message");

        return std::string(receiveBuffer.str, bytesReceived);
    }

private:
    const int socket_;
};

#ifdef _WIN32
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

std::string ToHex(const std::string& s)
{
    std::ostringstream ret;
    for (std::string::size_type i = 0; i < s.length(); ++i)
        ret << std::hex
        << std::setfill('0')
        << std::setw(2)
        << +(unsigned char)s[i];
    std::string res(ret.str());

    return res;
}

std::unique_ptr<wchar_t> get_user_name()
{
    DWORD username_len = UNLEN + 1;
    std::unique_ptr<wchar_t> username(new wchar_t[username_len]());
    if (!GetUserNameW(username.get(), &username_len))
    {
        throw std::runtime_error("Can't determine user name");
    }
    return username;
}
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

void start(LPCWSTR module, LPWSTR cmdLine)
{
    FileLogger::write((std::string("PvtboxClientAPI::start ") + ws2utf8(cmdLine)).c_str());
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcessW(module,   // module name
        cmdLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0x00000008,     // Detached process
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        FileLogger::write("CreateProcess failed");
        FileLogger::write(GetLastErrorAsString().c_str());
        return;
    }

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

bool PipeExists(const std::string user_name) {
    return true;
}
#else
bool socket_file_exists(const char *file_name) {
    struct stat buffer;
    return (stat(file_name, &buffer) == 0);
}

#endif

//=============================================================================
// Returns nanomsg socket/pipe name depending for current user
std::string get_sock_addr()
{
    std::ostringstream oss;
    std::string empty_addr = "";
    oss << "ipc:///tmp/pvtbox_";

    #ifdef _WIN32
    // On win use UTF8-encoded name of the user
    std::string user_name = ToHex(ws2utf8(get_user_name().get()));
    if (!PipeExists(user_name)) 
        return empty_addr;
   
    oss << user_name << ".ipc";
    std::string sock_addr(oss.str());
    #else
    // On posix use UID to distinguish socket names for different users
    oss << getuid() << ".ipc";
    std::string sock_addr(oss.str());
    if (!socket_file_exists(&sock_addr[6])) 
        return empty_addr;
    #endif
    
    return sock_addr;
}

//=============================================================================
// Send custom message to Pvtbox client and return the answer
std::string sendMessage(const std::string& message) {
#ifdef _WIN32
    FileLogger::write((std::string("PvtboxClientAPI::sendMessage: ") + message).c_str());
#endif
    NanomsgSoket socket(AF_SP, NN_PAIR);
    socket.connect(get_sock_addr().c_str(), 1000);
    socket.send(message);
    return socket.receive();
}

//=============================================================================
//
std::string createJsonMessage_(const std::string& cmd,
                               const std::vector<std::string>& selectedFilesPaths) {
#ifdef _WIN32
    FileLogger::write((std::string("PvtboxClientAPI::createJsonMessage ") + cmd).c_str());
#endif
    Json::Value msg;

    msg[jsonCmdKey] = std::string(cmd);
    for (std::vector<std::string>::const_iterator it = selectedFilesPaths.cbegin(); 
        it != selectedFilesPaths.cend();
        ++it) {
        msg[jsonPathsKey].append(*it);
    }

    return Json::FastWriter().write(msg);
}

//=============================================================================
// Format message and send it
// Return parsed answer
Json::Value sendJsonMessage(const std::string& cmd,
                            const std::vector<std::string>& selectedFilesPaths) {
#ifdef _WIN32
    FileLogger::write((std::string("PvtboxClientAPI::sendJsonMessage ") + cmd).c_str());
#endif
    std::string answer
        = sendMessage(createJsonMessage_(cmd,
                                         selectedFilesPaths));
#ifdef _WIN32
    FileLogger::write((std::string("PvtboxClientAPI::sendJsonMessage answer ") + answer).c_str());
#endif
    Json::Value msg;
    Json::Reader reader;
    if (!reader.parse(answer.c_str(), msg)) {
#ifdef _WIN32
        FileLogger::write("PvtboxClientApi::sendJsonMessage failed to parse json");
#endif
        throw std::runtime_error("Failed to parse answer");
    }

    if (msg[jsonCmdKey] != cmd)
        throw std::runtime_error("Answer to another command was received");

    return msg;
}

//=============================================================================
// Set app start command line file path
// path must not have trailing slash '/'
#ifdef _WIN32
void PvtboxClientAPI::initialize(const wchar_t* path) {
    FileLogger::write("PvtboxClientAPI::initialize");
    commandLine.assign(path);
    commandLine.append(L"\\pvtbox.exe");
}
#endif

//=============================================================================
// Request sychronized folder path from Pvtbox client
// Return UTF8 encoded full path to synchronized folder
std::string PvtboxClientAPI::getSyncDir() {
    try {
        Json::Value answer = sendJsonMessage("sync_dir",
                                             std::vector<std::string>());

        Json::Value defaultValue;
        Json::Value dirValue = answer.get(jsonPathKey,
                                          defaultValue);
        return dirValue.asString();
    }
    catch (const std::runtime_error& e) {
        if (socket_connection_error) {
            socket_connection_error = 0;
            return std::string();
        }
        else
            throw e;
    }
}

//=============================================================================
// Copy file pointed by fullPathUtf8 to synchronized directory
void PvtboxClientAPI::copyToSyncDir(const std::vector<std::string>& selectedFilesPaths) {
#ifdef _WIN32
    FileLogger::write("PvtboxClientAPI::copyToSyncDir");
#endif
    try {
        sendJsonMessage("copy_to_sync_dir",
                        selectedFilesPaths);
    }
    catch (const std::runtime_error& e) {
        if (socket_connection_error) {
            socket_connection_error = 0;
#ifdef _WIN32
            FileLogger::write("PvtboxClientAPI::copyToSyncDir socket connection error, launch application");
#endif
             if (commandLine.length()){
#ifdef _WIN32
                 std::wstring cmd = L"\"";
                 cmd.append(commandLine);
                 cmd.append(L"\"");
#else
                 std::string cmd = commandLine;
#endif
                 for (std::vector<std::string>::const_iterator it = selectedFilesPaths.cbegin();
                     it != selectedFilesPaths.cend();
                     ++it) {
#ifdef _WIN32
                     cmd.append(L" --copy \"");
                     cmd.append(utf82ws(*it));
                     cmd.append(L"\"");
#else
                     cmd.append(" --copy \"");
                     cmd.append(*it);
                     cmd.append("\"");
#endif
                 }
#ifdef _WIN32
                 start(commandLine.c_str(), &cmd[0]);
#else
                 cmd.append(" >/dev/null 2>&1 &");
                 system(cmd.c_str());
#endif
            }
            else
                throw e;
        }
        else
            throw e;
    }
}

//=============================================================================
// Copy link to specified file
void PvtboxClientAPI::sharePath(const std::vector<std::string>& selectedFilesPaths) {
    sendJsonMessage("share_path",
                    selectedFilesPaths);
}

//=============================================================================
// Close public access to path
void PvtboxClientAPI::blockPath(const std::vector<std::string>& selectedFilesPaths) {
    sendJsonMessage("block_path",
                    selectedFilesPaths);
}

//=============================================================================
// Send link to file by e-mail
void PvtboxClientAPI::emailLink(const std::vector<std::string>& selectedFilesPaths) {
    sendJsonMessage("email_link",
                    selectedFilesPaths);
}

//=============================================================================
// View file on the web site
void PvtboxClientAPI::openLink(const std::vector<std::string>& selectedFilesPaths) {
    sendJsonMessage("open_link",
                    selectedFilesPaths);
}

//=============================================================================
//Check if file has link 
std::string PvtboxClientAPI::isShared(const std::vector<std::string>& selectedFilesPaths) {
#ifdef _WIN32
    FileLogger::write("PvtboxClientAPI::isShared");
#endif
    Json::Value answer = sendJsonMessage("is_shared",
                                         selectedFilesPaths);
    Json::Value defaultValue;
    Json::Value isSharedValue = answer.get(jsonPathKey,
                                           defaultValue);
    return isSharedValue.asString();
}

//=============================================================================
// Open collaboration settings
void PvtboxClientAPI::collaborationSettings(const std::vector<std::string>& selectedFilesPaths) {
    sendJsonMessage("collaboration_settings",
        selectedFilesPaths);
}