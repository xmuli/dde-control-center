#ifndef POWERACTION_H
#define POWERACTION_H

#include "optionitem.h"
#include "contentwidget.h"
#include <QWidget>

using namespace dcc;
using namespace dcc::widgets;

class PowerAction : public ContentWidget
{
    Q_OBJECT
public:
    explicit PowerAction(QWidget *parent = 0);

signals:
    void requestSelected(const QVariant &value);

public slots:
    void setDefault(const QVariant &value);
    void itemSelected();

private:
    QList<OptionItem*> m_list;
};

#endif // POWERACTION_H
