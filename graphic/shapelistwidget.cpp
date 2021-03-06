/*
 * @Author: MRXY001
 * @Date: 2019-11-29 15:53:37
 * @LastEditors  : MRXY001
 * @LastEditTime : 2019-12-19 09:29:40
 * @Description: 左边的形状单元列表框
 */
#include "shapelistwidget.h"

ShapeListWidget::ShapeListWidget(QWidget *parent)
    : QListWidget(parent),
      _has_draged(false), _drag_prev_index(-1)
{
    setViewMode(QListView::IconMode); // 设置图标模式

    connect(this, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slotItemChanged(QListWidgetItem *, QListWidgetItem *)));

    loadShapes();
    setCurrentRow(us->getInt("recent/choosed_shape_unit"));
}

/**
 * 通过形状名字来获取一个形状
 * @param name 名字（请先确保不重复）
 * @return     形状模板。如果未找到，返回 nullptr
 */
ShapeBase *ShapeListWidget::getShapeByName(QString name)
{
    foreach (ShapeBase *shape, shape_units)
    {
        if (shape->getClass() == name)
        {
            return shape;
        }
    }
    return nullptr;
}

void ShapeListWidget::recoverDragPrevIndex()
{
    if (_drag_prev_index > -1 && _drag_prev_index < count())
    {
        setCurrentRow(_drag_prev_index);
        _drag_prev_index = -1;
    }
}

/**
 * 鼠标按下，记录按下位置，用来结合 mouseMoveEvent 判断拖拽事件
 */
void ShapeListWidget::mousePressEvent(QMouseEvent *event)
{
    _drag_prev_index = currentRow(); // 保存拖拽前的位置

    QListWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        _drag_start_pos = event->pos();
        _has_draged = false;
    }
}

/**
 * 鼠标移动
 * 拖拽形状至右边绘图区域来添加图形
 * 也可能没有图形，而是鼠标形状的选择工具
 */
void ShapeListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (_has_draged || !(event->buttons() & Qt::LeftButton))
        return QListWidget::mouseMoveEvent(event);

    // 判断有没有选中控件
    if (rt->current_choosed_shape == nullptr)
        return;

    // 计算拖拽距离
    int distance = (event->pos() - _drag_start_pos).manhattanLength();
    if (distance < QApplication::startDragDistance()) // 距离太小，不算移动
        return QListWidget::mouseMoveEvent(event);

    log("开始形状列表拖拽事件" + QString::number(reinterpret_cast<qint64>(rt->current_choosed_shape)));
    QMimeData *mime_data = new QMimeData;
    // 将指针设置成QByteArray
    mime_data->setData(CHOOSED_SHAPE_MIME_TYPE, ByteArrayUtil::int64ToByte(reinterpret_cast<qint64>(rt->current_choosed_shape)));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mime_data);
    drag->setPixmap(rt->current_choosed_shape->getPixmap());
    drag->setDragCursor(QPixmap(":/icons/locate"), Qt::DropAction::MoveAction);
    drag->exec(Qt::MoveAction);
    _has_draged = true; // 避免移动时触发重复拖拽或判断

    return QListWidget::mouseMoveEvent(event);
}

/**
 * 加载所有的电路元件
 */
void ShapeListWidget::loadShapes()
{
    // 加载默认的鼠标
    new QListWidgetItem(QIcon(":/icons/cursor"), MOVING_CURSOR_NAME, this);

    // 加载自定义形状
    loadCustomShape(new MasterModule(this));
    loadCustomShape(new SlaveModule(this));
    loadCustomShape(new SwitchModule(this));
    loadCustomShape(new ModuleCable(this));
    // loadCustomShape(new CableBase(this));
    loadCustomShape(new IPModule(this));
    loadCustomShape(new DRAMModule(this));

    loadCustomShape(new ModulePanel(this));
    loadCustomShape(new WatchModule(this));
    /*loadCustomShape(new CircleShape(this));
    loadCustomShape(new EllipseShape(this));
    loadCustomShape(new SquareShape(this));
    loadCustomShape(new RectangleShape(this));
    loadCustomShape(new HexagonShape(this));*/

    // 遍历文件目录，逐个加载
    QDir dir(rt->SHAPE_PATH);
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString info, list)
    {
        loadOneShape(info);
    }
}

/**
 * 加载自定义的形状
 * @param shape
 */
void ShapeListWidget::loadCustomShape(ShapeBase *shape)
{
    new QListWidgetItem(QIcon(shape->getPixmap()), shape->getClass(), this);
    shape->hide();

    shape_units.append(shape);
}

/**
 * 从一个目录中读取自定义的形状
 * 包括缩略图、大小、端口、自定义数据等
 */
void ShapeListWidget::loadOneShape(const QString name)
{
    log("读取形状：" + name);
    QString path = rt->SHAPE_PATH + name + "/";
    new QListWidgetItem(QIcon(path + "thumb.png"), name, this);

    ShapeBase *shape = new ShapeBase(name, QPixmap(path + "thumb.png"), this);
    shape->hide(); // 隐藏起来
    shape_units.append(shape);
}

/**
 * 左边选择的形状改变
 */
void ShapeListWidget::slotItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    // 列表项的名字
    QString name = current->text();
    us->setVal("recent/choosed_shape_unit", currentRow()); // 保存选中的形状单元
    log("选中项改变" + name);

    // 如果选中的是指针
    if (name == MOVING_CURSOR_NAME)
    {
        rt->current_choosed_shape = nullptr;
    }
    // 如果选中的是形状
    else
    {
        foreach (ShapeBase *shape, shape_units)
        {
            if (shape->getClass() == name)
            {
                rt->current_choosed_shape = shape;
                log("设置形状：" + name);
                break;
            }
        }
    }

    if (name.contains("Cable", Qt::CaseInsensitive) || name.contains("Line", Qt::CaseInsensitive))
    {
        rt->auto_stick_ports = true;
    }
    else
    {
        rt->auto_stick_ports = false;
    }
}
