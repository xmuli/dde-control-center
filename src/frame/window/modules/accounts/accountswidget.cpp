/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liuhong <liuhong_cm@deepin.com>
 *
 * Maintainer: liuhong <liuhong_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "accountswidget.h"

#include "modules/accounts/usermodel.h"
#include "modules/accounts/user.h"
#include "accountsdetailwidget.h"

#include <DStyleOption>
#include <DStandardItem>

#include <QWidget>
#include <QVBoxLayout>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <QDebug>
#include <QIcon>
#include <QSize>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>

DWIDGET_USE_NAMESPACE
using namespace dcc::accounts;
using namespace DCC_NAMESPACE::accounts;

//用户列表图标半径
const int UserImageRadius = 16;

AccountsWidget::AccountsWidget(QWidget *parent)
    : QWidget(parent)
    , m_createBtn(new DFloatingButton(DStyle::SP_IncreaseElement, this))
    , m_userlistView(new DListView)
    , m_userItemModel(new QStandardItemModel)
    , m_saveClickedRow(0)
{
    setObjectName("Accounts");

    m_createBtn->setFixedSize(50, 50);
    //~ contents_path /accounts/New Account
    m_createBtn->setToolTip(tr("Create Account"));

    QVBoxLayout *mainContentLayout = new QVBoxLayout();
    mainContentLayout->setMargin(0);
    mainContentLayout->addWidget(m_userlistView);
    mainContentLayout->addWidget(m_createBtn, 0, Qt::AlignHCenter);

    m_userlistView->setFrameShape(QFrame::NoFrame);
    m_userlistView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_userlistView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_userlistView->setEditTriggers(QListView::NoEditTriggers);
    m_userlistView->setDragEnabled(false);
    m_userlistView->setIconSize(QSize(30, 30));
    m_userlistView->setLayoutDirection(Qt::LeftToRight);
    m_userlistView->setModel(m_userItemModel);

    setLayout(mainContentLayout);

    connect(m_userlistView, &QListView::clicked, this, &AccountsWidget::onItemClicked);
    connect(m_createBtn, &QPushButton::clicked, this, &AccountsWidget::requestCreateAccount);
}

void AccountsWidget::setModel(UserModel *model)
{
    m_userModel = model;

    m_userList << nullptr;
    m_userItemModel->appendRow(new DStandardItem);

    connect(model, &UserModel::userAdded, this, [this](User * user) {
        addUser(user);
    });
    connect(model, &UserModel::userRemoved, this, &AccountsWidget::removeUser);
    //给账户列表添加用户数据
    for (auto user : model->userList()) {
        addUser(user, false);
    }
}

void AccountsWidget::showDefaultAccountInfo()
{
    QModelIndex qindex = m_userItemModel->index(0, 0);
    m_userlistView->setFocus();
    m_userlistView->setCurrentIndex(qindex);
    Q_EMIT m_userlistView->clicked(qindex);
}

void AccountsWidget::showLastAccountInfo()
{
    int lastindex = m_userItemModel->rowCount() - 1;
    QModelIndex qindex = m_userItemModel->index(lastindex, 0);
    m_userlistView->setFocus();
    m_userlistView->setCurrentIndex(qindex);
    Q_EMIT m_userlistView->clicked(qindex);
}

void AccountsWidget::setShowFirstUserInfo(bool show)
{
    m_isShowFirstUserInfo = show;
}

void AccountsWidget::addUser(User *user, bool t1)
{
    //active
    m_userList << user;
    DStandardItem *item = new DStandardItem;
    item->setData(0, AccountsWidget::ItemDataRole);
    m_userItemModel->appendRow(item);

    connectUserWithItem(user);

    connect(user, &User::isCurrentUserChanged, this, [ = ](bool isCurrentUser) {
        if (isCurrentUser) {
            auto tindex = m_userList.indexOf(user);
            auto titem = m_userItemModel->takeRow(tindex);

            m_userItemModel->removeRow(0);
            m_userItemModel->insertRow(0, titem);

            m_userList.removeOne(user);
            m_userList[0] = user;

            showDefaultAccountInfo();
        }
    });

    if (t1) {
        return;
    }

    QPixmap pixmap;
    if (user->currentAvatar().startsWith("file://")) {
        pixmap = PixmapToRound(ZoomImage(QUrl(user->currentAvatar()).toLocalFile()), UserImageRadius);

    } else {
        pixmap = PixmapToRound(ZoomImage(user->currentAvatar()), UserImageRadius);
    }
    item->setIcon(QIcon(pixmap));

    //对用户全名做限制，如果长度超过32，就在后面显示...
    QString fullname = user->displayName();
    if (fullname.length() > 32) {
        QString newfullname = fullname.left(32) + QString("...");
        item->setText(newfullname);
    } else {
        item->setText(fullname);
    }

    if (user->isCurrentUser()) {
        //如果是当前用户
        auto tttitem = m_userItemModel->takeRow(m_userItemModel->rowCount() - 1);
        Q_ASSERT(tttitem[0] == item);
        m_userItemModel->removeRow(0);
        m_userItemModel->insertRow(0, item);

        m_userList[0] = user;
        m_userList.pop_back();

        QTimer::singleShot(0, this, &AccountsWidget::showDefaultAccountInfo);
    } else {
        int count = m_userItemModel->rowCount();
        for (int idx = 1; idx < count; ++idx) {
            if (user->createdTime() < m_userList[idx]->createdTime()) {
                auto tttitem = m_userItemModel->takeRow(count - 1);
                Q_ASSERT(tttitem[0] == item);
                m_userItemModel->insertRow(idx, item);

                m_userList.insert(idx, user);
                m_userList.pop_back();
                break;
            }
        }
    }
}

void AccountsWidget::removeUser(User *user)
{
    m_userItemModel->removeRow(m_userList.indexOf(user)); // It will delete when remove
    m_userList.removeOne(user);

    if (m_isShowFirstUserInfo) {
        showDefaultAccountInfo();
    } else {
        showLastAccountInfo();
    }
}

void AccountsWidget::onItemClicked(const QModelIndex &index)
{
    m_saveClickedRow = index.row();
    Q_EMIT requestShowAccountsDetail(m_userList[index.row()]);
}

void AccountsWidget::connectUserWithItem(User *user)
{
    connect(user, &User::nameChanged, this, [ = ](const QString &) {
        int tindex = m_userList.indexOf(user);
        auto titem = m_userItemModel->item(tindex);
        if (!titem) {
            return;
        }
        titem->setText(user->displayName());
    });
    connect(user, &User::fullnameChanged, this, [ = ](const QString &) {
        int tindex = m_userList.indexOf(user);
        auto titem = m_userItemModel->item(tindex);
        if (!titem) {
            return;
        }
        //对用户全名做限制，如果长度超过32，就在后面显示...
        QString fullname = user->displayName();
        if (fullname.length() > 32) {
            QString newfullname = fullname.left(32) + QString("...");
            titem->setText(newfullname);
        } else {
            titem->setText(fullname);
        }
    });
    connect(user, &User::currentAvatarChanged, this, [ = ](const QString & avatar) {
        int tindex = m_userList.indexOf(user);
        auto titem = m_userItemModel->item(tindex);
        if (!titem) {
            return;
        }
        auto ratio = this->devicePixelRatioF();
        auto path = avatar;
        if (ratio > 4.0) {
            path.replace("icons/", "icons/bigger/");
        }

        QPixmap pixmap;
        if (path.startsWith("file://")) {
            pixmap = PixmapToRound(ZoomImage(QUrl(path).toLocalFile()), UserImageRadius);
        } else {
            pixmap = PixmapToRound(ZoomImage(path), UserImageRadius);
        }
        titem->setIcon(QIcon(pixmap));
    });
    connect(user, &User::createdTimeChanged, this, [ = ](const quint64 & createdtime) {
        if (user->isCurrentUser()) {
            return;
        }

        int tindex = m_userList.indexOf(user);
        auto titem = m_userItemModel->item(tindex);
        if (!titem) {
            return;
        }
        titem->setData(QVariant::fromValue(createdtime), AccountsWidget::ItemDataRole);

        for (int i = 1; i < m_userItemModel->rowCount(); ++i) {
            quint64 icreatedtime = m_userItemModel->index(i, 0).data(AccountsWidget::ItemDataRole).toULongLong();
            if (createdtime < icreatedtime) {
                m_userItemModel->takeRow(tindex);
                m_userList.takeAt(tindex);
                auto dstIdx = i;
                dstIdx += (tindex < i ? -1 : 0);
                m_userItemModel->insertRow(dstIdx, titem);
                m_userList.insert(dstIdx, user);
                break;
            }
        }
    });
}

QPixmap AccountsWidget::ZoomImage(const QString &src)
{
    QPixmap srcPixmap(src);
    QPixmap iconPixmap = srcPixmap.scaled(2 * UserImageRadius, 2 * UserImageRadius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return iconPixmap;
}

QPixmap AccountsWidget::PixmapToRound(const QPixmap &src, const int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    auto ratio = devicePixelRatioF();
    QSize size = QSizeF(2 * radius * ratio, 2 * radius * ratio).toSize();

    QPixmap mask(size);
    mask.fill(Qt::transparent);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addEllipse(0, 0, size.width(), size.height());
    painter.setClipPath(path);

    auto pixmap = QPixmap(src).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, pixmap);

    return mask;
}

void AccountsWidget::handleRequestBack(AccountsWidget::ActionOption option)
{
    switch (option) {
    case AccountsWidget::ClickCancel: { //点击取消
        QModelIndex qindex0 = m_userItemModel->index(m_saveClickedRow, 0);
        m_userlistView->setFocus();
        m_userlistView->setCurrentIndex(qindex0);
        onItemClicked(qindex0);
        }
        break;
    case AccountsWidget::CreateUserSuccess: { //创建账户成功
        QModelIndex qindex1 = m_userItemModel->index(m_userItemModel->rowCount() - 1, 0);
        m_userlistView->setFocus();
        m_userlistView->setCurrentIndex(qindex1);
        onItemClicked(qindex1);
        }
        break;
    case AccountsWidget::ModifyPwdSuccess: { //修改密码成功
        QModelIndex qindex2 = m_userItemModel->index(0, 0);
        m_userlistView->setFocus();
        m_userlistView->setCurrentIndex(qindex2);
        onItemClicked(qindex2);
        }
        break;
    }
}
