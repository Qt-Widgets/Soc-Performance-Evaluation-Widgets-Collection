/*
 * @Author: MRXY001
 * @Date: 2019-12-19 09:08:59
 * @LastEditors  : MRXY001
 * @LastEditTime : 2019-12-23 14:24:53
 * @Description: Switch
 */
#include "switchmodule.h"

SwitchModule::SwitchModule(QWidget *parent) : HexagonShape(parent)
{
    _class = _text = "Switch";
}

SwitchModule *SwitchModule::newInstanceBySelf(QWidget *parent)
{
    log("SwitchModule::newInstanceBySelf");
    SwitchModule *shape = new SwitchModule(parent);
    shape->copyDataFrom(this);
    return shape;
}

PortBase *SwitchModule::createPort()
{
    return new ModulePort(this);
}

void SwitchModule::initData()
{
    this->token = getData("token")->i();

    foreach (PortBase *p, ShapeBase::ports)
    {
        if (p->getCable() == nullptr) // 没有连接线
            continue;

        // 连接信号槽
        ModulePort *port = static_cast<ModulePort *>(p);
        port->setReceiveCache(false);
        connect(port, SIGNAL(signalDataReceived(ModulePort *, DataPacket *)), this, SLOT(slotDataReceived(ModulePort *, DataPacket *)));

        // ==== 发送部分（Master） ====
        connect(port, &ModulePort::signalSendDelayFinished, this, [=](ModulePort *port, DataPacket *packet) {
            qDebug() << "connect(port, &ModulePort::signalSendDelayFinished";
            ModuleCable *cable = static_cast<ModuleCable *>(port->getCable());
            if (cable == nullptr)
                return;
            rt->runningOut("port发送，对方能接收" + QString::number(port->another_can_receive));
            packet->setTargetPort(cable->getToPort());
            cable->request_list.append(packet);
            packet->resetDelay(cable->getTransferDelay());
        });
    }
}

void SwitchModule::clearData()
{
    request_queue.clear();
    response_queue.clear();
}

int SwitchModule::getToken()
{
    return token;
}

void SwitchModule::passOneClock(PASS_ONE_CLOCK_FLAG flag)
{
    // request queue
    if (flag == PASS_REQUEST)
    {
        int picker_bandwidth = getData("picker_bandwidth")->i();
        for (int i = 0; i < request_queue.size(); ++i)
        {
            DataPacket *packet = request_queue.at(i);
            if (packet->isDelayFinished())
            {
                // 判断packet的传输目标
                if (packet->getTargetPort() != nullptr)
                {
                    // 使用 Picker 进行轮询
                    if (picker_bandwidth-- > 0)
                    {
                        ModulePort* port = static_cast<ModulePort*>(packet->getTargetPort());
                        request_queue.removeAt(i--);
                        ModuleCable* cable = static_cast<ModuleCable*>(port->getCable());
                        if (cable == nullptr)
                            continue;
                        packet->resetDelay(cable->getData("delay")->i());
                        port->sendData(packet, DATA_REQUEST);
                        rt->runningOut("Hub 发送：" + packet->toString());

                        // 通过进来的端口，返回发送出去的token（依赖port的return delay）
                        if (packet->getComePort() != nullptr)
                        {
                            ModulePort* port = static_cast<ModulePort*>(packet->getComePort());
                            port->sendDequeueTokenToComeModule(new DataPacket());
                        }
                    }
                }
            }
            else
            {
                packet->delayToNext();
            }
        }
    }

    // response queue
    if (flag == PASS_RESPONSE)
    {
        for (int i = 0; i < response_queue.size(); ++i)
        {
            DataPacket *packet = response_queue.at(i);
            if (packet->isDelayFinished())
            {
                // 判断packet的传输目标
                if (packet->getTargetPort() != nullptr)
                {
                    ModulePort* port = static_cast<ModulePort*>(packet->getTargetPort());
                    response_queue.removeAt(i--);
                    ModuleCable* cable = static_cast<ModuleCable*>(port->getCable());
                    if (cable == nullptr)
                        continue;
                    packet->resetDelay(cable->getData("delay")->i());
                    port->sendData(packet, DATA_RESPONSE);

                    // 通过进来的端口，返回发送出去的token（依赖port的return delay）
                    if (packet->getComePort() != nullptr)
                    {
                        ModulePort* port = static_cast<ModulePort*>(packet->getComePort());
                        port->sendDequeueTokenToComeModule(new DataPacket());
                    }
                }
            }
            else
            {
                packet->delayToNext();
            }
        }

        foreach (PortBase* p, ports)
        {
            ModulePort* port = static_cast<ModulePort*>(p);
            port->passOneClock(PASS_RECEIVE);
        }
    }

    updatePacketPos();
}

/**
 * 收到数据
 * @param port   收到数据的端口
 * @param packet 收到的数据
 */
void SwitchModule::slotDataReceived(ModulePort *port, DataPacket *packet)
{
    ShapeBase* oppo = static_cast<ShapeBase*>(port->getOppositeShape());
    if (oppo != nullptr)
    {
        if (oppo->getText().indexOf("master",0,Qt::CaseInsensitive) != -1) // 收到来自Master的request
        {
            request_queue.enqueue(packet);
            rt->runningOut("Hub 收到 request : " + QString::number(request_queue.size()));
            packet->setComePort(port);
            packet->setTargetPort(getToPort(port));
            packet->resetDelay(getData("latency")->i());
        }
        else if (oppo->getText().indexOf("slave",0,Qt::CaseInsensitive) != -1) // 收到来自Slave的response
        {
            response_queue.enqueue(packet);
            rt->runningOut("Hub 收到 response : " + QString::number(response_queue.size()));
        }
        else // 不知道是啥
            return ;

        // 通过端口确定来去的方向
        packet->setComePort(port);
        packet->setTargetPort(getToPort(port));
        packet->resetDelay(getData("latency")->i());
    }
}

void SwitchModule::updatePacketPos()
{
    QFontMetrics fm(this->font());
    int height = fm.lineSpacing();
    int h = height * 3 + 4;
    int l = 4;
    qDebug() << request_queue.size();
    foreach (DataPacket *packet, request_queue)
    {
        packet->setDrawPos(pos() + QPoint(l, h));
        l += 4 + PACKET_SIZE;
    }

    h += height + 4;
    l = 4;
    foreach (DataPacket *packet, response_queue)
    {
        packet->setDrawPos(pos() + QPoint(l, h));
        l += 4 + PACKET_SIZE;
    }
}

void SwitchModule::paintEvent(QPaintEvent *event)
{
    HexagonShape::paintEvent(event);

    // 画自己的数量
    QPainter painter(this);
    QFontMetrics fm(this->font());
}

PortBase *SwitchModule::getToPort(PortBase *from_port)
{
    ShapeBase* from_shape = static_cast<ShapeBase*>(from_port->getOppositeShape());
    if (from_shape == nullptr)
        return nullptr;
    QString from_name = from_shape->getText();
    QString to_name;
    if (from_name.startsWith("master", Qt::CaseInsensitive))
        to_name = "Slave";
    else
        to_name = "Master";
    if (from_name.endsWith("1"))
        to_name += "1";
    else
        to_name += "2";

    foreach (PortBase* p, ports)
    {
        if (p->getOppositeShape() != nullptr && static_cast<ShapeBase*>(p->getOppositeShape())->getText() == to_name)
            return p;
    }
    ERR("没找到适合的 to port: "+to_name)

    return nullptr;
}
