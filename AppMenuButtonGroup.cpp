/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// own
#include "AppMenuButtonGroup.h"
#include "Button.h"
#include "AppMenuModel.h"
#include "AppMenuButton.h"
#include "TextButton.h"
#include "MenuOverflowButton.h"
#include "chameleon.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationButtonGroup>

// Qt
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QVariantAnimation>


namespace Material
{

AppMenuButtonGroup::AppMenuButtonGroup(Chameleon *decoration)
    : KDecoration2::DecorationButtonGroup(decoration)
    , m_appMenuModel(nullptr)
    , m_currentIndex(-1)
    , m_overflowIndex(-1)
    , m_hovered(false)
    , m_showing(true)
    , m_alwaysShow(true)
    , m_opacity(1)
{
    // Assign showing and opacity before we bind the onShowingChanged animation
    // so that new windows do not animate.
    setAlwaysShow(true);
    updateShowing();
    setOpacity(m_showing ? 1 : 0);

    connect(this, &AppMenuButtonGroup::showingChanged,
            this, &AppMenuButtonGroup::onShowingChanged);
    connect(this, &AppMenuButtonGroup::hoveredChanged,
            this, &AppMenuButtonGroup::updateShowing);
    connect(this, &AppMenuButtonGroup::alwaysShowChanged,
            this, &AppMenuButtonGroup::updateShowing);
    connect(this, &AppMenuButtonGroup::currentIndexChanged,
            this, &AppMenuButtonGroup::updateShowing);

    connect(this, &AppMenuButtonGroup::opacityChanged, this, [this]() {
        // update();
    });

    auto *decoratedClient = decoration->client().toStrongRef().data();
    connect(decoratedClient, &KDecoration2::DecoratedClient::hasApplicationMenuChanged,
            this, &AppMenuButtonGroup::updateAppMenuModel);
    connect(this, &AppMenuButtonGroup::requestActivateIndex,
            this, &AppMenuButtonGroup::trigger);
    connect(this, &AppMenuButtonGroup::requestActivateOverflow,
            this, &AppMenuButtonGroup::triggerOverflow);
}

AppMenuButtonGroup::~AppMenuButtonGroup()
{
}

int AppMenuButtonGroup::currentIndex() const
{
    return m_currentIndex;
}

void AppMenuButtonGroup::setCurrentIndex(int set)
{
    if (m_currentIndex != set) {
        m_currentIndex = set;
        // qCDebug(category) << this << "setCurrentIndex" << m_currentIndex;
        emit currentIndexChanged();
    }
}

bool AppMenuButtonGroup::overflowing() const
{
    return m_overflowing;
}

void AppMenuButtonGroup::setOverflowing(bool set)
{
    if (m_overflowing != set) {
        m_overflowing = set;
        // qCDebug(category) << this << "setOverflowing" << m_overflowing;
        emit overflowingChanged();
    }
}

bool AppMenuButtonGroup::hovered() const
{
    return m_hovered;
}

void AppMenuButtonGroup::setHovered(bool value)
{
    if (m_hovered != value) {
        m_hovered = value;
        // qCDebug(category) << this << "setHovered" << m_hovered;
        emit hoveredChanged(value);
    }
}

bool AppMenuButtonGroup::showing() const
{
    return m_showing;
}

void AppMenuButtonGroup::setShowing(bool value)
{
    if (m_showing != value) {
        m_showing = value;
        // qCDebug(category) << this << "setShowing" << m_showing << "alwaysShow" << m_alwaysShow << "currentIndex" << m_currentIndex << "opacity" << m_opacity;
        emit showingChanged(value);
    }
}

bool AppMenuButtonGroup::alwaysShow() const
{
    return m_alwaysShow;
}

void AppMenuButtonGroup::setAlwaysShow(bool value)
{
    if (m_alwaysShow != value) {
        m_alwaysShow = value;
        // qCDebug(category) << this << "setAlwaysShow" << m_alwaysShow;
        emit alwaysShowChanged(value);
    }
}

qreal AppMenuButtonGroup::opacity() const
{
    return m_opacity;
}

void AppMenuButtonGroup::setOpacity(qreal value)
{
    if (m_opacity != value) {
        m_opacity = value;

        for (int i = 0; i < buttons().length(); i++) {
            KDecoration2::DecorationButton* decoButton = buttons().value(i);
            auto *button = qobject_cast<Button *>(decoButton);
            if (button) {
                button->setOpacity(m_opacity);
            }
        }

        emit opacityChanged(value);
    }
}

KDecoration2::DecorationButton* AppMenuButtonGroup::buttonAt(int x, int y) const
{
    for (int i = 0; i < buttons().length(); i++) {
        KDecoration2::DecorationButton* button = buttons().value(i);
        if (!button->isVisible()) {
            continue;
        }
        if (button->geometry().contains(x, y)) {
            return button;
        }
    }
    return nullptr;
}

void AppMenuButtonGroup::resetButtons()
{
    // qCDebug(category) << "    resetButtons";
    // qCDebug(category) << "        before" << buttons();
    auto list = QVector<QPointer<KDecoration2::DecorationButton>>(buttons());
    // qCDebug(category) << "          list" << list;
    removeButton(KDecoration2::DecorationButtonType::Custom);
    // qCDebug(category) << "     remCustom" << buttons();
    while (!list.isEmpty()) {
        auto item = list.takeFirst();
        // qCDebug(category) << "        delete" << item;
        delete item;
    }
    // qCDebug(category) << "         after" << list;
    emit menuUpdated();
}

void AppMenuButtonGroup::initAppMenuModel()
{
    m_appMenuModel = new AppMenuModel(this);
    connect(m_appMenuModel, &AppMenuModel::modelReset,
        this, &AppMenuButtonGroup::updateAppMenuModel);
    // qCDebug(category) << "AppMenuModel" << m_appMenuModel;
}

void AppMenuButtonGroup::updateAppMenuModel()
{
    qDebug() << "Decoration::updateAppMenuModel" << m_appMenuModel;

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

    if (decoratedClient->hasApplicationMenu()) {
        resetButtons();
        qDebug() << "Decoration::updateAppMenuModel exits due to no application menu";
        return;
    }
    qDebug() << "Decoration::AppMenuModel 1" << m_appMenuModel;

    if (m_appMenuModel) {
        // Update AppMenuModel
         qDebug() << "Decoration::AppMenuModel 2" << m_appMenuModel;

        resetButtons();

        // Populate
        for (int row = 0; row < m_appMenuModel->rowCount(); row++) {
            const QModelIndex index = m_appMenuModel->index(row, 0);
            const QString itemLabel = m_appMenuModel->data(index, AppMenuModel::MenuRole).toString();

            // https://github.com/psifidotos/applet-window-appmenu/blob/908e60831d7d68ee56a56f9c24017a71822fc02d/lib/appmenuapplet.cpp#L167
            const QVariant data = m_appMenuModel->data(index, AppMenuModel::ActionRole);
            QAction *itemAction = (QAction *)data.value<void *>();

            // qCDebug(category) << "    " << itemAction;

            TextButton *b = new TextButton(deco, row, this);
            b->setText(itemLabel);
            b->setAction(itemAction);
            b->setOpacity(m_opacity);

            // Skip items with empty labels (The first item in a Gtk app)
            if (itemLabel.isEmpty()) {
                b->setEnabled(false);
                b->setVisible(false);
            }
            
            addButton(QPointer<KDecoration2::DecorationButton>(b));
        }
        m_overflowIndex = m_appMenuModel->rowCount();
        addButton(new MenuOverflowButton(deco, m_overflowIndex, this));

        emit menuUpdated();

    } else {
        // Init AppMenuModel
        // qCDebug(category) << "windowId" << decoratedClient->windowId();
        if (KWindowSystem::isPlatformX11()) {
#if HAVE_X11
            WId windowId = decoratedClient->windowId();
            if (windowId != 0) {
                initAppMenuModel();
                m_appMenuModel->setWinId(windowId);
                // qCDebug(category) << "AppMenuModel" << m_appMenuModel;
            }
#endif
        } else if (KWindowSystem::isPlatformWayland()) {
#if HAVE_Wayland
            // TODO
#endif
        }
    }
}

void AppMenuButtonGroup::updateOverflow(QRectF availableRect)
{
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

void AppMenuButtonGroup::triggerOverflow()
{
    // qCDebug(category) << "AppMenuButtonGroup::triggerOverflow" << m_overflowIndex;
    trigger(m_overflowIndex);
}

// FIXME TODO doesn't work on submenu
bool AppMenuButtonGroup::eventFilter(QObject *watched, QEvent *event)
{
    auto *menu = qobject_cast<QMenu *>(watched);

    if (!menu) {
        return false;
    }

    if (event->type() == QEvent::KeyPress) {
        auto *e = static_cast<QKeyEvent *>(event);

        // TODO right to left languages
        if (e->key() == Qt::Key_Left) {
            int desiredIndex = m_currentIndex - 1;
            emit requestActivateIndex(desiredIndex);
            return true;
        } else if (e->key() == Qt::Key_Right) {
            if (menu->activeAction() && menu->activeAction()->menu()) {
                return false;
            }

            int desiredIndex = m_currentIndex + 1;
            emit requestActivateIndex(desiredIndex);
            return true;
        }

    } else if (event->type() == QEvent::MouseMove) {
        auto *e = static_cast<QMouseEvent *>(event);

        const auto *deco = qobject_cast<Chameleon *>(decoration());

        QPoint decoPos(e->globalPos());
        decoPos -= deco->windowPos();
        decoPos.ry() += deco->titleBarHeight();
        // qCDebug(category) << "MouseMove";
        // qCDebug(category) << "       globalPos" << e->globalPos();
        // qCDebug(category) << "       windowPos" << deco->windowPos();
        // qCDebug(category) << "  titleBarHeight" << deco->titleBarHeight();

        KDecoration2::DecorationButton* item = buttonAt(decoPos.x(), decoPos.y());
        if (!item) {
            return false;
        }

        AppMenuButton* appMenuButton = qobject_cast<AppMenuButton *>(item);
        if (appMenuButton) {
            if (m_currentIndex != appMenuButton->buttonIndex()
                && appMenuButton->isVisible()
                && appMenuButton->isEnabled()
            ) {
                emit requestActivateIndex(appMenuButton->buttonIndex());
            }
            return false;
        }
    }

    return false;
}

bool AppMenuButtonGroup::isMenuOpen() const
{
    return 0 <= m_currentIndex;
}

void AppMenuButtonGroup::unPressAllButtons()
{
    // qCDebug(category) << "AppMenuButtonGroup::unPressAllButtons";
    for (int i = 0; i < buttons().length(); i++) {
        KDecoration2::DecorationButton* button = buttons().value(i);

        // Hack to setPressed(false)
        button->setEnabled(!button->isEnabled());
        button->setEnabled(!button->isEnabled());
    }
}

void AppMenuButtonGroup::updateShowing()
{
    setShowing(m_alwaysShow || m_hovered || isMenuOpen());
}

void AppMenuButtonGroup::onMenuAboutToHide()
{
    if (0 <= m_currentIndex && m_currentIndex < buttons().length()) {
        buttons().value(m_currentIndex)->setChecked(false);
    }
    setCurrentIndex(-1);
}

void AppMenuButtonGroup::onShowingChanged(bool showing)
{
    setOpacity(showing ? 1 : 0);
}

} // namespace Material
