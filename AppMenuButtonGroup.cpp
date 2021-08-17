//
// Created by ragdoll on 2021/8/15.
//

#include "AppMenuButton.h"
#include "AppMenuButtonGroup.h"
#include "debug.h"
#include "TextButton.h"
#include "MenuOverflowButton.h"

#include <QWindow>
#include <QMenu>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>

using Material::TextButton;
using Material::AppMenuButton;
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
    if (this->m_appMenuModel->rowCount() == 0 || decoratedClient->isModal()) {
        resetButtons();
        qDebug() << "Decoration::updateAppMenuModel exits due to isModal or empty menu";
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

        // 修复菜单刷新时的选中高亮消失问题
        if (m_currentIndex == row) {
            b->setChecked(true);
        }

        addButton(QPointer<KDecoration2::DecorationButton>(b));
    }
    m_overflowIndex = m_appMenuModel->rowCount();
    addButton(new MenuOverflowButton(deco, m_overflowIndex, this));

    emit menuUpdated();
    qCDebug(category) << this->windowId() << "AppMenuButtonGroup updateMenu ends";
}

void AppMenuButtonGroup::resetButtons() {
    auto list = QVector<QPointer<KDecoration2::DecorationButton>>(buttons());
    removeButton(KDecoration2::DecorationButtonType::Custom);
    while (!list.isEmpty()) {
        auto item = list.takeFirst();
        item.data()->setVisible(false);
        delete item;
    }
    emit menuUpdated();
}

void AppMenuButtonGroup::trigger(int buttonIndex) {
    if (buttonIndex == m_currentIndex) return;

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
//     if (actionMenu && deco) {
//         auto *decoratedClient = deco->client().toStrongRef().data();
//         actionMenu->setPalette(decoratedClient->palette());
//     }

    if (actionMenu && deco) {
        QRectF buttonRect = button->geometry();
        QPoint position = buttonRect.topLeft().toPoint();
        QPoint rootPosition(position);
        rootPosition += deco->windowPos();
        // qCDebug(category) << "    windowPos" << windowPos;

        actionMenu->adjustSize();
        actionMenu->winId();//create window handle
        actionMenu->installEventFilter(this);
        actionMenu->popup(rootPosition);

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
    qCDebug(category) << "updateOverflow" << availableRect;
    bool showOverflow = false;
    for (KDecoration2::DecorationButton *button : buttons()) {
//        qCDebug(category) << "    " << button->geometry() << button;
        if (qobject_cast<MenuOverflowButton *>(button)) {
            button->setVisible(showOverflow);
//            qCDebug(category) << "    showOverflow" << showOverflow;
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

bool AppMenuButtonGroup::eventFilter(QObject *watched, QEvent *event) {
    auto *menu = qobject_cast<QMenu *>(watched);

    if (!menu) {
        return false;
    }

    if (event->type() == QMouseEvent::KeyPress) {

    }

    if (event->type() == QEvent::KeyPress) {
        auto *e = static_cast<QKeyEvent *>(event);

        // TODO right to left languages
        if (e->key() == Qt::Key_Left) {
            int desiredIndex = m_currentIndex - 1;
            trigger(desiredIndex);
            return true;
        } else if (e->key() == Qt::Key_Right) {
            if (menu->activeAction() && menu->activeAction()->menu()) {
                return false;
            }

            // 修复越界导致的显示异常
            int desiredIndex = m_currentIndex + 1;
            while (desiredIndex < buttons().size() && !buttons()[desiredIndex]->isVisible()) {
                ++desiredIndex;
            }
            if (desiredIndex == buttons().size()) {
                desiredIndex = m_currentIndex;
            }

            trigger(desiredIndex);
            return true;
        }

    } else if (event->type() == QEvent::MouseMove) {
        auto *e = static_cast<QMouseEvent *>(event);

        const auto *deco = qobject_cast<Chameleon *>(decoration());

        QPoint decoPos(e->globalPos());
        decoPos -= deco->windowPos();
        decoPos.ry() += deco->titleBarHeight();
//         qCDebug(category) << "MouseMove";
//         qCDebug(category) << "       globalPos" << e->globalPos();
//         qCDebug(category) << "       windowPos" << deco->windowPos();
//         qCDebug(category) << "  titleBarHeight" << deco->titleBarHeight();

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
                trigger(appMenuButton->buttonIndex());
            }
            return false;
        }
    }

    return false;
}

KDecoration2::DecorationButton *AppMenuButtonGroup::buttonAt(int x, int y) const {
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
