#include "cablebase.h"

CableBase::CableBase(QWidget *parent)
    : ModuleBase(parent),
      from_port(nullptr), to_port(nullptr),
      arrow_pos1(QPoint(-1, -1)), arrow_pos2(QPoint(-1, -1)),
      _line_type(DEFAULT_LINE_TYPE)
{
    _class = "Cable Base";
    setMinimumSize(8, 8);

    // 缩略图
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.drawLine(QPoint(20, 20), QPoint(100, 100));
    _pixmap = pixmap;
}

CableBase::CableBase(PortBase *p1, PortBase *p2, QWidget *parent) : ModuleBase(parent)
{
    setPorts(p1, p2);
}

CableBase *CableBase::newInstanceBySelf(QWidget *parent)
{
    log("CableBase::newInstanceBySelf");
    CableBase *shape = new CableBase(parent);
    shape->copyDataFrom(this);
    return shape;
}

void CableBase::copyDataFrom(ShapeBase *shape)
{
    ShapeBase::copyDataFrom(shape);

    // RTTI运行时类型判断
    // 是不是当前这个类，若是则执行转换
    //    if (typeid (shape) == typeid (this))
    if (shape->getLargeType() == CableType)
    {
        CableBase *cable = static_cast<CableBase *>(shape);

        this->from_port = cable->from_port;
        this->to_port = cable->to_port;
        this->arrow_pos1 = cable->arrow_pos1;
        this->arrow_pos2 = cable->arrow_pos2;
        this->_line_type = cable->_line_type;
    }
}

void CableBase::fromStringAppend(QString s)
{
    QString line_type = StringUtil::getXml(s, "LINE_TYPE");
    if (!line_type.isEmpty())
        _line_type = line_type.toInt();
}

QString CableBase::toStringAppend()
{
    QString string;
    QString indent = "\n\t";
    if (from_port != nullptr)
        string += indent + StringUtil::makeXml(from_port->getPortId(), "FROM_PORT_ID");
    if (to_port != nullptr)
        string += indent + StringUtil::makeXml(to_port->getPortId(), "TO_PORT_ID");
    if (_line_type != DEFAULT_LINE_TYPE)
        string += indent + StringUtil::makeXml(_line_type, "LINE_TYPE");
    return string;
}

LargeShapeType CableBase::getLargeType()
{
    return CableType;
}

void CableBase::setPorts(PortBase *p1, PortBase *p2)
{
    from_port = p1;
    to_port = p2;
    if (p1 != nullptr && p2 != nullptr)
    {
        p1->setOppositePort(to_port);
        p2->setOppositePort(from_port);
        p1->setCable(this);
        p2->setCable(this);
    }
}

bool CableBase::usedPort(PortBase *port)
{
    return from_port == port || to_port == port;
}

bool CableBase::usedPort(QList<PortBase *> ports)
{
    foreach (PortBase *port, ports)
    {
        if (usedPort(port))
            return true;
    }
    return false;
}

PortBase *CableBase::getFromPort()
{
    return from_port;
}

PortBase *CableBase::getToPort()
{
    return to_port;
}

ShapeBase *CableBase::getFromShape()
{
    if (from_port == nullptr)
        return nullptr;
    return static_cast<ShapeBase *>(from_port->getShape());
}

ShapeBase *CableBase::getToShape()
{
    if (to_port == nullptr)
        return nullptr;
    return static_cast<ShapeBase *>(to_port->getShape());
}

void CableBase::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(_border_color, _border_size));
    if (from_port == nullptr) // 两个都没确定，预览
    {
        painter.drawLine(width() / 10, height() / 10, width() * 9 / 10, height() * 9 / 10);
    }
    else if (to_port == nullptr) // 已经确定了一个
    {
        QPoint pos = from_port->getGlobalPos();
        pos = pos - geometry().topLeft(); // 相对连接线左上角
        // 根据这个点在不同的角落，画对角线
        QPoint rev_pos; // 相反的对角位置
        if (pos.x() == 0)
            rev_pos.setX(width());
        else if (pos.x() == width())
            rev_pos.setX(0);
        if (pos.y() == 0)
            rev_pos.setY(height());
        else
            rev_pos.setY(0);
        painter.drawLine(pos, rev_pos);
    }
    else // 两个都确定了
    {
        if (_line_type == 0) // 直线
        {
            painter.drawLine(arrow_pos1, arrow_pos2);
        }
        else if (_line_type == 1) // 横竖
        {
            if (arrow_pos1.x() <= arrow_pos2.x() && arrow_pos1.y() <= arrow_pos2.y())
            {
                // 左上角 - 右下角
                painter.drawLine(0, 0, width(), 0);
                painter.drawLine(width(), 0, width(), height());
            }
            else
            {
                // 右上角 - 左下角
                painter.drawLine(0, 0, width(), 0);
                painter.drawLine(0, 0, 0, height());
            }
        }
        else if (_line_type == 2) // 竖横
        {
            if (arrow_pos1.x() <= arrow_pos2.x() && arrow_pos1.y() <= arrow_pos2.y())
            {
                // 左上角 - 右下角
                painter.drawLine(0, 0, 0, height());
                painter.drawLine(0, height(), width(), height());
            }
            else
            {
                // 右上角 - 左下角
                painter.drawLine(width(), 0, width(), height());
                painter.drawLine(width(), height(), 0, height());
            }
        }
    }
}

void CableBase::drawShapePixmap(QPainter &painter, QRect draw_rect)
{
    Q_UNUSED(painter)
    Q_UNUSED(draw_rect)
}

/**
 * 根据连接的两个端口自动调整自己的大小
 * 注意：本身是没有设置两端的位置的
 * 如果需要使用多条线，需要自己继承并且重写
 */
void CableBase::adjustGeometryByPorts()
{
    if (from_port == nullptr || to_port == nullptr)
    {
        log("没有指定连接的 ports");
        return;
    }

    QPoint cen1 = from_port->getGlobalPos();
    QPoint cen2 = to_port->getGlobalPos();

    int left = qMin(cen1.x(), cen2.x());
    int top = qMin(cen1.y(), cen2.y());
    int right = qMax(cen1.x(), cen2.x());
    int bottom = qMax(cen1.y(), cen2.y());

    // 设置坐标（千万不要用 setFixedSize ！否则无法调整大小）
    setGeometry(left, top, right - left, bottom - top);

    // 计算相对位置，缓存两个 arrow_pos，提升性能
    arrow_pos1 = cen1 - geometry().topLeft();
    arrow_pos2 = cen2 - geometry().topLeft();
}
