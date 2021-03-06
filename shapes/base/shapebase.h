/*
 * @Author      : MRXY001
 * @Date        : 2019-11-28 11: 23: 54
 * @LastEditors : MRXY001
 * @LastEditTime: 2019-12-11 10:03:53
 * @Description : 所有形状的基类，包含所有通用API
 */
#ifndef SHAPEBASE_H
#define SHAPEBASE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QDebug>
#include <QImage>
#include "globalvars.h"
#include "selectedge.h"
#include "portpositiondialog.h"
#include "customdatatype.h"

#define DEFAULT_SIZE 128
#define BORDER_SIZE 2 // 边缘padding，用来放调整大小的边界线
#define ERR(x) qDebug() << log(x);

const Qt::Alignment DEFAULT_TEXT_ALIGN = Qt::AlignBottom | Qt::AlignHCenter;
const QColor DEFAULT_TEXT_COLOR = Qt::black;
const bool DEFAULT_PIXMAP_SCALE = false;
const QColor DEFAULT_PIXMAP_COLOR = Qt::transparent;
const int DEFAULT_BORDER_SIZE = 3;
const QColor DEFAULT_BORDER_COLOR = Qt::gray;

enum LargeShapeType
{
    ShapeType,
    CableType
};

class ShapeBase;
typedef QList<ShapeBase *> ShapeList;

class ShapeBase : public QWidget
{
    Q_OBJECT
public:
    // 构造
    ShapeBase(QWidget *parent = nullptr);
    ShapeBase(QString text, QWidget *parent = nullptr);
    ShapeBase(QString text, QPixmap pixmap, QWidget *parent = nullptr);
    ~ShapeBase() override;

    friend class MainWindow;
    friend class GraphicArea;
    friend class ShapePropertyDialog;
    friend class ShapeDataDialog;
    friend class ModuleDataDialog;

    virtual ShapeBase *newInstanceBySelf(QWidget *parent = nullptr); // 根据形状类型创建对应的形状实例
    virtual void copyDataFrom(ShapeBase *shape);                     // 从形状实例中拷贝数据
    virtual QRect getSuitableRect(QPoint point);                     // 从列表拖到绘图区域时，自适应大小和坐标

    // 属性
    virtual LargeShapeType getLargeType();
    virtual const QString getClass();
    virtual const QString getText();
    virtual const QPixmap getPixmap();
    virtual void setText(QString text);
    bool hasColor(QPoint pos); // 某一个点是否有颜色（没有颜色则点击穿透）

    // 操作
    void showEdge();
    void hideEdge();
    bool isEdgeShowed();
    void setLightEdgeShowed(bool show);

    virtual PortBase *createPort();
    virtual void addPort(PortBase *port);
    QList<PortBase *> getPorts();
    PortBase *getPortByOpposite(ShapeBase *shape);

    void setPressOperatorEffected();
    void simulatePress(QMouseEvent *event);

    // 保存
    void fromString(QString s);
    virtual void fromStringAppend(QString s);
    QString toString();
    virtual QString toStringAppend();
    QString readedText();

    // 数据
    void setRoutingID(int id);
    int getRoutingID();
    CustomDataType *getData(QString name);
    bool containsData(QString name);
    QVariant getDataValue(QString name, QVariant def = 0);
    DataType getDataType(QString name);
    virtual CustomDataList ensureDataList();
    virtual void setDefaultDataList();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    virtual void drawShapePixmap(QPainter &painter, QRect draw_rect);
    virtual QPainterPath getShapePainterPath();                  // 获取绘图区域（基类）
    virtual void initDrawArea();                                 // 设置绘制区域大小
    virtual void resizeDrawArea(QSize old_size, QSize new_size); // 调整控件大小时，调整绘制区域的大小
    virtual QList<QAction *> addinMenuActions();                 // 添加shape对应的右键菜单

protected:
    void adjustPortsPosition();

signals:
    void signalResized(QSize size);
    void signalRaised();
    void signalTransparentForMousePressEvents(QMouseEvent *event);
    void signalClicked();
    void signalClickReleased();
    void signalCtrlClicked();
    void signalCtrlClickReleased();
    void signalDoubleClicked();
    void signalMoved(int dx, int dy);
    void signalLeftButtonReleased();
    void signalMenuShowed();

    void signalPortPositionModified(PortBase *port);
    void signalPortInserted(PortBase *port);
    void signalPortDeleted(PortBase *port);
    void signalPortWatch(PortBase *port);
    void signalPortToken(PortBase *port);

public slots:

protected:
    // 形状属性
    QString _class;            // 形状类名（默认设为前景文字）
    QString _text;             // 前景文字（也当做ID，必须唯一）
    int routing_id;            // 用户自定义路由ID
    Qt::Alignment _text_align; // 文字对齐
    QColor _text_color;        // 文字颜色
    QPixmap _pixmap;           // 前景图标
    QString _pixmap_name;      // 图标路径（图片资源中的图标文件名）
    bool _pixmap_scale;        // 是否拉伸图标
    QColor _pixmap_color;      // 图标颜色
    int _border_size;          // 边界粗细
    QColor _border_color;      // 边界颜色

    // 用户数据
    CustomDataList custom_data_list; // 用户自定义数据

    // 相关区域与控件
    QRect _area;             // 有效的显示区域（非控件大小）
    SelectEdge *edge;        // 四周边界线
    QList<PortBase *> ports; // 连接线的端口

    // 操作相关
    QPoint _press_pos_global; // 鼠标左键按下时鼠标的全局坐标
    QPoint _press_topLeft;    // 鼠标左键按下时控件的左上角坐标（用来移动）
    bool _pressing;           // 是否正在单击/拖拽本形状
    bool _press_moved;        // 这次单击是否移动了
    bool _press_effected;     // 按下时特殊操作是否已经生效
    bool _hovering;           // 是否鼠标悬浮期间
    bool _show_light_edge;    // 是否显示淡淡的边界
    qint64 _prev_rls_timest;  // 上次鼠标松开的时间戳（用来判断双击）
    qint64 _curr_rls_timest;  // 本次鼠标松开的时间戳（用来传递给上次）

    // 存储
    QString _readed_text; // 读取时的内容（即fromString的文本）
private:
    QFont big_font, normal_font, bold_font;
};

#endif // SHAPEBASE_H
