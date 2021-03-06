#include "drammodule.h"

DRAMModule::DRAMModule(QWidget *parent) : SlaveModule(parent), token_receive_count(0)
{
    _class = _text = "DRAM";

    QPixmap pixmap(DEFAULT_SIZE, DEFAULT_SIZE);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    drawShapePixmap(painter, QRect(BORDER_SIZE,BORDER_SIZE,DEFAULT_SIZE-BORDER_SIZE*2,DEFAULT_SIZE-BORDER_SIZE*2));
    _pixmap = pixmap;
    big_font = normal_font = bold_font = font();
    big_font.setPointSize(normal_font.pointSize() * 2);
    bold_font.setBold(true);
    big_font.setBold(true);
    normal_font.setBold(true);
}

DRAMModule *DRAMModule::newInstanceBySelf(QWidget *parent)
{
    log("DRAMModule::newInstanceBySelf");
    DRAMModule *shape = new DRAMModule(parent);
    shape->copyDataFrom(this);
    return shape;
}

void DRAMModule::initData()
{
    SlaveModule::initData();
    
    token_receive_count = 0;
    
    foreach (PortBase *p, ShapeBase::ports)
    {
        // 连接信号槽
        ModulePort *port = static_cast<ModulePort *>(p);
        connect(port, &ModulePort::signalOutPortReceived, this, [=](DataPacket *){
            token_receive_count++;
        });
    }
}

void DRAMModule::clearData()
{
    SlaveModule::clearData();

    foreach (PortBase *p, ShapeBase::ports)
    {
        // 连接信号槽
        ModulePort *port = static_cast<ModulePort *>(p);
        disconnect(port, SIGNAL(signalOutPortReceived(DataPacket *)), nullptr, nullptr);
    }
}

void DRAMModule::paintEvent(QPaintEvent *event)
{
    SlaveModule::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFontMetrics fm(this->font());
    int height = fm.lineSpacing();
    painter.setPen(_text_color);
    QFont font = this->font();
    painter.setFont(big_font);

    if(getPorts().size() > 0)
    {

    painter.drawText((width()) / 2 - fm.horizontalAdvance(QString("%1Ghz").arg(getPorts().first()->getOriginalBandwidth())),
                     height * 2, QString("%1Ghz").arg(getPorts().first()->getBandwidth()));
    // painter.drawText(4, 4+fm.lineSpacing(), QString("发送：%1 %2%").arg(token_send_count).arg(prop));
    }
   // painter.drawText(4, 4+fm.lineSpacing(), QString("接收：%1").arg(token_receive_count));
}

void DRAMModule::drawShapePixmap(QPainter &painter, QRect draw_rect)
{
    painter.save();
    QPainterPath path;
    painter.setBrush(QBrush(_pixmap_color));
    path.addRoundedRect(draw_rect, 35, 35);
    //painter.drawRect(draw_rect);
    if (_pixmap_color != Qt::transparent) // 填充内容非透明，画填充
    {
        painter.fillPath(path, _pixmap_color);
    }
    if (_border_size > 0 && _border_color != Qt::transparent) // 画边界
    {
        painter.setPen(QPen(Qt::gray, _border_size));
        painter.drawPath(path);
    }
    painter.restore();
}
