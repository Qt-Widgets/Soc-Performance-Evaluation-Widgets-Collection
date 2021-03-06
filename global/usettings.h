/*
 * @Author: MRXY001
 * @Date: 2019-12-02 15:29:29
 * @LastEditors: MRXY001
 * @LastEditTime: 2019-12-05 17:42:37
 * @Description: 文件描述
 */
#ifndef USETTINGS_H
#define USETTINGS_H

#include "settings.h"

class USettings : public Settings
{
    Q_OBJECT
public:
    USettings(QString filePath);

public:
    bool auto_save = getBool("us/auto_save", false);
    bool drag_shape_auto_return = getBool("us/drag_shape_auto_return", true);
    bool show_port_info = getBool("us/show_port_info", true);
    bool port_auto_watch = getBool("us/port_auto_watch", false);
    bool show_animation = getBool("us/show_animation", false);
    
    
};

#endif // USETTINGS_H
