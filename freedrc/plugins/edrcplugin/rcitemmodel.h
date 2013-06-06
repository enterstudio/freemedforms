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
 *  This program is distributed in the hope that it will be useful, *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main developer: Eric MAEKER, <eric.maeker@gmail.com>                   *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef EDRC_INTERNAL_RCITEMMODEL_H
#define EDRC_INTERNAL_RCITEMMODEL_H

#include <QSqlTableModel>

namespace eDRC {
namespace Internal {

class RCItemModel : public QSqlTableModel
{
    Q_OBJECT

public:
    enum Datarepresentation {
        Id = 0,
        Label
    };

    RCItemModel(QObject *parent = 0);
    ~RCItemModel();

    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setFilterOnRcId(const int rcId);

private:
//    void selectAllParents(const QModelIndex& index);
//    void deselectAllChilds(const QModelIndex& index);

//    QList<Criteres_Elements>    * m_pListCriteres;
};

} // namespace eDRC
} // namespace Internal

#endif  // EDRC_INTERNAL_RCITEMMODEL_H