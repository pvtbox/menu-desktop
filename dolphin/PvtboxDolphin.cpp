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
#include <KPluginFactory>
#include <KFileItemListProperties>
#include <KFileItem>
#include <KActionCollection>
#include <KLocalizedString>
#include <QMenu>
#include <QPointer>

#include <iostream>

#include "../common/Consts.h"
#include "../common/MenuDescription.h"
#include "PvtboxDolphin.h"


//=============================================================================
// PImpl class of PIMPL
class PvtboxPlugin::PImpl
{
public:
    PImpl(PvtboxPlugin* parent)
        : paths(),
          actions(new KActionCollection(parent)) {
    }

    QStringList paths;
    QPointer<KActionCollection> actions;
};


//=============================================================================
// Constructor
PvtboxPlugin::PvtboxPlugin(QObject* parent, const QVariantList& args)
    : KAbstractFileItemActionPlugin(parent),
      pimpl(new PImpl(this))
{
    Q_UNUSED(args);

    std::cout << "pvtbox: PvtboxPlugin is started\n";

    connect(pimpl->actions, SIGNAL(actionTriggered(QAction*)), this, SLOT(handleAction(QAction*)));

    KLocalizedString::setApplicationDomain(gettextDomain);
}


//=============================================================================
// Destructor
PvtboxPlugin::~PvtboxPlugin()
{
    std::cout << "pvtbox: PvtboxPlugin is finished\n";
    delete pimpl;
}


//=============================================================================
// Recursively build Pvtbox context menu
void PvtboxPlugin::buildMenu(QStringList* paths,
                             KActionCollection* actions,
                             const MenuItemDescription* parent) const
{
    MenuDescription menuDescription;

    std::vector<std::string> paths_v;
    for (int i = 0; i < paths->size(); ++i)
    {
        paths_v.push_back(paths->at(i).toStdString());
    }

    try
    {
        menuDescription = getMenu(paths_v, parent);
    }
    catch (std::runtime_error e)
    {
        std::cout << "pvtbox: cannot get menu\n";
        return;
    }

    for (int i = 0; i < menuDescription.size; ++i) {
        const MenuItemDescription& menuItem = menuDescription.items[i];
        QAction* action = actions->addAction(menuItem.name);
        action->setText(i18n(menuItem.label));
        action->setToolTip(i18n(menuItem.tip));
        action->setIcon(QIcon::fromTheme(menuItem.iconName));
        action->setData(QVariant(reinterpret_cast<qulonglong>(menuItem.action)));

        if (menuItem.hasChildren())
        {
            QMenu* subMenu = new QMenu();
            action->setMenu(subMenu);

            connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(handleAction(QAction*)));

            const MenuDescription& subMenuDescription = getMenu(paths_v,
                                                                &menuItem);
            for (int j = 0; j < subMenuDescription.size; ++j)
            {
                const MenuItemDescription& subMenuItem = subMenuDescription.items[j];
                QAction* subAction = subMenu->addAction(subMenuItem.name);
                subAction->setText(i18n(subMenuItem.label));
                subAction->setToolTip(i18n(subMenuItem.tip));
                subAction->setIcon(QIcon::fromTheme(subMenuItem.iconName));
                subAction->setData(QVariant(reinterpret_cast<qulonglong>(subMenuItem.action)));
            }
        }
    }
}


//=============================================================================
// Return List of actions that are available for the items
QList<QAction*> PvtboxPlugin::actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget)
{
    std::cout << "pvtbox: actions is requested\n";

    Q_UNUSED(parentWidget);

    KFileItemList items = fileItemInfos.items();
    Q_ASSERT(!items.isEmpty());

    pimpl->actions->clear();
    pimpl->paths.clear();

    foreach (const KFileItem& item, items)
    {
        if (!item.isLocalFile())
            return QList<QAction*>();

        pimpl->paths << item.localPath();
    }

    buildMenu(&pimpl->paths,
              pimpl->actions,
              NULL);

    return pimpl->actions->actions();
}


//=============================================================================
// Run selected action
void PvtboxPlugin::handleAction(QAction* action)
{
    std::cout << "pvtbox: handleActions '" << action->text().toStdString() << std::endl;

    static_assert(sizeof(void*) <= sizeof(qulonglong), "Pointer bitness violation!");
    const FileAction fileAction = reinterpret_cast<FileAction>(action->data().toULongLong());
    if (!fileAction)
        return;

    std::vector<std::string> paths;
    for (int i = 0; i < pimpl->paths.size(); ++i)
    {
        std::cout << "pvtbox: " << pimpl->paths[i].toUtf8().constData() << std::endl;
        paths.push_back(pimpl->paths[i].toUtf8().constData());
    }
    fileAction(paths);
}


//=============================================================================
// Register plugin
K_PLUGIN_CLASS_WITH_JSON(PvtboxPlugin, "pvtbox-dolphin-menu.json")

#include "PvtboxDolphin.moc"
