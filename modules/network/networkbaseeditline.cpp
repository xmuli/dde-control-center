#include <QHBoxLayout>
#include <QPointer>
#include <QDebug>

#include <libdui/libdui_global.h>
#include <libdui/dconstants.h>

#include "constants.h"

#include "networkglobal.h"
#include "networkbaseeditline.h"

static AvailableKeyMap availableKeys;
static QStringList availableSections;
static ErrorInfo errors;
static QPointer<NetworkBaseEditLine> firstBaseEditLine;

void updateAllEditLineVisible(NetworkBaseEditLine *editLine)
{
    for(NetworkBaseEditLine *line : editLine->parent()->findChildren<NetworkBaseEditLine*>()) {
        line->updateVisible();
    }
}

NetworkBaseEditLine::NetworkBaseEditLine(const QString &section, const QString &key,
                                         DBusConnectionSession *dbus, const QString &title,
                                         QWidget *parent):
    QWidget(parent),
    m_dbus(dbus),
    m_section(section),
    m_key(key)
{
    QHBoxLayout *layout = new QHBoxLayout;
    titleLabel = new DLabel(title);

    titleLabel->setWordWrap(true);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->setContentsMargins(15, 0, 15, 0);
    layout->setSpacing(15);
    layout->addWidget(titleLabel);

    setLayout(layout);
    updateVisible();

    if(!firstBaseEditLine) {
        firstBaseEditLine = this;

        availableSections = m_dbus->availableSections();
        availableKeys = m_dbus->availableKeys();
        errors = m_dbus->errors();

        connect(dbus, &DBusConnectionSession::AvailableSectionsChanged, this, [this] {
            availableSections = m_dbus->availableSections();
            updateAllEditLineVisible(this);
        });

        connect(dbus, &DBusConnectionSession::AvailableKeysChanged, this, [this] {
            availableKeys = m_dbus->availableKeys();
            updateAllEditLineVisible(this);
        });

        connect(dbus, &DBusConnectionSession::ErrorsChanged, this, [this] {
            errors = m_dbus->errors();
        });
    }

    connect(this, &NetworkBaseEditLine::setTitle, titleLabel, &DLabel::setText);
    connect(dbus, &DBusConnectionSession::ConnectionDataChanged, this, [this]{
        if(isVisible() && alwaysUpdate()) {
            setCacheValue(dbusKey());
        }
    });

    setFixedWidth(DCC::ModuleContentWidth);
}

QString NetworkBaseEditLine::section() const
{
    return m_section;
}

QString NetworkBaseEditLine::key() const
{
    return m_key;
}

QJsonValue NetworkBaseEditLine::cacheValue() const
{
    return m_cacheValue;
}

bool NetworkBaseEditLine::alwaysUpdate() const
{
    return m_alwaysUpdate;
}

bool NetworkBaseEditLine::setKeyAlways() const
{
    return m_setKeyAlways;
}

bool NetworkBaseEditLine::readOnly() const
{
    return m_readOnly;
}

void NetworkBaseEditLine::setDBusKey(const QJsonValue &key)
{
    if (key != cacheValue() || isValueError() || setKeyAlways()) {
        QString json;

        if(key.isString())
            json.append("\"" + key.toString() + "\"");
        else if(key.isDouble())
            json.append(QString::number(key.toDouble(-1)));
        else if(key.isBool())
            json.append(key.toBool() ? "true" : "false");
        /// TODO 此处将key转换成json的value类型，由于Qt json目前并不支持此类型，故需手动转换
        /// 目前只处理了String int double bool类型，以后有新类型的需求记得在此处添加

        setCacheValue(key);

        m_tempValue = key;

        if(m_updateKeying) {
            return;
        }

        m_updateKeying = true;

        ASYN_CALL(m_dbus->SetKey(section(), this->key(), json), {
                      m_updateKeying = false;

                      if(key != m_tempValue) {
                          bool key_always = setKeyAlways();

                          setSetKeyAlways(true);
                          setDBusKey(m_tempValue);
                          setSetKeyAlways(key_always);
                      }
                  }, this, key)
    }
}

const QJsonValue NetworkBaseEditLine::dbusKey()
{
    const QString &json = m_dbus->GetKey(section(), key());
    const QJsonDocument &json_doc = QJsonDocument::fromJson("{\"value\":" + json.toUtf8() + "}");

    return json_doc.object()["value"];
}

const QJsonArray NetworkBaseEditLine::getAvailableValues()
{
    const QString &json = m_dbus->GetAvailableValues(section(), key());
    const QJsonDocument &json_doc = QJsonDocument::fromJson(json.toUtf8());

    return json_doc.array();
}

const QList<QJsonValue> NetworkBaseEditLine::getAvailableValuesValue()
{
    QList<QJsonValue> list;
    const QJsonArray &array = getAvailableValues();

    if(array.isEmpty()) {
        return list;
    }

    for(const QJsonValue &value : array) {
        list << value.toObject()["Value"];
    }

    return list;
}

const QString NetworkBaseEditLine::getAvailableValuesTextByValue()
{
    const QJsonArray &array = getAvailableValues();

    if(!array.isEmpty() && !cacheValue().isNull()) {
        for(const QJsonValue &value : array) {
            const QJsonObject &obj = value.toObject();
            if(obj["Value"] == cacheValue()) {
                return obj["Text"].toString();
            }
        }
    }

    return "";
}

const QStringList NetworkBaseEditLine::getAvailableValuesText()
{
    const QJsonArray &array = getAvailableValues();

    QStringList list;

    if(array.isEmpty()) {
        return list;
    }

    for(const QJsonValue &value : array) {
        list << value.toObject()["Text"].toString();
    }

    return list;
}

int NetworkBaseEditLine::getAvailableValuesIndex()
{
    const QJsonArray &array = getAvailableValues();

    if(!array.isEmpty() && !cacheValue().isNull()) {
        for(int i = 0; i < array.count(); ++i) {
            const QJsonObject &obj = array[i].toObject();
            if(obj["Value"] == cacheValue()) {
                return i;
            }
        }
    }

    return -1;
}

void NetworkBaseEditLine::setRightWidget(QWidget *widget)
{
    QBoxLayout *layout = qobject_cast<QBoxLayout*>(this->layout());
    layout->addWidget(widget, 0, Qt::AlignRight);
}

void NetworkBaseEditLine::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    if(m_cacheValue.isNull() || alwaysUpdate()) {
        setCacheValue(dbusKey());
    } else {
        if(m_cacheValue != dbusKey()) {
            bool setKeyAlways = this->setKeyAlways();

            setSetKeyAlways(true);
            setDBusKey(m_cacheValue);
            setSetKeyAlways(setKeyAlways);
        }

        emit widgetShown();
    }

    emit showed();
}

void NetworkBaseEditLine::changeEvent(QEvent *e)
{
    if(!isVisible() && e->type() == QEvent::ParentChange) {
        updateVisible();
    }
}

void NetworkBaseEditLine::setCacheValue(const QJsonValue &value)
{
    if(m_cacheValue == value)
        return;

    m_cacheValue = value;

    emit cacheValueChanged();
}

void NetworkBaseEditLine::updateVisible()
{
    if(parentWidget()) {
        bool visible = availableSections.indexOf(section()) != -1
                &&  availableKeys[section()].indexOf(key()) != -1;
        setVisible(visible);
    }
}

void NetworkBaseEditLine::setDBusKey(const QString &key)
{
    setDBusKey(QJsonValue(key));
}

void NetworkBaseEditLine::setSection(const QString &section)
{
    m_section = section;
}

void NetworkBaseEditLine::setKey(const QString &key)
{
    m_key = key;
}

void NetworkBaseEditLine::setAlwaysUpdate(bool alwaysUpdate)
{
    m_alwaysUpdate = alwaysUpdate;
}

void NetworkBaseEditLine::setSetKeyAlways(bool setKeyAlways)
{
    m_setKeyAlways = setKeyAlways;
}

void NetworkBaseEditLine::setReadOnly(bool readOnly)
{
    if (m_readOnly == readOnly)
        return;

    m_readOnly = readOnly;
    emit readOnlyChanged(readOnly);
}

void NetworkBaseEditLine::checkKey()
{
    if(isValueError()) {
        emit showErrorAlert();

        qDebug() << "key error: " << section() << key() << cacheValue() << titleLabel->text();
    }
}

bool NetworkBaseEditLine::isValueError() const
{
    return !errors.isEmpty() && !errors[section()].isEmpty()
            && !errors[section()][key()].isEmpty();
}
