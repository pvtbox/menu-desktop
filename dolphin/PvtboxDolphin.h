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
#pragma once

#include <KAbstractFileItemActionPlugin>
#include <KActionCollection>


class MenuItemDescription;


//=============================================================================
// Pvtbox plugin class
class PvtboxPlugin : public KAbstractFileItemActionPlugin
{
    Q_OBJECT

public:
    PvtboxPlugin(QObject* parent = 0, const QVariantList& args = QVariantList());
    ~PvtboxPlugin();
    QList<QAction*> actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget);

private slots:
    void handleAction(QAction* action);

private:
    void buildMenu(QStringList* paths,
                   KActionCollection* actions,
                   const MenuItemDescription* parent = NULL) const;

private:
    class PImpl;
    PImpl* const pimpl;
};
