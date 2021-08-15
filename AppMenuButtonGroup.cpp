//
// Created by ragdoll on 2021/8/15.
//

#include "AppMenuButtonGroup.h"
#include "debug.h"

#include <KDecoration2/DecoratedClient>

AppMenuButtonGroup::AppMenuButtonGroup(Decoration *decoration)
    : DecorationButtonGroup(decoration)
{
    this->appMenuModel = new AppMenuModel(this);
    connect(this->appMenuModel, &AppMenuModel::modelNeedsUpdate, this, &AppMenuButtonGroup::updateMenu);
    this->appMenuModel->setWinId(this->windowId());

    qCDebug(category) << this->windowId() << "AppMenuButtonGroup init ends";
}

WId AppMenuButtonGroup::windowId() const {
    return this->decoration().data()->client().toStrongRef().data()->windowId();
}

void AppMenuButtonGroup::updateMenu() {
    qCDebug(category) << this->windowId() << "AppMenuButtonGroup updateMenu starts with"
                      << this->appMenuModel->rowCount() << "menu items";


    QList<QString> existedMenu;  // tricks for twice menu of libreoffice
    for (int r = 0; r < this->appMenuModel->rowCount(); ++r) {
        QString menuStr = appMenuModel->data(appMenuModel->index(r, 0), AppMenuModel::MenuRole).toString();

        // tricks to remove useless old menus
        existedMenu.append(menuStr);

        qCDebug(category) << this->windowId() << menuStr;
    }
    qCDebug(category) << this->windowId() << "AppMenuButtonGroup updateMenu ends";
}
