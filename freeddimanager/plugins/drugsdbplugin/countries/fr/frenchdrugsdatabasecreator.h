/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2014 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H
#define DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H

#include <drugsdbplugin/idrugdatabase.h>
#include <QStringList>

/**
 * \file frenchdrugsdatabasecreator.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 11 Jan 2014
*/

namespace DrugsDb {
namespace Internal {

class FrDrugDatabase : public DrugsDb::Internal::IDrugDatabase
{
    Q_OBJECT

public:
    FrDrugDatabase(QObject *parent = 0);
    ~FrDrugDatabase();

    QString id() const {return "FrDrugDatabase";}
    void setLicenseType(LicenseType type);

    bool process();
    QString processMessage() const;

    bool prepareData();
    bool populateDatabase();
//    bool linkDrugsComponentsAndDrugInteractors();

    QStringList errors() const {return m_Errors;}

private:
    QStringList m_Errors;
    bool m_WithProgress;
};

}  //  namespace Internal
}  //  namespace DrugsDb

#endif // DDIMANAGER_DRUGSDB_INTERNAL_FRENCHDRUGSDATABASECREATOR_H
