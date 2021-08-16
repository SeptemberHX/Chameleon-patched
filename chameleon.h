/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CHAMELEON_H
#define CHAMELEON_H

#include "chameleontheme.h"
#include "AppMenuButtonGroup.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButtonGroup>

#include <kwineffects.h>

#include <QVariant>
#include <QDir>
#include <QSettings>
#include <QScreen>
#include <QPainterPath>


class Settings;
class ChameleonWindowTheme;

/*
 * 详细见文档 https://api.kde.org/kdecoration/html/classKDecoration2_1_1Decoration.html
 * 理论上只需要阅读下面 override 的方法即可
 */
class Chameleon : public KDecoration2::Decoration
{
    Q_OBJECT

public:
    explicit Chameleon(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Chameleon();

    void paint(QPainter *painter, const QRect &repaintArea) override;

    const ChameleonTheme::Config *themeConfig() const;
    KWin::EffectWindow *effect() const;
    bool noTitleBar() const;

    qreal borderWidth() const;
    qreal titleBarHeight() const;
    qreal shadowRadius() const;
    QPointF shadowOffset() const;
    QPointF windowRadius() const;
    QMarginsF mouseInputAreaMargins() const;

    QColor shadowColor() const;
    QColor borderColor() const;

    QIcon menuIcon() const;
    QIcon minimizeIcon() const;
    QIcon maximizeIcon() const;
    QIcon unmaximizeIcon() const;
    QIcon closeIcon() const;

    // 从 material-decoration 里拿来的
    QRect centerRect() const;
    QRect titleBarRect() const;
    QPoint windowPos() const;
    QColor titleBarBackgroundColor() const;
    QColor titleBarForegroundColor() const;
    int getTextWidth(const QString text, bool showMnemonic = false) const;
    int appMenuButtonHorzPadding() const;

    QColor getTextColor() const;
    QColor getBackgroundColor() const;
signals:
    void noTitleBarChanged(bool noTitleBar);
    void effectInitialized(KWin::EffectWindow *effect);

protected:
    // 这个函数会被自动调用进行初始化
    void init() override;

private:
    void initButtons();
    void updateButtonsGeometry();

    void updateTitleGeometry();

    void updateTheme();
    void updateConfig();
    void updateTitleBarArea();
    void updateBorderPath();
    void updateShadow();
    void updateMouseInputAreaMargins();

    void onClientWidthChanged();
    void onClientHeightChanged();
    void onNoTitlebarPropertyChanged(quint32 windowId);

    void onThemeWindowRadiusChanged();
    void onThemeBorderWidthChanged();
    void onThemeBorderColorChanged();
    void onThemeShadowRadiusChanged();
    void onThemeShadowOffsetChanged();

    bool windowNeedRadius() const;
    bool windowNeedBorder() const;

    bool m_initialized = false;
    qint8 m_noTitleBar = -1;
    QObject *m_client = nullptr;

    QMarginsF m_titleBarAreaMargins;
    QPainterPath m_borderPath;
    ChameleonTheme::ConfigGroupPtr m_configGroup;
    ChameleonTheme::Config *m_config = nullptr;
    ChameleonWindowTheme *m_theme = nullptr;

    QString m_title;
    QRect m_titleArea;

    KDecoration2::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration2::DecorationButtonGroup *m_rightButtons = nullptr;
    AppMenuButtonGroup *m_menuButtons = nullptr;

    QPointer<KWin::EffectWindow> m_effect;
};

#endif // CHAMELEON_H
