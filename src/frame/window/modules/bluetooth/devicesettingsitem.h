/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     andywang <andywang_cm@deepin.com>
 *
 * Maintainer: andywang <andywang_cm@deepin.com>
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

#pragma once

#include "interface/namespace.h"
#include "modules/bluetooth/device.h"
#include "modules/bluetooth/adapter.h"

#include <DListView>
#include <DSpinner>

#include <QObject>
#include <QPointer>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QStyle;
QT_END_NAMESPACE

namespace dcc {
namespace bluetooth {
class Device;
class Adapter;
}
}

namespace DCC_NAMESPACE {
namespace bluetooth {

struct BtSortInfo {
    bool connected = false;
    int time;
    bool operator < (const BtSortInfo &other)
    {
        if (connected ^ other.connected) {
            return !connected;
        } else {
            return time < other.time;
        }
    }
};

class BtStandardItem : public DStandardItem
{
public:
    enum {
        SortRole = Dtk::UserRole + 1,
    };

    using DStandardItem::DStandardItem;

    void setSortInfo(const BtSortInfo &sortInfo) {
        setData(QVariant::fromValue(sortInfo), SortRole);
    }

    BtSortInfo sortInfo() {
        return data(SortRole).value<BtSortInfo>();
    }

    bool operator < (const QStandardItem &other) const {
        BtSortInfo thisApInfo = data(SortRole).value<BtSortInfo>();
        BtSortInfo otherApInfo = other.data(SortRole).value<BtSortInfo>();

        return thisApInfo < otherApInfo;

    }
};

class DeviceSettingsItem : public QObject
{
    Q_OBJECT
public:
    explicit DeviceSettingsItem(const dcc::bluetooth::Device *device, QStyle *style);
    virtual ~DeviceSettingsItem();
    BtStandardItem *getStandardItem(DTK_WIDGET_NAMESPACE::DListView *parent = nullptr);
    BtStandardItem *createStandardItem(DTK_WIDGET_NAMESPACE::DListView *parent = nullptr);
    const dcc::bluetooth::Device *device() const;
    void setLoading(const bool loading);
    void resetDeviceItem() { m_deviceItem = nullptr; }

private:
    void setDevice(const dcc::bluetooth::Device *device);
    void initItemActionList();

Q_SIGNALS:
    void requestConnectDevice(const dcc::bluetooth::Device *device, const dcc::bluetooth::Adapter *adapter) const;
    void requestShowDetail(const dcc::bluetooth::Device *device) const;
    void requestSort();

private Q_SLOTS:
    void onDeviceStateChanged(const dcc::bluetooth::Device::State &state, bool paired);
    void onDevicePairedChanged(const bool &paired);

private:
    const dcc::bluetooth::Device *m_device;
    QPointer<DTK_WIDGET_NAMESPACE::DSpinner> m_loadingIndicator;
    BtStandardItem *m_deviceItem;
    DTK_WIDGET_NAMESPACE::DListView *m_parentDListView;
    DViewItemActionList m_dActionList;
    QPointer<DViewItemAction> m_loadingAction;
    QPointer<DViewItemAction> m_textAction;
    QPointer<DViewItemAction> m_iconAction;
    QStyle *m_style;
};
} // namespace DCC_NAMESPACE
} // namespace dcc
Q_DECLARE_METATYPE(DCC_NAMESPACE::bluetooth::BtSortInfo)
