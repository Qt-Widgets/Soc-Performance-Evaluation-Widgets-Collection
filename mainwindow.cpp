/*
 * @Author: MRXY001
 * @Date: 2019-11-27 18:00:02
 * @LastEditors: MRXY001
 * @LastEditTime: 2019-12-05 14:56:35
 * @Description: 主窗口
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    log("初始化MainWindow");
    ui->setupUi(this);

    initSystem();
    initView();
    initData();
    log("初始化MainWindow结束");
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * 保存绘制的形状到文件，以便下次读取
 * @param file_path 文件路径
 */
void MainWindow::saveToFile(QString file_path)
{
    log("保存至文件：" + file_path);
    QString full_string = ui->scrollAreaWidgetContents_2->toString();
    FileUtil::writeTextFile(file_path, full_string);
}

/**
 * 从XML文件中读取形状实例，并且在绘图区域中绘制
 * @param file_path 文件路径
 */
void MainWindow::readFromFile(QString file_path)
{
    log("从文件读取：" + file_path);
    QString full_string = FileUtil::readTextFileIfExist(file_path);
    if (full_string.trimmed().isEmpty())
        return;

    int widthest = ui->scrollAreaWidgetContents_2->width(),
        heightest = ui->scrollAreaWidgetContents_2->height();

    int graphic_left = StringUtil::getXmlInt(full_string, "GRAPHIC_LEFT");
    int graphic_top = StringUtil::getXmlInt(full_string, "GRAPHIC_TOP");
    int graphic_width = StringUtil::getXmlInt(full_string, "GRAPHIC_WIDTH");
    int graphic_height = StringUtil::getXmlInt(full_string, "GRAPHIC_HEIGHT");
    if (graphic_width != 0 && graphic_height != 0)
        ui->scrollAreaWidgetContents_2->setGeometry(graphic_left, graphic_top, graphic_width, graphic_height);

    QStringList shape_string_list = StringUtil::getXmls(full_string, "SHAPE");
    foreach (QString shape_string, shape_string_list)
    {
        QString name = StringUtil::getXml(shape_string, "CLASS");

        // 创建形状实例
        ShapeBase *type = ui->listWidget->getShapeByName(name);
        if (type == nullptr) // 没有找到这个类，可能后期被删掉了
        {
            log("无法找到形状类：" + name);
            continue;
        }
        ShapeBase *shape = ui->scrollAreaWidgetContents_2->insertShapeByType(type, QPoint(0, 0));
        shape->fromString(shape_string);

        if (StringUtil::getXmlInt(shape_string, "SELECTED") != 0)
            ui->scrollAreaWidgetContents_2->select(shape, true);
    }
    ui->scrollAreaWidgetContents_2->setMinimumSize(widthest, heightest);
}

/**
 * 初始化整个系统
 * 比如初始化目录结构
 */
void MainWindow::initSystem()
{
    FileUtil::ensureDirExist(rt->DATA_PATH);
    FileUtil::ensureDirExist(rt->SHAPE_PATH);
}

/**
 * 初始化布局
 */
void MainWindow::initView()
{
    // 其他的在UI设计师中初始化，不需要在重新设置了

    // 连接绘图区域信号槽
    connect(ui->scrollAreaWidgetContents_2, &GraphicArea::signalScrollToPos, this, [=](int x, int y) {
        if (x != -1)
            ui->scrollArea->horizontalScrollBar()->setSliderPosition(x);

        if (y != -1)
            ui->scrollArea->verticalScrollBar()->setSliderPosition(y);
    });

    connect(ui->scrollAreaWidgetContents_2, &GraphicArea::signalEnsurePosVisible, this, [=](int x, int y) {
        if (x < ui->scrollArea->horizontalScrollBar()->sliderPosition())
            ui->scrollArea->horizontalScrollBar()->setSliderPosition(x);
        else if (x > ui->scrollArea->horizontalScrollBar()->sliderPosition() + ui->scrollArea->width())
            ui->scrollArea->horizontalScrollBar()->setSliderPosition(x - ui->scrollArea->width());

        if (y < ui->scrollArea->verticalScrollBar()->sliderPosition())
            ui->scrollArea->verticalScrollBar()->setSliderPosition(y);
        else if (y > ui->scrollArea->verticalScrollBar()->sliderPosition() + ui->scrollArea->height())
            ui->scrollArea->verticalScrollBar()->setSliderPosition(y - ui->scrollArea->height());
    });

    connect(ui->scrollAreaWidgetContents_2, &GraphicArea::signalScrollAreaScroll, this, [=](int h, int v) {
        if (h)
            ui->scrollArea->horizontalScrollBar()->setSliderPosition(ui->scrollArea->horizontalScrollBar()->sliderPosition() + h);

        if (v)
            ui->scrollArea->verticalScrollBar()->setSliderPosition(ui->scrollArea->verticalScrollBar()->sliderPosition() + v);
    });

    connect(ui->scrollAreaWidgetContents_2, SIGNAL(signalSave()), this, SLOT(on_actionSave_triggered()));
    connect(ui->scrollAreaWidgetContents_2, &GraphicArea::signalAutoSave, this, [=] {
        if (us->auto_save)
            on_actionSave_triggered();
    });
    connect(ui->scrollAreaWidgetContents_2, &GraphicArea::signalTurnBackToPointer, this, [=] {
        if (us->drag_shape_auto_return)
            ui->listWidget->recoverDragPrevIndex(); // 实现绘制完一个模块后调色板上的选项回到先前位置
    });
}

/**
 * 初始化数据
 * （自动读取上次打开的文件）
 */
void MainWindow::initData()
{
    graphic_file_path = rt->DATA_PATH + "graphic.xml";
    readFromFile(graphic_file_path);
}

void MainWindow::on_actionSave_triggered()
{
    saveToFile(graphic_file_path);
}
