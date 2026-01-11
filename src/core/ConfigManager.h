#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QMap>
#include "SessionData.h"

class ConfigManager : public QObject {
    Q_OBJECT
    
public:
    static ConfigManager* instance();
    
    // 加载/保存配置
    bool load();
    bool save();

    static QString configFilePath();
    
    // 会话管理
    QList<SessionData> sessions() const;
    SessionData session(const QString& id) const;
    void addSession(const SessionData& session);
    void updateSession(const SessionData& session);
    void removeSession(const QString& id);
    QList<SessionData> sessionsByGroup(const QString& groupId) const;
    
    // 会话排序
    void moveSessionUp(const QString& id);
    void moveSessionDown(const QString& id);
    void moveSessionToIndex(const QString& id, int newIndex);
    void reorderSessions(const QStringList& sessionIds);  // 按给定ID顺序重新排序

    // 分组管理
    QList<GroupData> groups() const;
    GroupData group(const QString& id) const;
    void addGroup(const GroupData& group);
    void updateGroup(const GroupData& group);
    void removeGroup(const QString& id);
    bool isGroupEmpty(const QString& groupId) const;

    // 分组排序
    void moveGroupUp(const QString& id);
    void moveGroupDown(const QString& id);
    void moveGroupToIndex(const QString& id, int newIndex);
    void reorderGroups(const QStringList& groupIds);  // 按给定ID顺序重新排序

    // 按钮分组管理
    QList<ButtonGroup> buttonGroups() const;
    ButtonGroup buttonGroup(const QString& id) const;
    void addButtonGroup(const ButtonGroup& group);
    void updateButtonGroup(const ButtonGroup& group);
    void removeButtonGroup(const QString& id);

    // 按钮分组排序
    void moveButtonGroupUp(const QString& id);
    void moveButtonGroupDown(const QString& id);
    void moveButtonGroupToIndex(const QString& id, int newIndex);
    void reorderButtonGroups(const QStringList& groupIds);

    // 快捷按钮管理
    QList<QuickButton> quickButtons() const;
    QList<QuickButton> quickButtonsByGroup(const QString& groupId) const;
    QuickButton quickButton(const QString& id) const;
    void addQuickButton(const QuickButton& button);
    void updateQuickButton(const QuickButton& button);
    void removeQuickButton(const QString& id);

    // 快捷按钮排序
    void moveQuickButtonUp(const QString& id);
    void moveQuickButtonDown(const QString& id);
    void moveQuickButtonToIndex(const QString& id, int newIndex);
    void reorderQuickButtons(const QStringList& buttonIds);

    // 全局设置
    GlobalSettings globalSettings() const;
    void setGlobalSettings(const GlobalSettings& settings);

signals:
    void sessionTreeUpdated();
    void buttonGroupsChanged();
    void quickButtonsChanged();
    void globalSettingsChanged();

private:
    ConfigManager(QObject* parent = nullptr);
    static ConfigManager* m_instance;

    // 辅助函数：获取下一个可用的排序号
    int nextSessionSortOrder(const QString& groupId) const;
    int nextGroupSortOrder() const;
    int nextButtonGroupSortOrder() const;
    int nextQuickButtonSortOrder(const QString& groupId) const;

    // 辅助函数：规范化排序号（消除间隙）
    void normalizeSessionSortOrders(const QString& groupId);
    void normalizeGroupSortOrders();
    void normalizeButtonGroupSortOrders();
    void normalizeQuickButtonSortOrders(const QString& groupId);
    
    QMap<QString, SessionData> m_sessions;
    QMap<QString, GroupData> m_groups;
    QMap<QString, ButtonGroup> m_buttonGroups;
    QMap<QString, QuickButton> m_quickButtons;
    GlobalSettings m_globalSettings;
};

#endif // CONFIGMANAGER_H
