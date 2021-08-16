//
// Created by ragdoll on 2021/8/15.
//

#include "AppMenuButtonGroup.h"
#include "debug.h"
#include "TextButton.h"
#include "MenuOverflowButton.h"

#include <QMenu>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>

using Material::TextButton;
using Material::MenuOverflowButton;

AppMenuButtonGroup::AppMenuButtonGroup(Decoration *decoration)
    : DecorationButtonGroup(decoration)
    , m_appMenuModel(nullptr)
    , m_currentIndex(-1)
    , m_overflowIndex(-1)
    , m_hovered(false)
    , m_showing(true)
    , m_alwaysShow(true)
{
    this->m_appMenuModel = new AppMenuModel(this);
    connect(this->m_appMenuModel, &AppMenuModel::modelNeedsUpdate, this, &AppMenuButtonGroup::updateMenu);
    this->m_appMenuModel->setWinId(this->windowId());

    qCDebug(category) << this->windowId() << "AppMenuButtonGroup init ends";
}

WId AppMenuButtonGroup::windowId() const {
    return this->decoration().data()->client().toStrongRef().data()->windowId();
}

void AppMenuButtonGroup::updateMenu() {
    qCDebug(category) << this->windowId() << "AppMenuButtonGroup updateMenu starts with"
                      << this->m_appMenuModel->rowCount() << "menu items";
    auto *deco = qobject_cast<Chameleon *>(decoration());
    if (!deco) {
        qDebug() << "Decoration::updateAppMenuModel exits due to nullptr of deco";
        return;
    }
    auto *decoratedClient = deco->client().toStrongRef().data();

    // Don't display AppMenu in modal windows.
    if (decoratedClient->isModal()) {
        resetButtons();
        qDebug() << "Decoration::updateAppMenuModel exits due to isModal";
        return;
    }

    resetButtons();

    // Populate
    for (int row = 0; row < m_appMenuModel->rowCount(); row++) {
        const QModelIndex index = m_appMenuModel->index(row, 0);
        QString itemLabel = m_appMenuModel->data(index, AppMenuModel::MenuRole).toString();

        // https://github.com/psifidotos/applet-window-appmenu/blob/908e60831d7d68ee56a56f9c24017a71822fc02d/lib/appmenuapplet.cpp#L167
        const QVariant data = m_appMenuModel->data(index, AppMenuModel::ActionRole);
        QAction *itemAction = (QAction *)data.value<void *>();

        qCDebug(category) << this->windowId() << itemLabel;

        TextButton *b = new TextButton(deco, row, this);
        QString menuStr = itemLabel.remove('&');
        int cIndex = menuStr.lastIndexOf('(');
        if (cIndex > 0) {
            menuStr = menuStr.mid(0, cIndex);
        }
        b->setText(QString("  %1  ").arg(menuStr));
        b->setAction(itemAction);

        // Skip items with empty labels (The first item in a Gtk app)
        if (itemLabel.isEmpty()) {
            b->setEnabled(false);
            b->setVisible(false);
        }

        addButton(QPointer<KDecoration2::DecorationButton>(b));
    }
    m_overflowIndex = m_appMenuModel->rowCount();
//    addButton(new MenuOverflowButton(deco, m_overflowIndex, this));

    emit menuUpdated();
    qCDebug(category) << this->windowId() << "AppMenuButtonGroup updateMenu ends";
}

void AppMenuButtonGroup::resetButtons() {
    auto list = QVector<QPointer<KDecoration2::DecorationButton>>(buttons());
    removeButton(KDecoration2::DecorationButtonType::Custom);
    while (!list.isEmpty()) {
        auto item = list.takeFirst();
        delete item;
    }
    emit menuUpdated();
}

void AppMenuButtonGroup::trigger(int buttonIndex) {
    // qCDebug(category) << "AppMenuButtonGroup::trigger" << buttonIndex;
    KDecoration2::DecorationButton* button = buttons().value(buttonIndex);

    // https://github.com/psifidotos/applet-window-appmenu/blob/908e60831d7d68ee56a56f9c24017a71822fc02d/lib/appmenuapplet.cpp#L167
    QMenu *actionMenu = nullptr;

    if (buttonIndex == m_appMenuModel->rowCount()) {
        // Overflow Menu
        actionMenu = new QMenu();
        actionMenu->setAttribute(Qt::WA_DeleteOnClose);

        int overflowStartsAt = 0;
        for (KDecoration2::DecorationButton *b : buttons()) {
            TextButton* textButton = qobject_cast<TextButton *>(b);
            if (textButton && textButton->isEnabled() && !textButton->isVisible()) {
                overflowStartsAt = textButton->buttonIndex();
                break;
            }
        }

        QAction *action = nullptr;
        for (int i = overflowStartsAt; i < m_appMenuModel->rowCount(); i++) {
            const QModelIndex index = m_appMenuModel->index(i, 0);
            const QVariant data = m_appMenuModel->data(index, AppMenuModel::ActionRole);
            action = (QAction *)data.value<void *>();
            actionMenu->addAction(action);
        }

    } else {
        const QModelIndex modelIndex = m_appMenuModel->index(buttonIndex, 0);
        const QVariant data = m_appMenuModel->data(modelIndex, AppMenuModel::ActionRole);
        QAction *itemAction = (QAction *)data.value<void *>();
        // qCDebug(category) << "    action" << itemAction;

        if (itemAction) {
            actionMenu = itemAction->menu();
            // qCDebug(category) << "    menu" << actionMenu;
        }
    }

    const auto *deco = qobject_cast<Chameleon *>(decoration());
    // if (actionMenu && deco) {
    //     auto *decoratedClient = deco->client().toStrongRef().data();
    //     actionMenu->setPalette(decoratedClient->palette());
    // }

    if (actionMenu && deco) {
        QRectF buttonRect = button->geometry();
        QPoint position = buttonRect.topLeft().toPoint();
        QPoint rootPosition(position);
        rootPosition += deco->windowPos();
        // qCDebug(category) << "    windowPos" << windowPos;

        // auto connection( QX11Info::connection() );

        // button release event
        // xcb_button_release_event_t releaseEvent;
        // memset(&releaseEvent, 0, sizeof(releaseEvent));

        // releaseEvent.response_type = XCB_BUTTON_RELEASE;
        // releaseEvent.event =  windowId;
        // releaseEvent.child = XCB_WINDOW_NONE;
        // releaseEvent.root = QX11Info::appRootWindow();
        // releaseEvent.event_x = position.x();
        // releaseEvent.event_y = position.y();
        // releaseEvent.root_x = rootPosition.x();
        // releaseEvent.root_y = rootPosition.y();
        // releaseEvent.detail = XCB_BUTTON_INDEX_1;
        // releaseEvent.state = XCB_BUTTON_MASK_1;
        // releaseEvent.time = XCB_CURRENT_TIME;
        // releaseEvent.same_screen = true;
        // xcb_send_event( connection, false, windowId, XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char*>(&releaseEvent));

        // xcb_ungrab_pointer( connection, XCB_TIME_CURRENT_TIME );
        //---

        actionMenu->installEventFilter(this);

        if (!KWindowSystem::isPlatformWayland()) {
            actionMenu->popup(rootPosition);
        }

        QMenu *oldMenu = m_currentMenu;
        m_currentMenu = actionMenu;

        if (oldMenu && oldMenu != actionMenu) {
            // Don't reset the currentIndex when another menu is already shown
            disconnect(oldMenu, &QMenu::aboutToHide, this, &AppMenuButtonGroup::onMenuAboutToHide);
            oldMenu->hide();
        }
        if (0 <= m_currentIndex && m_currentIndex < buttons().length()) {
            buttons().value(m_currentIndex)->setChecked(false);
        }

        if (KWindowSystem::isPlatformWayland()) {
            actionMenu->popup(rootPosition);
        }

        setCurrentIndex(buttonIndex);
        button->setChecked(true);

        // FIXME TODO connect only once
        connect(actionMenu, &QMenu::aboutToHide, this, &AppMenuButtonGroup::onMenuAboutToHide, Qt::UniqueConnection);
    }
}

void AppMenuButtonGroup::onMenuAboutToHide() {
    if (0 <= m_currentIndex && m_currentIndex < buttons().length()) {
        buttons().value(m_currentIndex)->setChecked(false);
    }
    setCurrentIndex(-1);
}

int AppMenuButtonGroup::currentIndex() const {
    return m_currentIndex;
}

void AppMenuButtonGroup::setCurrentIndex(int set) {
    if (m_currentIndex != set) {
        m_currentIndex = set;
        // qCDebug(category) << this << "setCurrentIndex" << m_currentIndex;
        emit currentIndexChanged();
    }
}

void AppMenuButtonGroup::updateOverflow(QRectF availableRect) {
    // qCDebug(category) << "updateOverflow" << availableRect;
    bool showOverflow = false;
    for (KDecoration2::DecorationButton *button : buttons()) {
        // qCDebug(category) << "    " << button->geometry() << button;
        if (qobject_cast<MenuOverflowButton *>(button)) {
            button->setVisible(showOverflow);
            // qCDebug(category) << "    showOverflow" << showOverflow;
        } else if (qobject_cast<TextButton *>(button)) {
            if (button->isEnabled()) {
                if (availableRect.contains(button->geometry())) {
                    button->setVisible(true);
                } else {
                    button->setVisible(false);
                    showOverflow = true;
                }
            }
        }
    }
    setOverflowing(showOverflow);
}

bool AppMenuButtonGroup::overflowing() const {
    return m_overflowing;
}

void AppMenuButtonGroup::setOverflowing(bool set) {
    if (m_overflowing != set) {
        m_overflowing = set;
        // qCDebug(category) << this << "setOverflowing" << m_overflowing;
        emit overflowingChanged();
    }
}

bool AppMenuButtonGroup::isMenuOpen() const {
    return 0 <= m_currentIndex;
}
