#include "ConfigManager.h"
#include "CryptoHelper.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager* ConfigManager::instance() {
    if (!m_instance) {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent) {
}

QString ConfigManager::configFilePath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("config.json");
}

bool ConfigManager::load() {
    QFile file(configFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // 加载分组
    m_groups.clear();
    QJsonArray groupsArray = root["groups"].toArray();
    for (const auto& val : groupsArray) {
        GroupData group = GroupData::fromJson(val.toObject());
        m_groups[group.id] = group;
    }
    
    // 加载会话
    m_sessions.clear();
    QJsonArray sessionsArray = root["sessions"].toArray();
    for (const auto& val : sessionsArray) {
        SessionData session = SessionData::fromJson(val.toObject());
        // 解密敏感信息
        session.sshConfig.password = CryptoHelper::decrypt(session.sshConfig.password);
        session.sshConfig.passphrase = CryptoHelper::decrypt(session.sshConfig.passphrase);
        m_sessions[session.id] = session;
    }
    
    // 加载按钮分组
    m_buttonGroups.clear();
    QJsonArray btnGroupsArray = root["buttonGroups"].toArray();
    for (const auto& val : btnGroupsArray) {
        ButtonGroup group = ButtonGroup::fromJson(val.toObject());
        m_buttonGroups[group.id] = group;
    }
    
    // 加载快捷按钮
    m_quickButtons.clear();
    QJsonArray buttonsArray = root["quickButtons"].toArray();
    for (const auto& val : buttonsArray) {
        QuickButton btn = QuickButton::fromJson(val.toObject());
        m_quickButtons[btn.id] = btn;
    }
    
    // 加载全局设置
    m_globalSettings = GlobalSettings::fromJson(root["globalSettings"].toObject());
    
    return true;
}

bool ConfigManager::save() {
    QJsonObject root;
    
    // 保存分组
    QJsonArray groupsArray;
    for (const auto& group : m_groups) {
        groupsArray.append(group.toJson());
    }
    root["groups"] = groupsArray;
    
    // 保存会话（加密敏感信息）
    QJsonArray sessionsArray;
    for (auto session : m_sessions) {
        session.sshConfig.password = CryptoHelper::encrypt(session.sshConfig.password);
        session.sshConfig.passphrase = CryptoHelper::encrypt(session.sshConfig.passphrase);
        sessionsArray.append(session.toJson());
    }
    root["sessions"] = sessionsArray;
    
    // 保存按钮分组
    QJsonArray btnGroupsArray;
    for (const auto& group : m_buttonGroups) {
        btnGroupsArray.append(group.toJson());
    }
    root["buttonGroups"] = btnGroupsArray;
    
    // 保存快捷按钮
    QJsonArray buttonsArray;
    for (const auto& btn : m_quickButtons) {
        buttonsArray.append(btn.toJson());
    }
    root["quickButtons"] = buttonsArray;
    
    // 保存全局设置
    root["globalSettings"] = m_globalSettings.toJson();
    
    QFile file(configFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

QList<SessionData> ConfigManager::sessions() const {
    return m_sessions.values();
}

SessionData ConfigManager::session(const QString& id) const {
    return m_sessions.value(id);
}

void ConfigManager::addSession(const SessionData& session) {
    m_sessions[session.id] = session;
    emit sessionAdded(session);
    save();
}

void ConfigManager::updateSession(const SessionData& session) {
    m_sessions[session.id] = session;
    emit sessionUpdated(session);
    save();
}

void ConfigManager::removeSession(const QString& id) {
    m_sessions.remove(id);
    emit sessionRemoved(id);
    save();
}

QList<SessionData> ConfigManager::sessionsByGroup(const QString& groupId) const {
    QList<SessionData> result;
    for (const auto& session : m_sessions) {
        if (session.groupId == groupId) {
            result.append(session);
        }
    }
    return result;
}

QList<GroupData> ConfigManager::groups() const {
    return m_groups.values();
}

GroupData ConfigManager::group(const QString& id) const {
    return m_groups.value(id);
}

void ConfigManager::addGroup(const GroupData& group) {
    m_groups[group.id] = group;
    emit groupAdded(group);
    save();
}

void ConfigManager::updateGroup(const GroupData& group) {
    m_groups[group.id] = group;
    emit groupUpdated(group);
    save();
}

void ConfigManager::removeGroup(const QString& id) {
    m_groups.remove(id);
    emit groupRemoved(id);
    save();
}

bool ConfigManager::isGroupEmpty(const QString& groupId) const {
    for (const auto& session : m_sessions) {
        if (session.groupId == groupId) {
            return false;
        }
    }
    return true;
}

QList<ButtonGroup> ConfigManager::buttonGroups() const {
    return m_buttonGroups.values();
}

void ConfigManager::addButtonGroup(const ButtonGroup& group) {
    m_buttonGroups[group.id] = group;
    emit buttonGroupsChanged();
    save();
}

void ConfigManager::updateButtonGroup(const ButtonGroup& group) {
    m_buttonGroups[group.id] = group;
    emit buttonGroupsChanged();
    save();
}

void ConfigManager::removeButtonGroup(const QString& id) {
    m_buttonGroups.remove(id);
    // 同时删除该分组下的所有按钮
    QStringList toRemove;
    for (const auto& btn : m_quickButtons) {
        if (btn.groupId == id) {
            toRemove.append(btn.id);
        }
    }
    for (const auto& btnId : toRemove) {
        m_quickButtons.remove(btnId);
    }
    emit buttonGroupsChanged();
    emit quickButtonsChanged();
    save();
}

QList<QuickButton> ConfigManager::quickButtons() const {
    return m_quickButtons.values();
}

QList<QuickButton> ConfigManager::quickButtonsByGroup(const QString& groupId) const {
    QList<QuickButton> result;
    for (const auto& btn : m_quickButtons) {
        if (btn.groupId == groupId) {
            result.append(btn);
        }
    }
    return result;
}

void ConfigManager::addQuickButton(const QuickButton& button) {
    m_quickButtons[button.id] = button;
    emit quickButtonsChanged();
    save();
}

void ConfigManager::updateQuickButton(const QuickButton& button) {
    m_quickButtons[button.id] = button;
    emit quickButtonsChanged();
    save();
}

void ConfigManager::removeQuickButton(const QString& id) {
    m_quickButtons.remove(id);
    emit quickButtonsChanged();
    save();
}

GlobalSettings ConfigManager::globalSettings() const {
    return m_globalSettings;
}

void ConfigManager::setGlobalSettings(const GlobalSettings& settings) {
    m_globalSettings = settings;
    emit globalSettingsChanged();
    save();
}
