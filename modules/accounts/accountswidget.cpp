#include "accountswidget.h"

#include "nextpagewidget.h"
#include "settingsgroup.h"
#include "useroptionitem.h"
#include "switchwidget.h"
#include "settingsheaderitem.h"

#include <QPushButton>
#include <QDebug>

using namespace dcc::widgets;
using namespace dcc::accounts;

AccountsWidget::AccountsWidget()
    : ModuleWidget(),

    m_userGroup(new dcc::widgets::SettingsGroup),
    m_createBtn(new QPushButton(tr("Create Account")))
{
    setObjectName("Accounts");

    m_centralLayout->addWidget(m_userGroup);
    m_centralLayout->addWidget(m_createBtn);

    setTitle(tr("Accounts"));

    connect(m_createBtn, &QPushButton::clicked, this, &AccountsWidget::requestCreateAccount);
}

void AccountsWidget::addUser(User *user)
{
    UserOptionItem *w = new UserOptionItem;

    m_userGroup->appendItem(w);

    auto setFullName = [w] (const QString &fn)
    {
        if (!fn.isEmpty())
            w->setTitle(fn);
        else
            w->setTitle("本地用户");
    };

    connect(user, &User::fullnameChanged, this, setFullName);
    connect(user, &User::currentAvatarChanged, w, &UserOptionItem::setAvatar);
    connect(w, &NextPageWidget::clicked, [=] { emit showAccountsDetail(user); });

    setFullName(user->fullname());
    w->setAvatar(user->currentAvatar());

    m_maps[user] = w;
}

void AccountsWidget::removeUser(User *user)
{
    m_userGroup->removeItem(m_maps[user]);
    m_maps[user]->deleteLater();
    m_maps.remove(user);
//    QList<NextPageWidget *> items = findChildren<NextPageWidget*>();
//    for (NextPageWidget *item : items) {
//        if (item->title() == user->name()) {
//            m_userGroup->removeItem(item);
//            item->deleteLater();
//            break;
//        }
//    }
}
