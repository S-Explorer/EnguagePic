#ifndef STYLE_H
#define STYLE_H

#include <QString>

// 应用程序样式配置
class AppStyle {
public:
    // 主色调
    static constexpr const char* PRIMARY_COLOR = "#009688";      // 青绿色
    static constexpr const char* PRIMARY_HOVER = "#00BFA5";      // 亮青绿
    static constexpr const char* PRIMARY_DARK = "#00796B";       // 深青绿
    
    // 辅助色
    static constexpr const char* ACCENT_COLOR = "#FF5722";       // 橙红色（强调）
    static constexpr const char* ACCENT_HOVER = "#FF7043";       // 亮橙红
    
    // 背景色
    static constexpr const char* BG_LIGHT = "#FAFAFA";           // 浅灰背景
    static constexpr const char* BG_CARD = "#FFFFFF";            // 卡片白
    static constexpr const char* BG_DARK = "#263238";            // 深色背景
    
    // 文字色
    static constexpr const char* TEXT_PRIMARY = "#212121";       // 主文字
    static constexpr const char* TEXT_SECONDARY = "#757575";     // 次要文字
    static constexpr const char* TEXT_WHITE = "#FFFFFF";         // 白色文字
    
    // 边框色
    static constexpr const char* BORDER_COLOR = "#E0E0E0";       // 边框灰
    
    // 状态色
    static constexpr const char* SUCCESS_COLOR = "#4CAF50";      // 成功绿
    static constexpr const char* WARNING_COLOR = "#FFC107";      // 警告黄
    static constexpr const char* ERROR_COLOR = "#F44336";        // 错误红
    
    // 获取应用程序全局样式表
    static QString getGlobalStyle();
    
    // 获取按钮样式
    static QString getPrimaryButtonStyle();
    static QString getSecondaryButtonStyle();
    static QString getDangerButtonStyle();
    
    // 获取输入框样式
    static QString getLineEditStyle();
    
    // 获取表格样式
    static QString getTableViewStyle();
    
    // 获取下拉框样式
    static QString getComboBoxStyle();
};

#endif // STYLE_H
