/*
 * @Author: MRXY001
 * @Date: 2019-12-09 14:01:52
 * @LastEditors: MRXY001
 * @LastEditTime: 2019-12-09 18:25:20
 * @Description: 模块接口，包含发送等功能
 */
#include "moduleinterface.h"

ModuleInterface::ModuleInterface(QObject *parent) : QObject(parent)
{

}

void ModuleInterface::sendPacket(DataPacket *packet)
{

}

void ModuleInterface::sentPacket(QList<DataPacket *> packet_list)
{

}

void ModuleInterface::receivedPacket(DataPacket *packet)
{

}

void ModuleInterface::passOneClock()
{

}