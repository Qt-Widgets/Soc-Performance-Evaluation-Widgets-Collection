#ifndef FLOWCONTROLAUTOMATIC_H
#define FLOWCONTROLAUTOMATIC_H

#include "flowcontrolbase.h"
#include "switchmodule.h"
#include "ipmodule.h"
#include "drammodule.h"

class FlowControlAutomatic : public FlowControlBase
{
public:
    FlowControlAutomatic(GraphicArea *ga, QObject *parent = nullptr);

protected:
    bool initModules() override;
    void initData() override;
    void clearData() override;
    void initOneClock() override;
    void passOneClock() override;
    void uninitOneClock() override;
    void refreshUI() override;

private:
    QList<ModuleBase *> getModules();

private:
    ShapeList shapes;
};

#endif // FLOWCONTROLAUTOMATIC_H
