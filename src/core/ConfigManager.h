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
    
    // 分组管理
    QList<GroupData> groups() const;
    GroupData group(const QString& id) const;
    void addGroup(const GroupData& group);
    void updateGroup(const GroupData& group);
    void removeGroup(const QString& id);
    bool isGroupEmpty(const QString& groupId) const;
    
    // 按钮分组管理
    QList<ButtonGroup> buttonGroups() const;
    void addButtonGroup(const ButtonGroup& group);
    void updateButtonGroup(const ButtonGroup& group);
    void removeButtonGroup(const QString& id);
    
    // 快捷按钮管理
    QList<QuickButton> quickButtons() const;
    QList<QuickButton> quickButtonsByGroup(const QString& groupId) const;
    void addQuickButton(const QuickButton& button);
    void updateQuickButton(const QuickButton& button);
    void removeQuickButton(const QString& id);
    
    // 全局设置
    GlobalSettings globalSettings() const;
    void setGlobalSettings(const GlobalSettings& settings);
    
signals:
    void sessionAdded(const SessionData& session);
    void sessionUpdated(const SessionData& session);
    void sessionRemoved(const QString& id);
    void groupAdded(const GroupData& group);
    void groupUpdated(const GroupData& group);
    void groupRemoved(const QString& id);
    void buttonGroupsChanged();
    void quickButtonsChanged();
    void globalSettingsChanged();
    
private:
    ConfigManager(QObject* parent = nullptr);
    static ConfigManager* m_instance;
    
    QMap<QString, SessionData> m_sessions;
    QMap<QString, GroupData> m_groups;
    QMap<QString, ButtonGroup> m_buttonGroups;
    QMap<QString, QuickButton> m_quickButtons;
    GlobalSettings m_globalSettings;
};

#endif // CONFIGMANAGER_H
