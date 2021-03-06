#ifndef DRAMMODULE_H
#define DRAMMODULE_H

#include "slavemodule.h"

class DRAMModule : public SlaveModule
{
public:
    DRAMModule(QWidget* parent = nullptr);

    virtual DRAMModule *newInstanceBySelf(QWidget* parent = nullptr) override;
    virtual void initData();
    virtual void clearData();

protected:
    void paintEvent(QPaintEvent *event) override;
    virtual void drawShapePixmap(QPainter &painter, QRect draw_rect) override;
    
private:
	int token_receive_count;
    QFont big_font, normal_font, bold_font;
};

#endif // DRAMMODULE_H
