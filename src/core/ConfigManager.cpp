#include "ConfigManager.h"
#include "CryptoHelper.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>

ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager* ConfigManager::instance() {
    if (!m_instance) {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent) {
    load();
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

    // 保存分组（按排序顺序）
    QList<GroupData> sortedGroups = groups();
    QJsonArray groupsArray;
    for (const auto& group : sortedGroups) {
        groupsArray.append(group.toJson());
    }
    root["groups"] = groupsArray;

    // 保存会话（加密敏感信息，按排序顺序）
    QList<SessionData> sortedSessions = sessions();
    QJsonArray sessionsArray;
    for (auto session : sortedSessions) {
        session.sshConfig.password = CryptoHelper::encrypt(session.sshConfig.password);
        session.sshConfig.passphrase = CryptoHelper::encrypt(session.sshConfig.passphrase);
        sessionsArray.append(session.toJson());
    }
    root["sessions"] = sessionsArray;

    // 保存按钮分组（按排序顺序）
    QList<ButtonGroup> sortedBtnGroups = buttonGroups();
    QJsonArray btnGroupsArray;
    for (const auto& group : sortedBtnGroups) {
        btnGroupsArray.append(group.toJson());
    }
    root["buttonGroups"] = btnGroupsArray;

    // 保存快捷按钮（按排序顺序）
    QList<QuickButton> sortedButtons = quickButtons();
    QJsonArray buttonsArray;
    for (const auto& btn : sortedButtons) {
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

// ==================== 会话管理 ====================

QList<SessionData> ConfigManager::sessions() const {
    QList<SessionData> result = m_sessions.values();
    std::sort(result.begin(), result.end());
    return result;
}

SessionData ConfigManager::session(const QString& id) const {
    return m_sessions.value(id);
}

void ConfigManager::addSession(const SessionData& session) {
    SessionData newSession = session;
    if (newSession.sortOrder == 0) {
        newSession.sortOrder = nextSessionSortOrder(newSession.groupId);
    }
    m_sessions[newSession.id] = newSession;
    emit sessionTreeUpdated();
    save();
}

void ConfigManager::updateSession(const SessionData& session) {
    m_sessions[session.id] = session;
    emit sessionTreeUpdated();
    save();
}

void ConfigManager::removeSession(const QString& id) {
    QString groupId = m_sessions.value(id).groupId;
    m_sessions.remove(id);
    normalizeSessionSortOrders(groupId);
    emit sessionTreeUpdated();
    save();
}

QList<SessionData> ConfigManager::sessionsByGroup(const QString& groupId) const {
    QList<SessionData> result;
    for (const auto& session : m_sessions) {
        if (session.groupId == groupId) {
            result.append(session);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

int ConfigManager::nextSessionSortOrder(const QString& groupId) const {
    int maxOrder = -1;
    for (const auto& session : m_sessions) {
        if (session.groupId == groupId && session.sortOrder > maxOrder) {
            maxOrder = session.sortOrder;
        }
    }
    return maxOrder + 1;
}

void ConfigManager::normalizeSessionSortOrders(const QString& groupId) {
    QList<SessionData> groupSessions = sessionsByGroup(groupId);
    for (int i = 0; i < groupSessions.size(); ++i) {
        if (m_sessions.contains(groupSessions[i].id)) {
            m_sessions[groupSessions[i].id].sortOrder = i;
        }
    }
}

void ConfigManager::moveSessionUp(const QString& id) {
    if (!m_sessions.contains(id)) return;

    SessionData& session = m_sessions[id];
    QList<SessionData> groupSessions = sessionsByGroup(session.groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupSessions.size(); ++i) {
        if (groupSessions[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex > 0) {
        // 交换排序号
        QString prevId = groupSessions[currentIndex - 1].id;
        int tempOrder = m_sessions[id].sortOrder;
        m_sessions[id].sortOrder = m_sessions[prevId].sortOrder;
        m_sessions[prevId].sortOrder = tempOrder;

        emit sessionTreeUpdated();
        save();
    }
}

void ConfigManager::moveSessionDown(const QString& id) {
    if (!m_sessions.contains(id)) return;

    SessionData& session = m_sessions[id];
    QList<SessionData> groupSessions = sessionsByGroup(session.groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupSessions.size(); ++i) {
        if (groupSessions[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex >= 0 && currentIndex < groupSessions.size() - 1) {
        // 交换排序号
        QString nextId = groupSessions[currentIndex + 1].id;
        int tempOrder = m_sessions[id].sortOrder;
        m_sessions[id].sortOrder = m_sessions[nextId].sortOrder;
        m_sessions[nextId].sortOrder = tempOrder;

        emit sessionTreeUpdated();
        save();
    }
}

void ConfigManager::moveSessionToIndex(const QString& id, int newIndex) {
    if (!m_sessions.contains(id)) return;

    QString groupId = m_sessions[id].groupId;
    QList<SessionData> groupSessions = sessionsByGroup(groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupSessions.size(); ++i) {
        if (groupSessions[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex < 0 || currentIndex == newIndex) return;
    if (newIndex < 0 || newIndex >= groupSessions.size()) return;

    // 移动元素
    SessionData movedSession = groupSessions.takeAt(currentIndex);
    groupSessions.insert(newIndex, movedSession);

    // 更新所有排序号
    for (int i = 0; i < groupSessions.size(); ++i) {
        m_sessions[groupSessions[i].id].sortOrder = i;
    }

    emit sessionTreeUpdated();
    save();
}

void ConfigManager::reorderSessions(const QStringList& sessionIds) {
    for (int i = 0; i < sessionIds.size(); ++i) {
        if (m_sessions.contains(sessionIds[i])) {
            m_sessions[sessionIds[i]].sortOrder = i;
        }
    }
    emit sessionTreeUpdated();
    save();
}

// ==================== 分组管理 ====================

QList<GroupData> ConfigManager::groups() const {
    QList<GroupData> result = m_groups.values();
    std::sort(result.begin(), result.end());
    return result;
}

GroupData ConfigManager::group(const QString& id) const {
    return m_groups.value(id);
}

void ConfigManager::addGroup(const GroupData& group) {
    GroupData newGroup = group;
    if (newGroup.sortOrder == 0 && !m_groups.isEmpty()) {
        newGroup.sortOrder = nextGroupSortOrder();
    }
    m_groups[newGroup.id] = newGroup;
    emit sessionTreeUpdated();
    save();
}

void ConfigManager::updateGroup(const GroupData& group) {
    m_groups[group.id] = group;
    emit sessionTreeUpdated();
    save();
}

void ConfigManager::removeGroup(const QString& id) {
    m_groups.remove(id);
    normalizeGroupSortOrders();
    emit sessionTreeUpdated();
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

int ConfigManager::nextGroupSortOrder() const {
    int maxOrder = -1;
    for (const auto& group : m_groups) {
        if (group.sortOrder > maxOrder) {
            maxOrder = group.sortOrder;
        }
    }
    return maxOrder + 1;
}

void ConfigManager::normalizeGroupSortOrders() {
    QList<GroupData> sortedGroups = groups();
    for (int i = 0; i < sortedGroups.size(); ++i) {
        if (m_groups.contains(sortedGroups[i].id)) {
            m_groups[sortedGroups[i].id].sortOrder = i;
        }
    }
}

void ConfigManager::moveGroupUp(const QString& id) {
    if (!m_groups.contains(id)) return;

    QList<GroupData> sortedGroups = groups();

    int currentIndex = -1;
    for (int i = 0; i < sortedGroups.size(); ++i) {
        if (sortedGroups[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex > 0) {
        QString prevId = sortedGroups[currentIndex - 1].id;
        int tempOrder = m_groups[id].sortOrder;
        m_groups[id].sortOrder = m_groups[prevId].sortOrder;
        m_groups[prevId].sortOrder = tempOrder;

        emit sessionTreeUpdated();
        save();
    }
}

void ConfigManager::moveGroupDown(const QString& id) {
    if (!m_groups.contains(id)) return;

    QList<GroupData> sortedGroups = groups();

    int currentIndex = -1;
    for (int i = 0; i < sortedGroups.size(); ++i) {
        if (sortedGroups[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex >= 0 && currentIndex < sortedGroups.size() - 1) {
        QString nextId = sortedGroups[currentIndex + 1].id;
        int tempOrder = m_groups[id].sortOrder;
        m_groups[id].sortOrder = m_groups[nextId].sortOrder;
        m_groups[nextId].sortOrder = tempOrder;

        emit sessionTreeUpdated();
        save();
    }
}

void ConfigManager::moveGroupToIndex(const QString& id, int newIndex) {
    if (!m_groups.contains(id)) return;

    QList<GroupData> sortedGroups = groups();

    int currentIndex = -1;
    for (int i = 0; i < sortedGroups.size(); ++i) {
        if (sortedGroups[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex < 0 || currentIndex == newIndex) return;
    if (newIndex < 0 || newIndex >= sortedGroups.size()) return;

    GroupData movedGroup = sortedGroups.takeAt(currentIndex);
    sortedGroups.insert(newIndex, movedGroup);

    for (int i = 0; i < sortedGroups.size(); ++i) {
        m_groups[sortedGroups[i].id].sortOrder = i;
    }

    emit sessionTreeUpdated();
    save();
}

void ConfigManager::reorderGroups(const QStringList& groupIds) {
    for (int i = 0; i < groupIds.size(); ++i) {
        if (m_groups.contains(groupIds[i])) {
            m_groups[groupIds[i]].sortOrder = i;
        }
    }
    emit sessionTreeUpdated();
    save();
}

// ==================== 按钮分组管理 ====================

QList<ButtonGroup> ConfigManager::buttonGroups() const {
    QList<ButtonGroup> result = m_buttonGroups.values();
    std::sort(result.begin(), result.end());
    return result;
}

ButtonGroup ConfigManager::buttonGroup(const QString& id) const {
    return m_buttonGroups.value(id);
}

void ConfigManager::addButtonGroup(const ButtonGroup& group) {
    ButtonGroup newGroup = group;
    if (newGroup.sortOrder == 0 && !m_buttonGroups.isEmpty()) {
        newGroup.sortOrder = nextButtonGroupSortOrder();
    }
    m_buttonGroups[newGroup.id] = newGroup;
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
    normalizeButtonGroupSortOrders();
    emit buttonGroupsChanged();
    emit quickButtonsChanged();
    save();
}

int ConfigManager::nextButtonGroupSortOrder() const {
    int maxOrder = -1;
    for (const auto& group : m_buttonGroups) {
        if (group.sortOrder > maxOrder) {
            maxOrder = group.sortOrder;
        }
    }
    return maxOrder + 1;
}

void ConfigManager::normalizeButtonGroupSortOrders() {
    QList<ButtonGroup> sorted = buttonGroups();
    for (int i = 0; i < sorted.size(); ++i) {
        if (m_buttonGroups.contains(sorted[i].id)) {
            m_buttonGroups[sorted[i].id].sortOrder = i;
        }
    }
}

void ConfigManager::moveButtonGroupUp(const QString& id) {
    if (!m_buttonGroups.contains(id)) return;

    QList<ButtonGroup> sorted = buttonGroups();

    int currentIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex > 0) {
        QString prevId = sorted[currentIndex - 1].id;
        int tempOrder = m_buttonGroups[id].sortOrder;
        m_buttonGroups[id].sortOrder = m_buttonGroups[prevId].sortOrder;
        m_buttonGroups[prevId].sortOrder = tempOrder;

        emit buttonGroupsChanged();
        save();
    }
}

void ConfigManager::moveButtonGroupDown(const QString& id) {
    if (!m_buttonGroups.contains(id)) return;

    QList<ButtonGroup> sorted = buttonGroups();

    int currentIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex >= 0 && currentIndex < sorted.size() - 1) {
        QString nextId = sorted[currentIndex + 1].id;
        int tempOrder = m_buttonGroups[id].sortOrder;
        m_buttonGroups[id].sortOrder = m_buttonGroups[nextId].sortOrder;
        m_buttonGroups[nextId].sortOrder = tempOrder;

        emit buttonGroupsChanged();
        save();
    }
}

void ConfigManager::moveButtonGroupToIndex(const QString& id, int newIndex) {
    if (!m_buttonGroups.contains(id)) return;

    QList<ButtonGroup> sorted = buttonGroups();

    int currentIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex < 0 || currentIndex == newIndex) return;
    if (newIndex < 0 || newIndex >= sorted.size()) return;

    ButtonGroup moved = sorted.takeAt(currentIndex);
    sorted.insert(newIndex, moved);

    for (int i = 0; i < sorted.size(); ++i) {
        m_buttonGroups[sorted[i].id].sortOrder = i;
    }

    emit buttonGroupsChanged();
    save();
}

void ConfigManager::reorderButtonGroups(const QStringList& groupIds) {
    for (int i = 0; i < groupIds.size(); ++i) {
        if (m_buttonGroups.contains(groupIds[i])) {
            m_buttonGroups[groupIds[i]].sortOrder = i;
        }
    }
    emit buttonGroupsChanged();
    save();
}

// ==================== 快捷按钮管理 ====================

QList<QuickButton> ConfigManager::quickButtons() const {
    QList<QuickButton> result = m_quickButtons.values();
    std::sort(result.begin(), result.end());
    return result;
}

QList<QuickButton> ConfigManager::quickButtonsByGroup(const QString& groupId) const {
    QList<QuickButton> result;
    for (const auto& btn : m_quickButtons) {
        if (btn.groupId == groupId) {
            result.append(btn);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

QuickButton ConfigManager::quickButton(const QString& id) const {
    return m_quickButtons.value(id);
}

void ConfigManager::addQuickButton(const QuickButton& button) {
    QuickButton newButton = button;
    if (newButton.sortOrder == 0) {
        newButton.sortOrder = nextQuickButtonSortOrder(newButton.groupId);
    }
    m_quickButtons[newButton.id] = newButton;
    emit quickButtonsChanged();
    save();
}

void ConfigManager::updateQuickButton(const QuickButton& button) {
    m_quickButtons[button.id] = button;
    emit quickButtonsChanged();
    save();
}

void ConfigManager::removeQuickButton(const QString& id) {
    QString groupId = m_quickButtons.value(id).groupId;
    m_quickButtons.remove(id);
    normalizeQuickButtonSortOrders(groupId);
    emit quickButtonsChanged();
    save();
}

int ConfigManager::nextQuickButtonSortOrder(const QString& groupId) const {
    int maxOrder = -1;
    for (const auto& btn : m_quickButtons) {
        if (btn.groupId == groupId && btn.sortOrder > maxOrder) {
            maxOrder = btn.sortOrder;
        }
    }
    return maxOrder + 1;
}

void ConfigManager::normalizeQuickButtonSortOrders(const QString& groupId) {
    QList<QuickButton> groupButtons = quickButtonsByGroup(groupId);
    for (int i = 0; i < groupButtons.size(); ++i) {
        if (m_quickButtons.contains(groupButtons[i].id)) {
            m_quickButtons[groupButtons[i].id].sortOrder = i;
        }
    }
}

void ConfigManager::moveQuickButtonUp(const QString& id) {
    if (!m_quickButtons.contains(id)) return;

    QString groupId = m_quickButtons[id].groupId;
    QList<QuickButton> groupButtons = quickButtonsByGroup(groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupButtons.size(); ++i) {
        if (groupButtons[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex > 0) {
        QString prevId = groupButtons[currentIndex - 1].id;
        int tempOrder = m_quickButtons[id].sortOrder;
        m_quickButtons[id].sortOrder = m_quickButtons[prevId].sortOrder;
        m_quickButtons[prevId].sortOrder = tempOrder;

        emit quickButtonsChanged();
        save();
    }
}

void ConfigManager::moveQuickButtonDown(const QString& id) {
    if (!m_quickButtons.contains(id)) return;

    QString groupId = m_quickButtons[id].groupId;
    QList<QuickButton> groupButtons = quickButtonsByGroup(groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupButtons.size(); ++i) {
        if (groupButtons[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex >= 0 && currentIndex < groupButtons.size() - 1) {
        QString nextId = groupButtons[currentIndex + 1].id;
        int tempOrder = m_quickButtons[id].sortOrder;
        m_quickButtons[id].sortOrder = m_quickButtons[nextId].sortOrder;
        m_quickButtons[nextId].sortOrder = tempOrder;

        emit quickButtonsChanged();
        save();
    }
}

void ConfigManager::moveQuickButtonToIndex(const QString& id, int newIndex) {
    if (!m_quickButtons.contains(id)) return;

    QString groupId = m_quickButtons[id].groupId;
    QList<QuickButton> groupButtons = quickButtonsByGroup(groupId);

    int currentIndex = -1;
    for (int i = 0; i < groupButtons.size(); ++i) {
        if (groupButtons[i].id == id) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex < 0 || currentIndex == newIndex) return;
    if (newIndex < 0 || newIndex >= groupButtons.size()) return;

    QuickButton moved = groupButtons.takeAt(currentIndex);
    groupButtons.insert(newIndex, moved);

    for (int i = 0; i < groupButtons.size(); ++i) {
        m_quickButtons[groupButtons[i].id].sortOrder = i;
    }

    emit quickButtonsChanged();
    save();
}

void ConfigManager::reorderQuickButtons(const QStringList& buttonIds) {
    for (int i = 0; i < buttonIds.size(); ++i) {
        if (m_quickButtons.contains(buttonIds[i])) {
            m_quickButtons[buttonIds[i]].sortOrder = i;
        }
    }
    emit quickButtonsChanged();
    save();
}

// ==================== 全局设置 ====================

GlobalSettings ConfigManager::globalSettings() const {
    return m_globalSettings;
}

void ConfigManager::setGlobalSettings(const GlobalSettings& settings) {
    m_globalSettings = settings;
    emit globalSettingsChanged();
    save();
}
