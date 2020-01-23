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
#include <QUrl>
#include <QDebug>

#include <iostream>
#include <cstring>

#include "PvtboxOverlayPlugin.h"


#define Q_DEBUG qDebug() << "net.pvtbox.overlayiconplugin: "


//=============================================================================
// Constructor
PvtboxOverlayPlugin::PvtboxOverlayPlugin()
    : ws_listener(new WSListener(this))
{
    Q_DEBUG << "plugin has started";
}


//=============================================================================
// Destructor
PvtboxOverlayPlugin::~PvtboxOverlayPlugin()
{
    Q_DEBUG << "plugin has finished";
}


//=============================================================================
QStringList PvtboxOverlayPlugin::getOverlays (const QUrl &url)
{
    if (!url.isLocalFile())
    {
        //not local file
        return QStringList();
    }

    std::string path = url.toLocalFile().toStdString();
    std::string status = ws_listener->getStatus(path);

    if ( status == "")
    {
        ws_listener->addPath(path);
        return QStringList();
    }

    QStringList overlays;

    if (status == "synced")
        overlays << "pvtbox-synced";
    else if (status == "syncing")
        overlays << "pvtbox-syncing";
    else if (status ==  "paused")
        overlays << "pvtbox-paused";
    else if (status == "error")
        overlays << "pvtbox-error";

    return overlays;
}


//=============================================================================
PvtboxOverlayPlugin::WSListener::WSListener(PvtboxOverlayPlugin* parent)
    : ws_client(this), sync_dir(""), parent(parent)
{
    ws_client.start();
}


//=============================================================================
PvtboxOverlayPlugin::WSListener::~WSListener()
{
    ws_client.stop();
}


//=============================================================================
void PvtboxOverlayPlugin::WSListener::on_message(
        const std::string & command,
        const std::string & path,
        std::vector<std::string> & paths,
        const std::string & status)
{
    if (command == "sync_dir")
    {
        if (sync_dir == "" && path != "")
        {
            Q_DEBUG << "client has connected";
            for ( std::string deferredPath: deferredPaths)
            {
                if (std::strstr(deferredPath.c_str(), (path + "/").c_str()) == deferredPath.c_str())
                {
                    // Send message to subscribe if path inside sync dir
                    Q_DEBUG << "send subscribe for: " << QString(deferredPath.c_str());
                    ws_client.send_message("status_subscribe", deferredPath);
                }
            }
            deferredPaths.clear();
        }
        else if (sync_dir != "" && path == "")
        {
            Q_DEBUG << "client has disconnected";
            deferredPaths.clear();
            for (auto subscribed_path : subscribedPaths)
            {
                deferredPaths.insert(subscribed_path.first);
            }
            subscribedPaths.clear();
            for (auto p : deferredPaths)
            {
                QUrl url = QUrl::fromLocalFile(QString(p.c_str()));
                emit parent->overlaysChanged(url, QStringList());
            }
        }
        else if (sync_dir != "" && path != "")
        {
            Q_DEBUG << "sync dir has changed";
        }

        sync_dir = std::string(path);
    }
    else if (command == "status")
    {
        for (auto p : paths)
        {
            Q_DEBUG << QString(status.c_str()) << " -- " << QString(p.c_str());
            subscribedPaths[p] = status;
            QUrl url = QUrl::fromLocalFile(QString(p.c_str()));
            emit parent->overlaysChanged(url, QStringList());
        }
    }
    else
    {
        //
    }
}


//=============================================================================
void PvtboxOverlayPlugin::WSListener::addPath(const std::string & path)
{
    std::string current_dir = path.substr(0, path.find_last_of("/"));

    if (sync_dir == "")
    {
        if ( !deferredPaths.empty() )
        {
            std::string s = *deferredPaths.begin();
            std::string deffered_dir = s.substr(0, s.find_last_of("/"));
            if (current_dir != deffered_dir)
            {
                deferredPaths.clear();
            }
        }

        deferredPaths.insert(path);
    }
    else
    {
        // Unsubscribe
        std::string s = "";
        for (auto p : subscribedPaths)
        {
            // Find path to obtain subscribed_dir.
            // Here we need exclude sync_dir that might be subscribed.
            if (p.first != sync_dir)
            {
                s = p.first;
                break;
            }
        }
        std::string subscribed_dir = s.substr(0, s.find_last_of("/"));
        if (s != "" && current_dir != subscribed_dir)
        {
            subscribedPaths.clear();

            Q_DEBUG << "send unsubscribe for: " << QString(subscribed_dir.c_str());
            ws_client.send_message("status_unsubscribe", subscribed_dir);
        }

        // Send message to subscribe if path inside sync dir
        if (std::strstr(path.c_str(), (sync_dir + "/").c_str()) == path.c_str()
            || path == sync_dir)
        {
            Q_DEBUG << "send subscribe for: " << QString(path.c_str());
            ws_client.send_message("status_subscribe", path);
        }
    }
}


//=============================================================================
std::string PvtboxOverlayPlugin::WSListener::getStatus(const std::string & path)
{
    auto subscribed_path = subscribedPaths.find(path);
    return subscribed_path != subscribedPaths.end() ? subscribed_path->second : "";
}


#include "PvtboxOverlayPlugin.moc"
