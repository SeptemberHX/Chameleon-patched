/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2019 Zain Ahmad <zain.x.ahmad@gmail.com>
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
#include "MenuOverflowButton.h"
#include "AppMenuButton.h"
#include "ApplicationMenuButton.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// Qt
#include <QDebug>
#include <QPainter>


namespace Material
{

MenuOverflowButton::MenuOverflowButton(Chameleon *decoration, const int buttonIndex, QObject *parent)
    : AppMenuButton(decoration, buttonIndex, parent)
{
    auto *decoratedClient = decoration->client().toStrongRef().data();

    setVisible(decoratedClient->hasApplicationMenu());
}

MenuOverflowButton::~MenuOverflowButton()
{
}

void MenuOverflowButton::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal gridUnit)
{
    ApplicationMenuButton::paintIcon(this, painter, iconRect, gridUnit);
}

} // namespace Material
