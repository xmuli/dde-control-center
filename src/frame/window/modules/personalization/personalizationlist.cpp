/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LiLinling <lilinling_cm@deepin.com>
 *
 * Maintainer: LiLinling <lilinling_cm@deepin.com>
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
#include "personalizationlist.h"

#include <DListView>

#include <QVBoxLayout>

using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::personalization;

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

PersonalizationList::PersonalizationList(QWidget *parent)
    : QWidget(parent)
    , m_categoryListView(new DListView())
    , m_model(new QStandardItemModel(this))
    , m_centralLayout(new QVBoxLayout())
{
    setObjectName("personanization");
    m_categoryListView->setIconSize(QSize(26, 26));
    m_categoryListView->setResizeMode(QListView::Adjust);
    m_categoryListView->setMovement(QListView::Static);
    m_categoryListView->setFrameShape(QFrame::NoFrame);

    //Initialize second page view and model
    QStringList menus;
    //~ contents_path /personalization/General
    menus << tr("General");
    //~ contents_path /personalization/Icon Theme
    menus << tr("Icon Theme");
    //~ contents_path /personalization/Cursor Theme
    menus << tr("Cursor Theme");
    //~ contents_path /personalization/Font
    menus << tr("Font");

    QStringList icons;
    icons << "dcc_common"
          << "dcc_Icon_theme"
          << "dcc_cursor_theme"
          << "dcc_Font";

    for (int i = 0; i < menus.size(); ++i) {
        DStandardItem *item = new DStandardItem(QIcon::fromTheme(icons.at(i)), menus.at(i));

        item->setSizeHint(QSize(230, 48));
        item->setData(QVariant::fromValue(QMargins(10, 10, 10, 0)), Dtk::MarginsRole);

        m_model->appendRow(item);
    }

    m_categoryListView->setModel(m_model);
    m_categoryListView->setEditTriggers(QAbstractItemView:: NoEditTriggers);

    m_centralLayout->addWidget(m_categoryListView);
    setLayout(m_centralLayout);
    connect(m_categoryListView, &QListView::clicked, this, &PersonalizationList::onCategoryClicked);
    //set default show page
    m_categoryListView->setCurrentIndex(m_model->indexFromItem(m_model->item(0)));
}

void PersonalizationList::onCategoryClicked(const QModelIndex &index)
{
    switch (index.row()) {
    case 0:
        Q_EMIT requestShowGeneral();
        break;
    case 1:
        Q_EMIT requestShowIconTheme();
        break;
    case 2:
        Q_EMIT requestShowCursorTheme();
        break;
    case 3:
        Q_EMIT requestShowFonts();
        break;
    default:
        break;
    }
}

void PersonalizationList::setCurrentIndex(int row)
{
    if (row > m_model->rowCount())
        row = 0;

    m_categoryListView->setCurrentIndex(m_model->indexFromItem(m_model->item(row)));
}
