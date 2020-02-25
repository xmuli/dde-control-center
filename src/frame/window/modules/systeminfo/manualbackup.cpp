#include "manualbackup.h"

#include <QVBoxLayout>
#include <QLabel>
#include <DBackgroundGroup>
#include <QPushButton>
#include <QDir>
#include <QProcess>
#include <DDBusSender>

DCORE_USE_NAMESPACE

using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::systeminfo;

ManualBackup::ManualBackup(QWidget* parent)
    : QWidget(parent)
    , m_directoryChooseWidget(new DFileChooserEdit)
    , m_tipsLabel(new QLabel)
    , m_backupBtn(new QPushButton(tr("Backup")))
{
    m_tipsLabel->setWordWrap(true);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(5);

    QHBoxLayout* chooseLayout = new QHBoxLayout;
    chooseLayout->setMargin(0);
    chooseLayout->setSpacing(0);
    chooseLayout->addWidget(new QLabel(tr("Set backup directory")), 0, Qt::AlignVCenter);
    chooseLayout->addSpacing(5);
    chooseLayout->addWidget(m_directoryChooseWidget, 0, Qt::AlignVCenter);

    QWidget* bgWidget = new QWidget;
    bgWidget->setLayout(chooseLayout);

    QVBoxLayout* bgLayout = new QVBoxLayout;
    bgLayout->addWidget(bgWidget);

    DBackgroundGroup* bgGroup = new DBackgroundGroup;
    bgGroup->setLayout(bgLayout);
    bgGroup->setBackgroundRole(QPalette::Window);
    bgGroup->setItemSpacing(1);

    mainLayout->addWidget(bgGroup);
    mainLayout->addWidget(m_tipsLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(m_backupBtn);

    setLayout(mainLayout);

    m_tipsLabel->hide();
    m_backupBtn->setEnabled(false);

    QFileDialog* fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::Directory);
    m_directoryChooseWidget->setFileDialog(fileDialog);

    connect(m_directoryChooseWidget, &DFileChooserEdit::fileChoosed, this, &ManualBackup::onChoose);
    connect(m_backupBtn, &QPushButton::clicked, this, &ManualBackup::backup);
}

void ManualBackup::onChoose()
{
    m_backupBtn->setEnabled(!m_directoryChooseWidget->lineEdit()->text().isEmpty());
}

void ManualBackup::backup()
{
    if (m_directoryChooseWidget->lineEdit()->text().isEmpty()) {
        return;
    }

    QProcess* process = new QProcess(this);
    process->start("pkexec", QStringList() << "/bin/restore-tool" << "--actionType" << "manual_backup" << "--path" << m_directoryChooseWidget->lineEdit()->text());
    process->waitForFinished();

    process->start("pkexec", { "/sbin/grub-reboot", "Deepin Recovery" });
    process->waitForFinished();
    process->deleteLater();

    DDBusSender()
    .service("com.deepin.dde.shutdownFront")
    .path("/com/deepin/dde/shutdownFront")
    .interface("com.deepin.dde.shutdownFront")
    .method("Restart")
    .call();
}