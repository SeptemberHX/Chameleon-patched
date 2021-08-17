//
// Created by ragdoll on 2021/8/15.
//

#ifndef CHAMELEON_PATCHED_APPMENUBUTTONGROUP_H
#define CHAMELEON_PATCHED_APPMENUBUTTONGROUP_H

#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/Decoration>
#include <QProxyStyle>

#include "appmenu/appmenumodel.h"
#include "debug.h"

using KDecoration2::DecorationButtonGroup;
using KDecoration2::Decoration;


class MyProxyStyle : public QProxyStyle {
public:
    void setPixelRatio(qreal pixelRatio) {
        MyProxyStyle::pixelRatio = pixelRatio;
    }

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override {
        qCDebug(category) << metric;
        if (metric == QStyle::PM_ButtonIconSize) {
            return QProxyStyle::pixelMetric(metric, option, widget) * pixelRatio;
        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }

private:
    qreal pixelRatio;
};

/*
 * 根据 KDE 官方文档 https://api.kde.org/kdecoration/html/classKDecoration2_1_1DecorationButtonGroup.html，
 * DecorationButtonGroup 帮助我们屏蔽了 buttons 的布局等通用问题，我们只需要往里面添加我们想要的 button 即可。
 *
 * 显然，根据 Decoration 的文档，每个窗口是由 DecorationClient 负责一些只读属性的获取，Decoration 与 DecorationClient 一对一绑定。
 * 也就意味着每个窗口都会创建一个 Decoration 实例。在这里也就是 Chameleon 实例。而 AppMenuButtonGroup 中涉及到窗口的菜单获取，且在每个
 * Chameleon 中都有一个实例，在该类中我们需要创建一个实例，来监听每个窗口的菜单的变化情况并获取菜单。
 */
class AppMenuButtonGroup : public DecorationButtonGroup {

    Q_OBJECT

public:
    explicit AppMenuButtonGroup(Decoration *decoration);

    // 这个方法负责绘制。但是根据官方文档，默认实现会调用每个 DecorationButton 的绘制，
    // 所以我们只需要正确的插入 button 以及实现 button 绘制即可
    // void paint(QPainter *painter, const QRect &repaintArea) override;

    int currentIndex() const;
    void setCurrentIndex(int set);
    bool overflowing() const;
    void setOverflowing(bool set);
    bool isMenuOpen() const;

signals:
    void menuUpdated();
    void overflowingChanged();
    void currentIndexChanged();

public slots:
    void updateOverflow(QRectF availableRect);
    void trigger(int index);

    // 负责在菜单变化后，获取菜单内容
    void updateMenu();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onMenuAboutToHide();

private:
    // 以下是一些辅助函数

    // 当前 AppMenuButtonGroup 对应的窗口 id
    WId windowId() const;

    // 清除当前所有的 button 并释放内存
    void resetButtons();

    KDecoration2::DecorationButton* buttonAt(int x, int y) const;

    // 负责监听给定窗口的菜单变化以及获取菜单
    AppMenuModel *m_appMenuModel;

    int m_currentIndex;
    int m_overflowIndex;
    bool m_overflowing;
    bool m_hovered;
    bool m_showing;
    bool m_alwaysShow;
    QPointer<QMenu> m_currentMenu;

    MyProxyStyle *m_proxyStyle;
};


#endif //CHAMELEON_PATCHED_APPMENUBUTTONGROUP_H
