/*
 * @Author: MRXY001
 * @Date: 2019-12-09 14:09:05
 * @LastEditors: MRXY001
 * @LastEditTime: 2019-12-09 14:34:23
 * @Description: SlaveModule
 */
#include "slavemodule.h"

SlaveModule::SlaveModule(QWidget *parent) : EllipseShape(parent), buffer_size(0)
{
    _class = _text = "Slave";
}

void SlaveModule::setBufferSize(int size)
{
    buffer_size = size;
}

void SlaveModule::sendPacket(DataPacket *packet)
{
}

void SlaveModule::receivedPacket(DataPacket *packet)
{
}
