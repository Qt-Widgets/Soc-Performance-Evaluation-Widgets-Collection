#include "circleshape.h"

CircleShape::CircleShape(QWidget *parent) : ShapeBase(parent)
{
    _class = _text = "Circle Class";

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    drawShapePixmap(painter, QRect(2,2,124,124));
    _pixmap = pixmap;
}

CircleShape *CircleShape::newInstanceBySelf(QWidget *parent)
{
    log("CircleShape::newInstanceBySelf");
    CircleShape* shape = new CircleShape(parent);
    shape->copyDataFrom(this);
    return shape;
}

void CircleShape::drawShapePixmap(QPainter &painter, QRect draw_rect)
{
    QPainterPath path;
    path.addEllipse(draw_rect);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (_pixmap_color != Qt::transparent) // 填充内容非透明，画填充
    {
        painter.fillPath(path, _pixmap_color);
    }
    if (_border_size>0 && _border_color!=Qt::transparent) // 画边界
    {
        painter.setPen(QPen(Qt::gray, 3));
        painter.drawPath(path);
    }
}
