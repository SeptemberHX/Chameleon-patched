/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
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
#include "Button.h"

#include "ApplicationMenuButton.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButton>

// KF
#include <KColorUtils>

// Qt
#include <QDebug>
#include <QMargins>
#include <QPainter>
#include <QVariantAnimation>
#include <QtMath> // qFloor


namespace Material
{

Button::Button(KDecoration2::DecorationButtonType type, Chameleon *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_padding(new QMargins())
    , m_isGtkButton(false)
{
    if (QCoreApplication::applicationName() == QStringLiteral("kded5")) {
        // See: https://github.com/Zren/material-decoration/issues/22
        // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.
        m_isGtkButton = true;
    }

    setHeight(decoration->titleBarHeight());

    auto *decoratedClient = decoration->client().toStrongRef().data();

    switch (type) {
    case KDecoration2::DecorationButtonType::ApplicationMenu:
        ApplicationMenuButton::init(this, decoratedClient);
        break;
    default:
        break;
    }
}

Button::~Button()
{
}

KDecoration2::DecorationButton* Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    auto deco = qobject_cast<Chameleon*>(decoration);
    if (!deco) {
        return nullptr;
    }

    switch (type) {
    case KDecoration2::DecorationButtonType::Menu:
    // case KDecoration2::DecorationButtonType::ApplicationMenu:
    case KDecoration2::DecorationButtonType::OnAllDesktops:
    case KDecoration2::DecorationButtonType::ContextHelp:
    case KDecoration2::DecorationButtonType::Shade:
    case KDecoration2::DecorationButtonType::KeepAbove:
    case KDecoration2::DecorationButtonType::KeepBelow:
    case KDecoration2::DecorationButtonType::Close:
    case KDecoration2::DecorationButtonType::Maximize:
    case KDecoration2::DecorationButtonType::Minimize:
        return new Button(type, deco, parent);

    default:
        return nullptr;
    }
}

Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Chameleon*>(), parent)
{
}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(repaintRegion)
    auto deco = qobject_cast<Chameleon*>(decoration());

    // Buttons are coded assuming 24 units in size.
    QRectF buttonRect = geometry();
    buttonRect.setHeight(qMin(30.0 * deco->getPixelRatio(), geometry().height()));
    buttonRect.moveCenter(geometry().center());

    const QRectF contentRect = contentArea();

    const qreal iconScale = contentRect.height()/24;
    int iconSize;
    if (m_isGtkButton) {
        // See: https://github.com/Zren/material-decoration/issues/22
        // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.

        // The svgs are 50x50, located at ~/.config/gtk-3.0/assets/
        // They are usually scaled down to just 18x18 when drawn in gtk headerbars.
        // The Gtk theme already has a fairly large amount of padding, as
        // the Breeze theme doesn't currently follow fitt's law. So use less padding
        // around the icon so that the icon is not a very tiny 8px.

        // 15% top/bottom padding, 70% leftover for the icon.
        // 24 = 3.5 topPadding + 17 icon + 3.5 bottomPadding
        // 17/24 * 18 = 12.75
        iconSize = qRound(iconScale * 17);
    } else {
        // 30% top/bottom padding, 40% leftover for the icon.
        // 24 = 7 topPadding + 10 icon + 7 bottomPadding
        iconSize = qRound(iconScale * 10);
    }
    QRectF iconRect = QRectF(0, 0, iconSize, iconSize);
    iconRect.moveCenter(contentRect.center().toPoint());

    const qreal gridUnit = iconRect.height()/10;

    painter->save();

    painter->setRenderHints(QPainter::HighQualityAntialiasing, true);

    // Opacity
//    painter->setOpacity(m_opacity);

    // Background.
    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor());
    painter->drawRoundedRect(buttonRect, 8, 8);

    // Foreground.
    setPenWidth(painter, gridUnit, 1);
    painter->setBrush(Qt::NoBrush);

    // Icon
    switch (type()) {
    case KDecoration2::DecorationButtonType::ApplicationMenu:
        ApplicationMenuButton::paintIcon(this, painter, iconRect, gridUnit);
        break;
    default:
        paintIcon(painter, iconRect, gridUnit);
        break;
    }
    painter->restore();
}

void Button::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal gridUnit)
{
    Q_UNUSED(painter)
    Q_UNUSED(iconRect)
    Q_UNUSED(gridUnit)
}

void Button::updateSize(int contentWidth, int contentHeight)
{
    const QSize size(
        m_padding->left() + contentWidth + m_padding->right(),
        m_padding->top() + contentHeight + m_padding->bottom()
    );
    setGeometry(QRect(geometry().topLeft().toPoint(), size));
}

void Button::setHeight(int buttonHeight)
{
    // For simplicity, don't count the 1.33:1 scaling in the left/right padding.
    // The left/right padding is mainly for the border offset alignment.
    updateSize(qRound(buttonHeight * 1.33), buttonHeight);
}

qreal Button::iconLineWidth(const qreal gridUnit) const
{
    return 1.0 * qMax(1.0, gridUnit);
}

void Button::setPenWidth(QPainter *painter, const qreal gridUnit, const qreal scale)
{
    QPen pen(foregroundColor());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setWidthF(iconLineWidth(gridUnit) * scale);
    painter->setPen(pen);
}

QColor Button::backgroundColor() const
{
    //--- Normal
    auto *c = qobject_cast<Chameleon*>(decoration().data());

    const QColor hoveredColor("#0081FF");
    QColor normalColor = QColor(hoveredColor);
    normalColor.setAlphaF(0);


    if (isChecked() || isPressed() || isHovered()) {
        return hoveredColor;
    }

    return c->getBackgroundColor();
}

QColor Button::foregroundColor() const
{
    //--- Normal
    auto *c = qobject_cast<Chameleon*>(decoration().data());
    if (isChecked() || isPressed() || isHovered()) {
        return Qt::white;
    }

    return c->getTextColor();
}


QRectF Button::contentArea() const
{
    return geometry().adjusted(
        m_padding->left(),
        m_padding->top(),
        -m_padding->right(),
        -m_padding->bottom()
    );
}

QMargins* Button::padding()
{
    return m_padding;
}


} // namespace Material
