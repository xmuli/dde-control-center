#include "poweraction.h"
#include "settingsgroup.h"
#include "translucentframe.h"
#include <QVBoxLayout>

using namespace dcc;
using namespace dcc::widgets;

PowerAction::PowerAction(QWidget *parent) : ContentWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    SettingsGroup *group = new SettingsGroup;

    OptionItem *shutdown = new OptionItem;
    shutdown->setTitle("关机");
    shutdown->setValue("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Shutdown");
    OptionItem *reboot = new OptionItem;
    reboot->setTitle("重启");
    reboot->setValue("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Reboot");
    OptionItem *lock = new OptionItem;
    lock->setTitle("锁定");
    lock->setValue("dbus-send --print-reply --dest=com.deepin.dde.lockFront /com/deepin/dde/lockFront com.deepin.dde.lockFront.Show");
    OptionItem *sleep = new OptionItem;
    sleep->setTitle("睡眠");
    sleep->setValue("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Suspend");
    OptionItem *logout = new OptionItem;
    logout->setTitle("注销");
    logout->setValue("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Logout");
    OptionItem *inquiry = new OptionItem;
    inquiry->setTitle("询问");
    inquiry->setValue("dde-shutdown");

    m_list << shutdown << reboot << lock << sleep << logout << inquiry;

    for (OptionItem *item : m_list) {
        item->setContentsMargins(20, 0, 10, 0);
        group->appendItem(item);
    }

    layout->addSpacing(10);
    layout->addWidget(group);
    TranslucentFrame *w = new TranslucentFrame;
    w->setLayout(layout);
    setContent(w);

    connect(shutdown, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
    connect(reboot, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
    connect(lock, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
    connect(sleep, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
    connect(logout, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
    connect(inquiry, &OptionItem::selectedChanged, this, &PowerAction::itemSelected);
}

void PowerAction::setDefault(const QVariant &value)
{
    for (OptionItem *item : m_list) {
        item->blockSignals(true);
        item->setSelected(item->value() == value);
        item->blockSignals(false);
    }
}

void PowerAction::itemSelected()
{
    OptionItem *item = qobject_cast<OptionItem*>(sender());

    if (item)
        emit requestSelected(item->value());
}
