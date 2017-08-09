#include "powermodule.h"
#include "powerwidget.h"
#include "powerworker.h"
#include "powermodel.h"
#include <QGSettings>

using namespace dcc;
using namespace dcc::power;

PowerModule::PowerModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent),
      ModuleInterface(frame),

      m_powerWidget(nullptr),
      m_powerAction(nullptr)
{

}

PowerModule::~PowerModule()
{
    m_powerModel->deleteLater();
    m_powerWorker->deleteLater();
}

void PowerModule::initialize()
{
    m_powerModel = new PowerModel;
    m_powerWorker = new PowerWorker(m_powerModel);

    m_powerModel->moveToThread(qApp->thread());
    m_powerWorker->moveToThread(qApp->thread());
}

void PowerModule::moduleActive()
{
    m_powerWorker->active();
}

void PowerModule::moduleDeactive()
{
    m_powerWorker->deactive();
}

void PowerModule::reset()
{

}

void PowerModule::contentPopped(ContentWidget * const w)
{
    Q_UNUSED(w);
}

ModuleWidget *PowerModule::moduleWidget()
{
    if (!m_powerWidget)
    {
        m_powerWidget = new PowerWidget;
        m_powerWidget->setModel(m_powerModel);

        connect(m_powerWidget, &PowerWidget::requestSetScreenBlackLock, m_powerWorker, &PowerWorker::setScreenBlackLock);
        connect(m_powerWidget, &PowerWidget::requestSetSleepLock, m_powerWorker, &PowerWorker::setSleepLock);
        connect(m_powerWidget, &PowerWidget::requestSetSleepOnLidClosed, m_powerWorker, &PowerWorker::setSleepOnLidClosed);
        connect(m_powerWidget, &PowerWidget::requestSetScreenBlackDelay, m_powerWorker, &PowerWorker::setScreenBlackDelay);
        connect(m_powerWidget, &PowerWidget::requestSetSleepDelay, m_powerWorker, &PowerWorker::setSleepDelay);
        connect(m_powerWidget, &PowerWidget::requestSetDefaultPowerAction, this, &PowerModule::showPowerActionPage);
    }

    return m_powerWidget;
}

void PowerModule::showPowerActionPage()
{
    if (!m_powerAction) {
        m_powerAction = new PowerAction;
        connect(m_powerAction, &PowerAction::requestSelected, this, &PowerModule::setPowerAction);
    }

    QGSettings gsettings("com.deepin.dde.power");
    m_powerAction->setDefault(gsettings.get("power-button-pressed-exec"));

    m_frameProxy->pushWidget(this, m_powerAction);
}

const QString PowerModule::name() const
{
    return QStringLiteral("power");
}

void PowerModule::setPowerAction(const QVariant &value)
{
    QGSettings gsettings("com.deepin.dde.power");
    gsettings.set("power-button-pressed-exec", value);
    m_powerAction->setDefault(gsettings.get("power-button-pressed-exec"));
}
