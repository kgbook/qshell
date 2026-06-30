#ifndef QSHELL_SESSIONTABWIDGET_H
#define QSHELL_SESSIONTABWIDGET_H

#include <QTabWidget>
#include <QProxyStyle>

#ifdef Q_OS_MACOS
class LeftAlignedTabStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;
    
    int styleHint(StyleHint hint, const QStyleOption *option,
                  const QWidget *widget, QStyleHintReturn *returnData) const override {
        if (hint == QStyle::SH_TabBar_Alignment) {
            return Qt::AlignLeft;
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
    
    QRect subElementRect(SubElement element, const QStyleOption *option,
                         const QWidget *widget) const override {
        if (element == SE_TabWidgetTabBar) {
            QRect rect = QProxyStyle::subElementRect(element, option, widget);
            // 强制 tab bar 左对齐（x=0）并占满整个宽度
            rect.moveLeft(0);
            if (widget) {
                rect.setWidth(widget->width());
            }
            return rect;
        }
        return QProxyStyle::subElementRect(element, option, widget);
    }
};
#endif

class SessionTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit SessionTabWidget(QWidget *parent = nullptr);
    ~SessionTabWidget() override = default;

private slots:
    void showTabContextMenu(const QPoint &pos);
    void renameTab();

private:
    int m_contextMenuTabIndex = -1;
};

#endif // QSHELL_SESSIONTABWIDGET_H
