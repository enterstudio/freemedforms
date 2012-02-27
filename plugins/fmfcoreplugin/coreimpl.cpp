/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developer : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "coreimpl.h"
#include "appconfigwizard.h"
#include "commandlineparser.h"

#include <coreplugin/settings_p.h>
#include <coreplugin/isettings.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/theme.h>
#include <coreplugin/translators.h>
#include <coreplugin/actionmanager/actionmanager_p.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/contextmanager/contextmanager.h>
#include <coreplugin/contextmanager/contextmanager_p.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/modemanager/modemanager.h>
#include <coreplugin/constants_icons.h>

#include <utils/log.h>
#include <utils/global.h>
#include <utils/updatechecker.h>
#include <utils/versionnumber.h>
#include <translationutils/constanttranslations.h>

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

namespace Core {
namespace Internal {
static CoreImpl *m_instance = 0;
} // namespace Internal
} // namespace Core


using namespace Core;
using namespace Core::Internal;
using namespace Trans::ConstantTranslations;

ICore* ICore::instance()
{
    return m_instance;
}

// instance is created by Core::CorePlugin()
CoreImpl::CoreImpl(QObject *parent) :
    ICore(parent),
    m_MainWindow(0),
    m_ActionManager(0),
    m_ContextManager(0),
    m_ModeManager(0),
    m_Patient(0),
    m_User(0),
    m_Script(0)
{
    setObjectName("Core");
    m_Settings = new SettingsPrivate(this);
    m_Settings->setPath(ISettings::UpdateUrl, Utils::Constants::FREEMEDFORMS_UPDATE_URL);

    m_Theme = new ThemePrivate(this);
    m_Theme->setThemeRootPath(m_Settings->path(ISettings::ThemeRootPath));
    m_CommandLine = new CommandLine();

    QTime chrono;
    chrono.start();
    bool logChrono = m_CommandLine->value(ICommandLine::Chrono).toBool();
    if (logChrono)
        Utils::Log::logTimeElapsed(chrono, "Core", "command line parsing");

    m_Theme->createSplashScreen(Constants::FREEMEDFORMS_SPLASHSCREEN);
    m_Settings->setPath(ISettings::Splashscreen, Constants::FREEMEDFORMS_SPLASHSCREEN);

    // add translators
    m_Theme->messageSplashScreen(tkTr(Trans::Constants::INITIALIZING_TRANSLATIONS));
    m_Translators = new Translators(this);
    m_Translators->setPathToTranslations(m_Settings->path(ISettings::TranslationsPath));
    // Qt
    m_Translators->addNewTranslator("qt");
    // Core Needed Libs
    m_Translators->addNewTranslator(Trans::Constants::CONSTANTS_TRANSLATOR_NAME);
    m_Translators->addNewTranslator("utils");
    m_Translators->addNewTranslator("medicalutils");
    m_Translators->addNewTranslator("calendar");
    m_Translators->addNewTranslator("fmfcoreplugin");

    if (logChrono)
        Utils::Log::logTimeElapsed(chrono, "Core", "translators");

    m_Theme->messageSplashScreen(tkTr(Trans::Constants::STARTING_APPLICATION_AT_1).arg(QDateTime::currentDateTime().toString()));
    LOG(tkTr(Trans::Constants::STARTING_APPLICATION_AT_1).arg( QDateTime::currentDateTime().toString()));

    foreach(const QString &l, QCoreApplication::libraryPaths()) {
        LOG(tkTr(Trans::Constants::USING_LIBRARY_1).arg(l));
    }

    // initialize the settings
    m_Theme->messageSplashScreen(tkTr(Trans::Constants::LOADING_SETTINGS));

    // WINE compatibility (only for testing under ubuntu when crosscompiling)
#ifdef Q_OS_WIN
    // For WINE testings
    if (m_CommandLine->value(Core::ICommandLine::RunningUnderWine).toBool()) {
        LOG("Running under Wine environnement." );
        QFont::insertSubstitution("MS Shell Dlg", "Tahoma" );
        QFont::insertSubstitution("MS Shell Dlg 2", "Tahoma" );
    }
#endif

//    m_MainWindow = new MainWindow();
//    m_ActionManager = new ActionManagerPrivate(m_MainWindow);
//    m_ContextManager = new ContextManagerPrivate(m_MainWindow);
    m_FileManager = new FileManager(this);
    m_UpdateChecker = new Utils::UpdateChecker(this);
    m_UID = new UniqueIDManager();
    if (logChrono)
        Utils::Log::logTimeElapsed(chrono, "Core", "managers");

    if (Utils::isRunningOnMac())
        QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);

    // ready
    m_Theme->messageSplashScreen(tr("Core intialization finished..."));

    LOG(tr("Core intialization finished..."));
    if (logChrono)
        Utils::Log::logTimeElapsed(chrono, "Core", "end of core intialization");

    m_instance = this;
}

CoreImpl::~CoreImpl()
{
    Q_EMIT coreAboutToClose();
    if (m_UID)
        delete m_UID;
    if (m_CommandLine)
        delete m_CommandLine;
    if (m_ModeManager)
        delete m_ModeManager;
    if (m_MainWindow)
        delete m_MainWindow;
}

IMainWindow *CoreImpl::mainWindow() const  { return m_MainWindow; }
void CoreImpl::setMainWindow(IMainWindow *win)
{
    // can be called only once
    Q_ASSERT(m_MainWindow==0);
    Q_ASSERT(m_ActionManager==0);
    Q_ASSERT(m_ContextManager==0);
    Q_ASSERT(m_ModeManager==0);
    m_MainWindow = win;
    m_ModeManager = new ModeManager(m_MainWindow);
    m_ContextManager = new ContextManagerPrivate(m_MainWindow);
    m_ActionManager = new ActionManagerPrivate(m_MainWindow);
}

ActionManager *CoreImpl::actionManager() const { return m_ActionManager; }
ContextManager *CoreImpl::contextManager() const { return m_ContextManager; }
UniqueIDManager *CoreImpl::uniqueIDManager() const { return m_UID; }
ITheme *CoreImpl::theme() const { return m_Theme; }
Translators *CoreImpl::translators() const { return m_Translators; }
ISettings *CoreImpl::settings() const{ return m_Settings; }
FileManager *CoreImpl::fileManager() const { return m_FileManager; }
Utils::UpdateChecker *CoreImpl::updateChecker() const { return m_UpdateChecker; }
ICommandLine *CoreImpl::commandLine() const { return m_CommandLine; }
ModeManager *CoreImpl::modeManager() const {return m_ModeManager;}

bool CoreImpl::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    // Test Versions
    QString appr = m_Settings->licenseApprovedApplicationNumber();
    if (appr.isEmpty())
        appr="0.0.0";
    Utils::VersionNumber approved(appr);
    Utils::VersionNumber app(qApp->applicationVersion());
    // End

    // For alpha -> remove config.ini to restart the app configurator
    if (Utils::isAlpha() && (approved < app)) {
        // remove the config.ini content
        if (!QFile(settings()->fileName()).remove())
            LOG_ERROR("Unable to clean user settings... " + settings()->fileName());

        // remove all files in UserResources dir
        QString error;
        if (!settings()->path(Core::ISettings::ReadWriteDatabasesPath).contains("global_resources")) {
            if (!Utils::removeDirRecursively(settings()->path(Core::ISettings::ReadWriteDatabasesPath), &error))
                LOG_FOR("Settings", "Unable to clean user databases... " + error);
        } else {
            LOG_FOR("Settings", "Unable to clean user databases... The global_resources can not be deleted");
        }

        settings()->sync();
        settings()->setFirstTimeRunning(true);
    }

    // first time runnning ?
    if (m_Settings->firstTimeRunning()) {
        AppConfigWizard wizard;
        if (wizard.exec()==QDialog::Rejected) {
            return false;
        }
        m_Settings->noMoreFirstTimeRunning();
        /** \todo code here: if alpha -> delete old configuration && databases */
        m_Settings->setLicenseApprovedApplicationNumber(qApp->applicationVersion());
        // Clear identifiants
        Utils::DatabaseConnector connector = settings()->databaseConnector();
        connector.setClearLog("_");
        connector.setClearPass("_");
        settings()->setDatabaseConnector(connector);
//        qWarning() << connector;

//        // show the license agreement dialog
//#ifndef LINUX_INTEGRATED
//        if (!Utils::defaultLicenceAgreementDialog("", Utils::LicenseTerms::GPLv3))
//            return false;
//#endif
//        m_Settings->setLicenseApprovedApplicationNumber(qApp->applicationVersion());
//    } else if (m_Settings->licenseApprovedApplicationNumber() != qApp->applicationVersion()) {
//        // show the license agreement dialog
//#ifndef LINUX_INTEGRATED
//        if (!Utils::defaultLicenceAgreementDialog(
//                QCoreApplication::translate("Core", "You are running a new version of %1, you need to renew the licence agreement.").arg(qApp->applicationName()),
//                Utils::LicenseTerms::GPLv3 ))
//            return false;
//#endif
    }

    return true;
}

void CoreImpl::extensionsInitialized()
{
    LOG("Core opened");
    Q_EMIT coreOpened();
}

