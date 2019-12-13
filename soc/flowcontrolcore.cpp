/*
 * @Author: MRXY001
 * @Date: 2019-12-11 16:47:58
 * @LastEditors: MRXY001
 * @LastEditTime: 2019-12-13 16:31:51
 * @Description: 流控的核心数据部分
 */
#include "flowcontrolcore.h"

FlowControlCore::FlowControlCore(QObject *parent) : QObject(parent), master(nullptr), slave(nullptr), current_clock(-1)
{
}

/**
 * 初始化所有的数据
 */
void FlowControlCore::initData()
{
    current_clock = 0;

    // 初始化属性
    master->setToken(master->getDataValue("token", 16).toInt());
    master->setBandwidth(master->getDataValue("bandwidth", 1).toInt());
    master->setLatency(master->getDataValue("latency", 0).toInt());

    slave->setToken(slave->getDataValue("token", 16).toInt());
    slave->setBandwidth(slave->getDataValue("bandwidth", 1).toInt());
    slave->setLatency(slave->getDataValue("latency", 6).toInt());

    ms_cable->setTransferDelay(ms_cable->getDataValue("delay", 5).toInt());

    // 设置运行数据
    master->setSlaveFree(slave->getToken());

    // 初始化数据包
    for (int i = 0; i < master->getToken(); i++)
    {
        DataPacket *packet = new DataPacket(QString("编号%1").arg(i+1), this);
        master->data_list.append(packet);
        all_packets.append(packet);
    }
}

void FlowControlCore::clearData()
{
    if (!rt->running || current_clock == -1)
        return ;
    master->data_list.clear();
    slave->enqueue_list.clear();
    slave->data_queue.clear();
    slave->dequeue_list.clear();
    slave->process_list.clear();
    ms_cable->request_list.clear();
    ms_cable->request_data_list.clear();
    ms_cable->response_list.clear();
    ms_cable->response_data_list.clear();
    foreach (DataPacket* packet, all_packets)
    {
        packet->deleteLater();
    }
    all_packets.clear();
}

/**
 * 模拟时钟流逝 1 个 clock
 */
void FlowControlCore::passOneClock()
{
    FCDEB "Clock:" << current_clock << " >>";

    // ==== 发送数据 ====
    // Slave有空位时，Master发送数据（0 clock）
    if (/* master->isSlaveFree() &&  */master->data_list.size())
    {
        DataPacket *packet = master->data_list.takeFirst();
        packet->resetDelay(ms_cable->getTransferDelay());
        ms_cable->request_list.append(packet);
        master->slave_free--;
    }

    // 连接线延迟传输（5 clock）-->给Slave
    for (int i = 0; i < ms_cable->request_list.size(); i++)
    {
        DataPacket *packet = ms_cable->request_list.at(i);
        if (packet->isDelayFinished()) // 传输结束，Slave收到Master的数据
        {
            ms_cable->request_list.removeAt(i--);
            slave->enqueue_list.append(packet);
            packet->resetDelay(slave->getEnqueueDelay());
        }
        else // 仍然在传输中
        {
            packet->delayToNext();
        }
    }

    // Slave进队列（1 clock）
    for (int i = 0; i < slave->enqueue_list.size(); i++)
    {
        DataPacket *packet = slave->enqueue_list.at(i);
        if (packet->isDelayFinished())
        {
            slave->enqueue_list.removeAt(i--);
            slave->data_queue.enqueue(packet);
        }
        else
        {
            packet->delayToNext();
        }
    }

    // Slave队列中（0 clock）-->出队列（不耗时，只出1个）
    if (slave->data_queue.size())
    {
        DataPacket *packet = slave->data_queue.dequeue();
        slave->dequeue_list.append(packet);
        packet->resetDelay(slave->getDequeueDelay());
    }

    // Slave出队列（1 clock）-->处理数据
    for (int i = 0; i < slave->dequeue_list.size(); i++)
    {
        DataPacket *packet = slave->dequeue_list.at(i);
        if (packet->isDelayFinished())
        {
            slave->dequeue_list.removeAt(i--);
            slave->process_list.append(packet);
            packet->resetDelay(slave->getProcessDelay());
            // Slave空出一个位置
            master->slave_free++;
        }
        else
        {
            packet->delayToNext();
        }
    }

    // Slave处理数据（3 clock）-->返回给Master
    for (int i = 0; i < slave->process_list.size(); i++)
    {
        DataPacket *packet = slave->process_list.at(i);
        if (packet->isDelayFinished())
        {
            slave->process_list.removeAt(i--);
            ms_cable->response_list.append(packet);
            packet->resetDelay(ms_cable->getTransferDelay());
        }
        else
        {
            packet->delayToNext();
        }
    }

    // Slave返回给Master（5 clock）-->Master接收
    for (int i = 0; i < ms_cable->response_list.size(); i++)
    {
        DataPacket *packet = ms_cable->response_list.at(i);
        if (packet->isDelayFinished())
        {
            ms_cable->response_list.removeAt(i--);
            master->data_list.append(packet);
            packet->resetDelay(0);
            qDebug() << "Master接收到response" << packet->toString();
        }
        else
        {
            packet->delayToNext();
        }
    }

    // ==== 时钟结束后首尾 ====
    current_clock++;

    // 内部模拟时钟流逝，设置数据包位置等
    master->passOneClock();
    slave->passOneClock();
    ms_cable->passOneClock();
}

void FlowControlCore::printfAllData()
{
	qDebug() << "=================================";
    for (int i = 0; i < all_packets.size(); i++)
    {
        qDebug() << all_packets.at(i)->toString();
    }
	qDebug() << "=================================";
}