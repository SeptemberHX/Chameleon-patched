/*
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

#pragma once

// own
#include "Button.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// Qt
#include <QPainter>

namespace Material
{

class ApplicationMenuButton
{

public:
    static void init(Button *button, KDecoration2::DecoratedClient *decoratedClient) {
        button->setVisible(decoratedClient->hasApplicationMenu());
    }
    static void paintIcon(Button *button, QPainter *painter, const QRectF &iconRect, const qreal gridUnit) {
        button->setPenWidth(painter, gridUnit, 1.75);

        int spacing = qRound(gridUnit * 4);
        for (int i = -1; i <= 1; ++i) {
            const QPointF left { iconRect.left(), iconRect.center().y() + i * spacing };
            const QPointF right { iconRect.right(), iconRect.center().y() + i * spacing };

            painter->drawLine(left, right);
        }
    }
};

} // namespace Material
