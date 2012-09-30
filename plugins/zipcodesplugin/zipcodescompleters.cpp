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
 *   Main developers : Eric MAEKER, MD <eric.maeker@gmail.com>             *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "zipcodescompleters.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>

#include <utils/global.h>
#include <utils/log.h>
#include <datapackutils/datapackcore.h>
#include <datapackutils/ipackmanager.h>
#include <datapackutils/pack.h>
#include <translationutils/constants.h>
#include <translationutils/trans_database.h>

#include <QLineEdit>
#include <QSqlTableModel>
#include <QLocale>
#include <QCompleter>
#include <QApplication>
#include <QListView>
#include <QDir>
#include <QString>
#include <QComboBox>
#include <QToolButton>
#include <QSqlQuery>

#include <QDebug>

using namespace ZipCodes;
using namespace Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline Core::ITheme *theme() {return Core::ICore::instance()->theme();}
static inline DataPack::DataPackCore &dataPackCore() { return DataPack::DataPackCore::instance(); }
static inline DataPack::IPackManager *packManager() { return dataPackCore().packManager(); }

/**
  \class ZipCountryModel
  Private model used by the ZipCountryCompleters
*/
ZipCountryModel::ZipCountryModel(QObject *parent, QSqlDatabase _db, bool dbAvailable) :
    QSqlQueryModel(parent),
    m_DbAvailable(dbAvailable)
{
    db = _db;
    m_countryIso = Utils::countryToIso(QLocale().country()).toLower();

    if (m_DbAvailable) {  // BUG with if (m_DbAvailable && db.isOpen()) --> returning false with (true && true) ????
        if (db.isOpen()) {
            setQuery("SELECT ZIP, CITY FROM ZIPS LIMIT 0, 1", _db);
            if (!query().isActive()) {
                LOG_QUERY_ERROR(query());
            }
        }
    }


}

QVariant ZipCountryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case Zip:
            return QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 0));
        case City:
            return QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 1));
        case Country:
            return QLocale::countryToString(QLocale(QSqlQueryModel::data(QSqlQueryModel::index(index.row(), Country)).toString()).country());
        case ZipCity:
        {
            const QString &zip = QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 0)).toString();
            const QString &city = QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 1)).toString();
            return zip + ", " + city;
        }
        }

    }
    return QVariant();
}

bool ZipCountryModel::countryAvailable(const QLocale::Country country) const
{
    if (!m_DbAvailable) {  // BUG with if (m_DbAvailable && db.isOpen()) --> returning false with (true && true) ????
        if (!db.isOpen()) {
            return false;
        }
    }
    QString req = QString("SELECT DISTINCT COUNT(COUNTRY) FROM ZIPS WHERE `COUNTRY`='%1'")
            .arg(Utils::countryToIso(country).toLower());
    QSqlQuery query(db);
    if (query.exec(req)) {
        if (query.next())
            return query.value(0).toInt();
    } else {
        LOG_QUERY_ERROR(query);
    }
    return false;
}

bool ZipCountryModel::coupleExists(const QString &zip, const QString &city) const
{
    if (!m_DbAvailable) {  // BUG with if (m_DbAvailable && db.isOpen()) --> returning false with (true && true) ????
        if (!db.isOpen()) {
            return false;
        }
    }
    QString req = QString("SELECT COUNT(ZIP) FROM ZIPS WHERE `COUNTRY`='%1' AND `CITY`='%2' AND ZIP='%3'")
            .arg(m_countryIso).arg(city).arg(zip);
    QSqlQuery query(db);
    if (query.exec(req)) {
        if (query.next())
            return query.value(0).toInt();
    } else {
        LOG_QUERY_ERROR(query);
    }
    return false;
}

void ZipCountryModel::setCityFilter(const QString &city)
{
    if (!m_DbAvailable) {  // BUG with if (m_DbAvailable && db.isOpen()) --> returning false with (true && true) ????
        if (!db.isOpen()) {
            return;
        }
    }
    if (m_City==city)
        return;
    m_City=city;
    QString req = QString("SELECT ZIP, CITY FROM ZIPS WHERE `COUNTRY`='%1' AND `CITY` like '%2%' ORDER BY CITY ASC LIMIT 0, 20")
            .arg(m_countryIso).arg(city);
    setQuery(req, db);
    if (!query().isActive()) {
        LOG_QUERY_ERROR(query());
    }
}

void ZipCountryModel::setZipCodeFilter(const QString &zipCode)
{
    if (!m_DbAvailable) {  // BUG with if (m_DbAvailable && db.isOpen()) --> returning false with (true && true) ????
        if (!db.isOpen()) {
            return;
        }
    }
    if (m_Zip==zipCode)
        return;
    m_Zip=zipCode;
    QString req = QString("SELECT ZIP, CITY FROM ZIPS WHERE `COUNTRY`='%1' AND `ZIP` like '%2%' ORDER BY ZIP LIMIT 0, 20")
            .arg(m_countryIso).arg(zipCode);
    setQuery(req, db);
    if (!query().isActive()) {
        LOG_QUERY_ERROR(query());
    }
}

void ZipCountryModel::setCountryIsoFilter(const QString &countryIso)
{
    m_countryIso = countryIso.toLower();
}

// Find the database to use. In priority order:
// - User datapack
// - Application installed datapack
static QString databasePath()
{
    QString dbRelPath = "/zipcodes/zipcodes.db";
    QString tmp;
    tmp = settings()->dataPackInstallPath() + dbRelPath;
    if (QFileInfo(tmp).exists())
        return settings()->dataPackInstallPath();
    tmp = settings()->dataPackApplicationInstalledPath() + dbRelPath;
    return settings()->dataPackApplicationInstalledPath();
}

static QString databaseFileName()
{
    return databasePath() + QDir::separator() + "zipcodes" + QDir::separator() + "zipcodes.db";
}

/**
  \class ZipCountryCompleters
  The ZipCountryCompleters is an object that eases the input of zipcodes and city names. It
  works together with a zipcode database. You just have to set the zipcode and city QLineEdit with
  setZipLineEdit() and setCityLineEdit(). Users will get a completer for the zipcode and the city.
  When the user will select a zipcode or a city name, both line edit will be updated with the selected
  data.
*/
ZipCountryCompleters::ZipCountryCompleters(QObject *parent) :
    QObject(parent), m_cityEdit(0), m_zipEdit(0), m_countryCombo(0), m_Model(0),
    m_ZipButton(0), m_CityButton(0), m_DbAvailable(false)
{
    setObjectName("ZipCountryCompleters");
    createModel();

    // Manage datapacks
    connect(packManager(), SIGNAL(packInstalled(DataPack::Pack)), this, SLOT(packChanged(DataPack::Pack)));
    connect(packManager(), SIGNAL(packRemoved(DataPack::Pack)), this, SLOT(packChanged(DataPack::Pack)));
//    connect(packManager(), SIGNAL(packUpdated(DataPack::Pack)), this, SLOT(packChanged(DataPack::Pack)));
}

ZipCountryCompleters::~ZipCountryCompleters()
{
}

void ZipCountryCompleters::createModel()
{
    QSqlDatabase db;
    if (QSqlDatabase::connectionNames().contains("ZIPS")) {
        db = QSqlDatabase::database("ZIPS");
    } else {
        LOG(QString("Trying to open ZipCode database from %1").arg(databaseFileName()));
        db = QSqlDatabase::addDatabase("QSQLITE", "ZIPS");
        if (QFileInfo(databaseFileName()).exists()) {
            db.setDatabaseName(databaseFileName());
            m_DbAvailable = true;
        } else {
            m_DbAvailable = false;
        }
    }
    if (m_DbAvailable) {
        if (!db.open()) {
            LOG_ERROR("Unable to open Zip database");
            m_DbAvailable = false;
        } else {
            LOG(tkTr(Trans::Constants::CONNECTED_TO_DATABASE_1_DRIVER_2).arg("zipcodes").arg("sqlite"));
        }
    }
    m_Model = new ZipCountryModel(this, db, m_DbAvailable);
}

/** Define the QComboBox to use as country selector. The combo will be automatically populated and its current index will be set to the current QLocale::Country */
void ZipCountryCompleters::setCountryComboBox(Utils::CountryComboBox *box)
{
    // if there was an old countryCombo, remove it's currentIndexChanged connection to here
    if (m_countryCombo)
        disconnect(m_countryCombo, SIGNAL(currentCountryChanged(QLocale::Country)), this, SLOT(setCountryFilter(QLocale::Country)));

    m_countryCombo = box;
    connect(m_countryCombo, SIGNAL(currentCountryChanged(QLocale::Country)), this, SLOT(setCountryFilter(QLocale::Country)));
    setCountryFilter(m_countryCombo->currentCountry());
}

/** Define the QLineEdit to use as city name editor */
void ZipCountryCompleters::setCityLineEdit(Utils::QButtonLineEdit *city)
{
    m_cityEdit = city;
    // Completer
    QCompleter *completer = new QCompleter(this);
    completer->setModel(m_Model);
    completer->setCompletionColumn(ZipCountryModel::City);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
//    m_Completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->popup()->setAlternatingRowColors(true);
    city->setCompleter(completer);
    connect(m_cityEdit, SIGNAL(textChanged(QString)), this, SLOT(cityTextChanged()));
    connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(indexActivated(QModelIndex)));

    // button
    m_CityButton = new QToolButton(m_cityEdit);
    m_CityButton->setIcon(theme()->icon(Core::Constants::ICONOK));
    m_cityEdit->setLeftButton(m_CityButton);
//    m_CityButton->setToolTip("City button");

    m_cityEdit->installEventFilter(this);
}

/** Define the QLineEdit to use as zip code editor */
void ZipCountryCompleters::setZipLineEdit(Utils::QButtonLineEdit *zip)
{
    m_zipEdit = zip;
    // Completer
    QCompleter *completer = new QCompleter(this);
    completer->setModel(m_Model);
    completer->setCompletionColumn(ZipCountryModel::ZipCity);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->popup()->setAlternatingRowColors(true);
    zip->setCompleter(completer);
    connect(m_zipEdit, SIGNAL(textChanged(QString)), this, SLOT(zipTextChanged()));
    connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(indexActivated(QModelIndex)));

    // button
    m_ZipButton = new QToolButton(m_zipEdit);
    m_ZipButton->setIcon(theme()->icon(Core::Constants::ICONOK));
    m_zipEdit->setLeftButton(m_ZipButton);
    m_zipEdit->installEventFilter(this);
}

/** When user select a zip or a city this private slot is activated. It causes the line edit
    to be populated with the selected values.
*/
void ZipCountryCompleters::indexActivated(const QModelIndex &index)
{
    QString zip = m_Model->index(index.row(), ZipCountryModel::Zip).data().toString();
    QString city = m_Model->index(index.row(), ZipCountryModel::City).data().toString();
    if (m_zipEdit) {
        m_zipEdit->clearFocus();
        m_zipEdit->setText(zip);
    }
    if (m_cityEdit) {
        m_cityEdit->clearFocus();
        m_cityEdit->setText(city);
    }
    checkData();
}

void ZipCountryCompleters::setCountryFilter(const QLocale::Country country)
{
    if (!m_countryCombo)
        return;
    if (!m_Model)
        return;
    m_Model->setCountryIsoFilter(Utils::countryToIso(country));
    checkData();
}

void ZipCountryCompleters::zipTextChanged()
{
    m_Model->setZipCodeFilter(m_zipEdit->completer()->completionPrefix());
}

void ZipCountryCompleters::cityTextChanged()
{
    m_Model->setCityFilter(m_cityEdit->completer()->completionPrefix());
}

/** Private slot used to check the validity of the country/city/zipcode association. */
void ZipCountryCompleters::checkData()
{
    if (!m_zipEdit || !m_cityEdit) {
        return;
    }
    // country should never be empty
    if (!m_countryCombo) {
        m_ZipButton->setIcon(theme()->icon(Core::Constants::ICONCRITICAL));
        m_CityButton->setIcon(theme()->icon(Core::Constants::ICONCRITICAL));
        m_ZipButton->setToolTip(tr("No country selected"));
        m_CityButton->setToolTip(tr("No country selected"));
        return;
    }
    // country zipcodes available
    if (!m_Model->countryAvailable(m_countryCombo->currentCountry())) {
        m_ZipButton->setIcon(theme()->icon(Core::Constants::ICONHELP));
        m_CityButton->setIcon(theme()->icon(Core::Constants::ICONHELP));
        //: %1 is a country
        m_ZipButton->setToolTip(tr("Autocompletion of zipcodes for %1 not available").
                                arg(QLocale::countryToString(m_countryCombo->currentCountry())));
        //: %1 is a country
        m_CityButton->setToolTip(tr("Autocompletion of cities for %1 not available").
                                 arg(QLocale::countryToString(m_countryCombo->currentCountry())));
        return;
    }

    // zip/city empty -> can be calculated ?
    // zip && city not empty -> check the couple with the model
    if (!m_zipEdit->text().isEmpty() && !m_cityEdit->text().isEmpty()) {
        if (m_Model->coupleExists(m_zipEdit->text(), m_cityEdit->text())) {
            m_ZipButton->setIcon(theme()->icon(Core::Constants::ICONOK));
            m_CityButton->setIcon(theme()->icon(Core::Constants::ICONOK));
            m_ZipButton->setToolTip(tr("Zip/city/country association checked"));
            m_CityButton->setToolTip(tr("Zip/city/country association checked"));
        } else {
            m_ZipButton->setIcon(theme()->icon(Core::Constants::ICONWARNING));
            m_CityButton->setIcon(theme()->icon(Core::Constants::ICONWARNING));
            m_ZipButton->setToolTip(tr("Wrong zip/city/country association"));
            m_CityButton->setToolTip(tr("Wrong zip/city/country association"));
        }
    }
}

/** Refresh database if a Zip code data pack is installed/removed */
void ZipCountryCompleters::packChanged(const DataPack::Pack &pack)
{
    if (pack.dataType() == DataPack::Pack::ZipCodes) {
        delete m_Model;
        m_Model = 0;
        QSqlDatabase::removeDatabase("ZIPS");
        createModel();
        m_cityEdit->completer()->setModel(m_Model);
        m_zipEdit->completer()->setModel(m_Model);
        m_Model->setCountryIsoFilter(m_countryCombo->currentIsoCountry());
        checkData();
    }
}

/** \deprecated Event Filter is used to draw toolButtons inside the QLineEdit. */
bool ZipCountryCompleters::eventFilter(QObject *o, QEvent *e)
{
    if (o==m_zipEdit) {
        if (e->type()==QEvent::Resize) {
            m_zipEdit->event(e);
            QSize sz = m_ZipButton->sizeHint();
            int frameWidth = m_zipEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            m_ZipButton->move(m_zipEdit->rect().left() + frameWidth ,
                              (m_zipEdit->rect().bottom() + 1 - sz.height()) / 2);
        }
    } else if (o==m_cityEdit) {
        if (e->type()==QEvent::Resize) {
            m_cityEdit->event(e);
            QSize sz = m_CityButton->sizeHint();
            int frameWidth = m_cityEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            m_CityButton->move(m_cityEdit->rect().left() + frameWidth ,
                              (m_cityEdit->rect().bottom() + 1 - sz.height()) / 2);
        }
    }
    return false;
}
