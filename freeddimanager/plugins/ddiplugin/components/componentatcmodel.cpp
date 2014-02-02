/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
/**
 * \class DDI::ComponentAtcModel
 * This model holds information about drug's component and
 * their link to any ATC code(s). \n
 * This model is not in manual submit but this should be configurable.
*/

#include "componentatcmodel.h"
#include <ddiplugin/ddicore.h>
#include <ddiplugin/constants.h>
#include <ddiplugin/atc/atctablemodel.h>
#include <ddiplugin/database/ddidatabase.h>
#include <ddiplugin/components/componentlinkerdata.h>

#include <coreplugin/icore.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/isettings.h>

#include <utils/log.h>
#include <utils/global.h>

#include <QColor>
#include <QIcon>
#include <QSqlTableModel>
#include <QHash>

using namespace DDI;
using namespace Internal;

static inline Core::IMainWindow *mainwindow() {return Core::ICore::instance()->mainWindow();}
static inline Core::ISettings *settings()  { return Core::ICore::instance()->settings(); }
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline DDI::DDICore *ddiCore() {return DDI::DDICore::instance();}
static inline DDI::Internal::DDIDatabase &ddiBase()  { return DDI::DDICore::instance()->database(); }

namespace DDI {
namespace Internal {
class ComponentAtcModelPrivate
{
public:
    ComponentAtcModelPrivate(ComponentAtcModel *parent) :
        _sql(0),
        _rows(0),
        q(parent)
    {
    }

    ~ComponentAtcModelPrivate()
    {}

    void createSqlModel()
    {
        _sql = new QSqlTableModel(q, ddiBase().database());
        _sql->setTable(ddiBase().table(Constants::Table_COMPONENTS));
        // _sql->setEditStrategy(QSqlTableModel::OnManualSubmit);

        // QObject::connect(_sql, SIGNAL(primeInsert(int,QSqlRecord&)), q, SLOT(populateNewRowWithDefault(int, QSqlRecord&)));
        QObject::connect(_sql, SIGNAL(layoutAboutToBeChanged()), q, SIGNAL(layoutAboutToBeChanged()));
        QObject::connect(_sql, SIGNAL(layoutChanged()), q, SIGNAL(layoutChanged()));
        QObject::connect(_sql, SIGNAL(modelAboutToBeReset()), q, SIGNAL(modelAboutToBeReset()));
        QObject::connect(_sql, SIGNAL(modelReset()), q, SIGNAL(modelReset()));
    }

    int modelColumnToSqlColumn(const int modelColumn)
    {
        int sql = -1;
        switch (modelColumn) {
        case ComponentAtcModel::Id: sql = Constants::COMPO_ID; break;
        case ComponentAtcModel::Uid: sql = Constants::COMPO_UID; break;
        case ComponentAtcModel::DrugDatabaseComponentUid1: sql = Constants::COMPO_DRUGDB_UID1; break;
        case ComponentAtcModel::DrugDatabaseComponentUid2: sql = Constants::COMPO_DRUGDB_UID2; break;
        case ComponentAtcModel::IsValid: sql = Constants::COMPO_ISVALID; break;
        case ComponentAtcModel::IsReviewed: sql = Constants::COMPO_ISREVIEWED; break;
        case ComponentAtcModel::Name: sql = Constants::COMPO_LABEL; break;
        case ComponentAtcModel::AtcCodeList: sql = Constants::COMPO_ATCCODES; break;
        case ComponentAtcModel::SuggestedAtcCodeList: sql = Constants::COMPO_SUGGESTED; break;
        case ComponentAtcModel::Reviewer: sql = Constants::COMPO_REVIEWERS; break;
        case ComponentAtcModel::DateCreation: sql = Constants::COMPO_DATECREATE; break;
        case ComponentAtcModel::DateUpdate: sql = Constants::COMPO_DATEUPDATE; break;
        case ComponentAtcModel::Comments: sql = Constants::COMPO_COMMENT; break;
        };
        return sql;
    }

public:
    QSqlTableModel *_sql;
    QString _reviewer, _actualDbUid;
    int _rows;
    QString _drugsDbFilter;

private:
    ComponentAtcModel *q;
};

}  // namespace Internal
}  // namespace DDI

ComponentAtcModel::ComponentAtcModel(QObject *parent) :
    QAbstractTableModel(parent),
    d(new Internal::ComponentAtcModelPrivate(this))
{
    setObjectName("ComponentAtcModel");
    d->createSqlModel();
}

ComponentAtcModel::~ComponentAtcModel()
{
    if (d)
        delete d;
    d = 0;
}

bool ComponentAtcModel::initialize()
{
    // Fetch all row from the sql model
    d->_sql->select();
    while (d->_sql->canFetchMore(index(d->_sql->rowCount(), 0)))
        d->_sql->fetchMore(index(d->_sql->rowCount(), 0));
    return true;
}

bool ComponentAtcModel::onDdiDatabaseChanged()
{
    delete d->_sql;
    d->_sql = 0;
    d->createSqlModel();
    return initialize();
}

/** Return all available Drug database Uid from the database */
QStringList ComponentAtcModel::availableDrugsDatabases() const
{
    return ddiBase().availableComponentDrugsDatabaseUids();
}

/** Filter data according to the drug database uid \e dbUid1 and \e dbUid2 */
bool ComponentAtcModel::selectDatabase(const QString &dbUid1, const QString &dbUid2)
{
//    if (dbUid == d->_drugsDbFilter)
//        return true;
//    qWarning() << "ComponentAtcModel::selectDatabase" << dbUid;
    beginResetModel();
    QHash<int, QString> where;
    where.insert(Constants::COMPO_DRUGDB_UID1, QString("='%1'").arg(dbUid1));
    if (!dbUid2.isEmpty())
        where.insert(Constants::COMPO_DRUGDB_UID2, QString("='%1'").arg(dbUid2));
    QString filter = ddiBase().getWhereClause(Constants::Table_COMPONENTS, where);
    d->_sql->setFilter(filter);
    initialize();
    endResetModel();
    return true;
}

/** Define the reviewer \e name to use */
void ComponentAtcModel::setActualReviewer(const QString &name)
{
    d->_reviewer = name;
}

int ComponentAtcModel::rowCount(const QModelIndex &parent) const
{
    return d->_sql->rowCount(parent);
}

int ComponentAtcModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QVariant ComponentAtcModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        QModelIndex sqlIndex = d->_sql->index(index.row(), d->modelColumnToSqlColumn(index.column()));
        switch (index.column()) {
        case IsReviewed: return d->_sql->data(sqlIndex).toInt()==1?"Reviewed":"Unreviewed";
        case IsValid: return d->_sql->data(sqlIndex).toInt()==1?"Valid":"Invalid";
        default: break;
        }
        return d->_sql->data(sqlIndex, role);
    } else if (role == Qt::EditRole) {
        QModelIndex sqlIndex = d->_sql->index(index.row(), d->modelColumnToSqlColumn(index.column()));
        return d->_sql->data(sqlIndex, role);
    } else if (role == Qt::CheckStateRole) {
        int sql = -1;
        switch (index.column()) {
        case IsValid: sql = Constants::COMPO_ISVALID; break;
        case IsReviewed: sql = Constants::COMPO_ISREVIEWED; break;
        default: sql = -1; break;
        }
        if (sql != -1) {
            QModelIndex sqlIndex = d->_sql->index(index.row(), sql);
            // using displayrole
            return d->_sql->data(sqlIndex).toBool()?Qt::Checked:Qt::Unchecked;
        }
    } else if (role == Qt::ToolTipRole) {
        QStringList atc = d->_sql->index(index.row(), Constants::COMPO_ATCCODES).data().toString().split(";", QString::SkipEmptyParts);
        QStringList atcFull;
        if (!atc.isEmpty()) {
            foreach(const QString &c, atc)
                atcFull << QString("%1 - %2").arg(c).arg(ddiBase().atcLabelForCode(c));
        }
        atc = d->_sql->index(index.row(), Constants::COMPO_SUGGESTED).data().toString().split(";", QString::SkipEmptyParts);
        QStringList suggestedFull;
        if (!atc.isEmpty()) {
            foreach(const QString &c, atc)
                suggestedFull << QString ("%1 - %2").arg(c).arg(ddiBase().atcLabelForCode(c));
        }

        QStringList lines;
        lines << QString("<b>%1</b><br>    %2")
                 .arg(d->_sql->index(index.row(), Constants::COMPO_LABEL).data().toString())
                 .arg(d->_sql->index(index.row(), Constants::COMPO_ISREVIEWED).data().toBool()?"Reviewed":"Unreviewed")
                 .replace(" ", "&nbsp;");
        if (!atcFull.isEmpty()) {
            lines << QString("<u>%1</u>").arg(tr("Linked ATC codes:"));
            lines << QString("&nbsp;&nbsp;%1").arg(atcFull.join("<br>&nbsp;&nbsp;"));
        }
        if (!suggestedFull.isEmpty()) {
            lines << QString("<u>%1</u>").arg(tr("Suggested ATC codes:"));
            lines << QString("&nbsp;&nbsp;%1").arg(suggestedFull.join("<br>&nbsp;&nbsp;"));
        }
        return lines.join("<br>");
    }
    return QVariant();
}

bool ComponentAtcModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    int sql = d->modelColumnToSqlColumn(index.column());
    if (role == Qt::EditRole) {
        bool ok = false;
        QModelIndex sqlIndex = d->_sql->index(index.row(), sql);

        switch (index.column()) {
        case IsValid:
        case IsReviewed:
            ok = d->_sql->setData(sqlIndex, value.toBool()?1:0, role);
            break;
        default: ok = d->_sql->setData(sqlIndex, value, role); break;
        }

        // set the date update
        if (ok) {
            Q_EMIT dataChanged(index, index);
            sqlIndex = d->_sql->index(index.row(), Constants::COMPO_DATEUPDATE);
            ok = d->_sql->setData(sqlIndex, QDateTime::currentDateTime(), role);
            if (ok) {
                QModelIndex idx = this->index(index.row(), DateUpdate);
                Q_EMIT dataChanged(idx, idx);
            }
        }

        return ok;
    }
    return false;
}

Qt::ItemFlags ComponentAtcModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    switch (index.column()) {
    case Name:
    case DateUpdate:
    case DateCreation:
    case FancyButton:
        return f;
    case IsReviewed:
    case IsValid:
        f |= Qt::ItemIsUserCheckable;
        break;
    default:
        f |= Qt::ItemIsEditable;
    }

    return f;
}

QVariant ComponentAtcModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case Id:
            return "Id";
        case Uid:
            return "Uid";
        case DrugDatabaseComponentUid1:
            return tr("Drugs database Uid1");
        case DrugDatabaseComponentUid2:
            return tr("Drugs database Uid2");
        case Name:
            return tr("Component name");
        case AtcCodeList:
            return tr("ATC code");
        case SuggestedAtcCodeList:
            return tr("Suggested ATC code");
        case IsValid:
            return tr("Validity");
        case IsReviewed:
            return tr("Reviewed");
        case Reviewer:
            return tr("Reviewers");
        case Comments:
            return tr("Comments");
        case DateCreation:
            return tr("Date of creation");
        case DateUpdate:
            return tr("Date of update");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool ComponentAtcModel::addUnreviewedMolecules(const QString &dbUid, const QStringList &molecules)
{
//    QDomElement el = d->m_RootNode.firstChildElement("Database");
//    while (!el.isNull()) {
//        if (el.attribute("uid").compare(dbUid, Qt::CaseInsensitive) == 0) {
//            break;
//        }
//        el = el.nextSiblingElement("Database");
//    }

//    if (el.isNull())
//        return false;

//    selectDatabase(dbUid);

//    QStringList known;
//    for(int i=0; i < rowCount(); ++i) {
//        known << index(i, MoleculeName).data().toString();
//    }
//    known.removeDuplicates();
//    known.removeAll("");

//    foreach(const QString &mol, molecules) {
//        if (mol.isEmpty())
//            continue;
//        if (known.contains(mol, Qt::CaseInsensitive))
//            continue;
//        QDomElement newmol = d->domDocument.createElement("Molecule");
//        newmol.setAttribute("name", mol);
//        newmol.setAttribute("AtcCode", QString());
//        newmol.setAttribute("review", "false");
//        newmol.setAttribute("reviewer", QString());
//        newmol.setAttribute("references", QString());
//        newmol.setAttribute("comment", QString());
//        newmol.setAttribute("dateofreview", QString());
//        el.appendChild(newmol);
//        known << mol;
//    }

//    reset();
    return true;
}

bool ComponentAtcModel::addAutoFoundMolecules(const QMultiHash<QString, QString> &mol_atc, bool removeFromModel)
{
//    int nb = 0;
//    foreach(const QString &mol, mol_atc.keys()) {
//        QDomNode n = d->m_RootItem->node().firstChild();

//         while (!n.isNull()) {
//             QDomNamedNodeMap attributeMap = n.attributes();
//             if (attributeMap.namedItem("name").nodeValue() == mol) {
//                 if (removeFromModel) {
//                     const QStringList &list = mol_atc.values(mol);
//                     n.toElement().setAttribute("autofound", list.join(","));
//                     QDomNode rem = d->m_RootItem->node().removeChild(n);
//                 } else {
//                     const QStringList &list = mol_atc.values(mol);
//                     n.toElement().setAttribute("autofound", list.join(","));
//                 }
//                 ++nb;
//                 break;
//             }
//             n = n.nextSibling();
//         }
//    }
    return true;
}


struct MolLink {
    int lk_nature;
    int mol;
    QString mol_form;
};

/**
 * Create the link between drugs components and drug interactors / ATC codes. \n
 * Use the DDI::ComponentLinkerData to give and take data.
 * \sa DDI::ComponentLinkerData
 */
ComponentLinkerResult &ComponentAtcModel::startComponentLinkage(const ComponentLinkerData &data)
{
    ComponentLinkerResult *result = new ComponentLinkerResult;
    if (!ddiBase().database().isOpen()) {
        result->addErrorMessage("Can not connect to DDI::DDIDatabase");
        LOG_ERROR("Can not connect to DDI::DDIDatabase");
        return *result;
    }

    // Get all ATC Code and Label from the DDI database
    // All labels and codes are upper case
    LOG("Getting ATC Informations from the interactions database");
    QList<int> fields;
    fields << Constants::ATC_CODE;
    if (data.lang.compare("fr", Qt::CaseInsensitive) == 0)
        fields << Constants::ATC_FR;
    else if (data.lang.compare("en", Qt::CaseInsensitive) == 0)
        fields << Constants::ATC_EN;
    else if (data.lang.compare("de", Qt::CaseInsensitive) == 0)
        fields << Constants::ATC_DE;
    else if (data.lang.compare("sp", Qt::CaseInsensitive) == 0)
        fields << Constants::ATC_SP;

    QHash<QString, QString> atcCodeToName;
    QString req = ddiBase().select(Constants::Table_ATC, fields);
    QSqlQuery query(ddiBase().database());
    if (query.exec(req)) {
        while (query.next())
            atcCodeToName.insert(query.value(0).toString().toUpper(), query.value(1).toString().toUpper());
    } else {
        LOG_QUERY_ERROR(query);
        query.finish();
        return *result;
    }
    query.finish();
    qWarning() << "ATC" << atcCodeToName.count();

    qWarning() << "Number of distinct molecules" << data.compoIds.uniqueKeys().count();
    const QStringList &knownComponentNames = data.compoIds.uniqueKeys();

    // Manage corrected components link by ATC label
    int corrected = 0;
    foreach(const QString &componentLbl, data.correctedByName.keys()) {
        QString lbl = componentLbl.toUpper();
        if (!knownComponentNames.contains(lbl))
            continue;
        // Get ATC id from the ATC name
        int componentId = data.compoIds.value(lbl);
        const QStringList &atcCodes = atcCodeToName.keys(data.correctedByName.value(lbl));
        foreach(const QString &atcCode, atcCodes) {
            int atcId = data.atcCodeIds.value(atcCode);
            corrected++;
            result->addComponentToAtcLink(componentId, atcId);
        }
    }

    // Manage corrected components link by ATC codes
    foreach(const QString &componentLbl, data.correctedByAtcCode.uniqueKeys()) {  // Key=componentLbl, Val=ATC
        if (!knownComponentNames.contains(componentLbl))
            continue;
        int componentId = data.compoIds.value(componentLbl);
        const QStringList &atcCodes = data.correctedByAtcCode.values(componentLbl);
        foreach(const QString &atcCode, atcCodes) {
            int atcId = data.atcCodeIds.value(atcCode);
            corrected++;
            result->addComponentToAtcLink(componentId, atcId);
        }
    }

    LOG(QString("Hand made association: %1").arg(corrected));
    result->addMessage(QString("Hand made association: %1").arg(corrected));

    // Find component / ATC links
    int autoFound = 0;
    foreach(const QString &componentLbl, knownComponentNames) {
        if (componentLbl.isEmpty())
            continue;

        foreach(const int componentId, data.compoIds.values(componentLbl)) {
            // Already processed?
            if (result->containsComponentId(componentId))
                continue;

            // Does component name exact-matches an ATC label
            QStringList atcCodes = atcCodeToName.keys(componentLbl);
            QString transformedLbl = componentLbl;
            if (atcCodes.isEmpty()) {
                // Try to find the ATC label using component name transformation
                // remove accents
                transformedLbl = Utils::removeAccents(transformedLbl);
                atcCodes = atcCodeToName.keys(transformedLbl);
                if (atcCodes.count() > 0) {
                    result->addMessage(QString("Found component link after label transformation: %1 - %2").arg("remove accents").arg(componentLbl));
                    LOG(QString("Found component link after label transformation: %1 - %2").arg("remove accents").arg(componentLbl));
                }

                // Not found try some transformations
                // remove '(*)'
                if ((atcCodes.isEmpty()) && (componentLbl.contains("("))) {
                    transformedLbl = componentLbl.left(componentLbl.indexOf("(")).simplified();
                    atcCodes = atcCodeToName.keys(transformedLbl);
                    if (atcCodes.count() > 0) {
                        result->addMessage(QString("Found component link after label transformation: %1 - %2").arg("remove (*)").arg(componentLbl));
                        LOG(QString("Found component link after label transformation: %1 - %2").arg("remove (*)").arg(componentLbl));
                    } else {
                        // Try without accents
                        transformedLbl = Utils::removeAccents(transformedLbl);
                        atcCodes = atcCodeToName.keys(transformedLbl);
                        if (atcCodes.count() > 0) {
                            result->addMessage(QString("Found component link after label transformation: %1 - %2").arg("remove (*) & accents").arg(componentLbl));
                            LOG(QString("Found component link after label transformation: %1 - %2").arg("remove (*) & accents").arg(componentLbl));
                        }
                    }
                }

                // still not found
                if (atcCodes.isEmpty()) {
                    // remove last word :
                    // CITRATE, DIHYDRATE, SODIUM, HYDROCHLORIDE,
                    // POLISTIREX, BROMIDE, MONOHYDRATE, CHLORIDE, CARBAMATE
                    // INDANYL SODIUM, ULTRAMICROCRYSTALLINE, TROMETHAMINE
                    transformedLbl = componentLbl.left(componentLbl.lastIndexOf(" ")).simplified();
                    atcCodes = atcCodeToName.keys(transformedLbl);
                    if (atcCodes.count() > 0) {
                        result->addMessage(QString("Found component link after label transformation: %1 - %2").arg("remove last word").arg(componentLbl));
                        LOG(QString("Found component link after label transformation: %1 - %2").arg("remove last word").arg(componentLbl));
                    } else {
                        // Try without accents
                        transformedLbl = Utils::removeAccents(transformedLbl);
                        atcCodes = atcCodeToName.keys(transformedLbl);
                        if (atcCodes.count() > 0) {
                            result->addMessage(QString("Found component link after label transformation: %1 - %2").arg("remove last word & accents").arg(componentLbl));
                            LOG(QString("Found component link after label transformation: %1 - %2").arg("remove last word & accents").arg(componentLbl));
                        }
                    }
                }

                // Still not found
                if (atcCodes.isEmpty()) {
                    // remove first words : CHLORHYDRATE DE, ACETATE DE
                    QStringList toRemove;
                    toRemove << "CHLORHYDRATE DE" << "CHLORHYDRATE D'" << "ACETATE DE" << "ACETATE D'";
                    transformedLbl = Utils::removeAccents(componentLbl);
                    foreach(const QString &prefix, toRemove) {
                        if (transformedLbl.startsWith(prefix)) {
                            QString tmp = transformedLbl;
                            tmp.remove(prefix);
                            tmp = tmp.simplified();
                            atcCodes = atcCodeToName.keys(tmp);
                            if (atcCodes.count() > 0) {
                                result->addMessage(QString("Found component link after label transformation: %1 - %2").arg(QString("removed %1").arg(prefix)).arg(componentLbl));
                                LOG(QString("Found component link after label transformation: %1 - %2").arg(QString("removed %1").arg(prefix)).arg(componentLbl));
                                break;
                            }
                        }
                    }
                }

                // Still not found
                if (atcCodes.isEmpty()) {
                    // Manage names like XXXXX (DINITRATE D') -> DINITRATE D'XXXXX
                    QStringList change;
                    change << "DINITRATE D'" << "DINITRATE DE" << "NITRATE D'" << "NITRATE DE";
                    transformedLbl = Utils::removeAccents(componentLbl);
                    foreach(const QString &prefix, change) {
                        if (transformedLbl.contains(QString("(%1)").arg(prefix))) {
                            QString tmp = transformedLbl;
                            tmp = tmp.remove(QString("(%1)").arg(prefix));
                            tmp += prefix;
                            atcCodes = atcCodeToName.keys(tmp);
                            if (atcCodes.count() > 0) {
                                result->addMessage(QString("Found component link after label transformation: %1 - %2").arg(QString("moved %1").arg(prefix)).arg(componentLbl));
                                LOG(QString("Found component link after label transformation: %1 - %2").arg(QString("moved %1").arg(prefix)).arg(componentLbl));
                                break;
                            }
                        }
                    }
                }
            }

            // Now we checked all possibilities, check if we found something
            if (atcCodes.isEmpty()) {
                // Nothing -> add the component id to the unmatched list
                result->addUnfoundComponent(componentLbl);
            } else {
                // transformation string matches to database id link
                foreach(const QString &atcCode, atcCodes) {
                    int atcId = data.atcCodeIds.value(atcCode);
                    autoFound++;
                    result->addComponentToAtcLink(componentId, atcId);
                }
            }
        }
    }
    LOG(QString("Automatic association: %1").arg(autoFound));
    result->addMessage(QString("Automatic association: %1").arg(autoFound));

//    // Inform model of founded links
//    QMultiHash<QString, QString> mol_atc_forModel;
//    QHashIterator<int,int> it(data->moleculeIdToAtcId);
//    while (it.hasNext()) {
//        it.next();
//        mol_atc_forModel.insertMulti(mols.key(it.key()), atc_id.key(it.value()));
//    }
//    addAutoFoundMolecules(mol_atc_forModel);
//    mol_atc_forModel.clear();

//    // Try to find molecules in the ComponentAtcModel
//    QHash<QString, QString> model_mol_atc;
//    int modelFound = 0;
//    int reviewedWithoutAtcLink = 0;
//    selectDatabase(data->drugDbUid);
//    while (canFetchMore(QModelIndex()))
//        fetchMore(QModelIndex());

//    for(int i=0; i< rowCount(); ++i) {
//        if (index(i, ComponentAtcModel::Review).data().toString() == "true") {
//            model_mol_atc.insert(index(i, ComponentAtcModel::MoleculeName).data().toString(),
//                                 index(i, ComponentAtcModel::ATC_Code).data().toString());
//        }
//    }
//    qWarning() << "ComponentAtcModel::AvailableLinks" << model_mol_atc.count();

//    foreach(const QString &mol, data->unfoundMoleculeAssociations) {
//        if (model_mol_atc.keys().contains(mol)) {
//            int codeMol = mols.value(mol);
//            if (!data->moleculeIdToAtcId.keys().contains(codeMol)) {
//                if (model_mol_atc.value(mol).trimmed().isEmpty()) {
//                    ++reviewedWithoutAtcLink;
//                    continue;
//                }
//                QStringList atcCodes = model_mol_atc.value(mol).split(",");
//                foreach(const QString &atcCode, atcCodes) {
//                    QString atcName = atcName_id.key(atc_id.value(atcCode));
//                    QList<int> atcIds = atcName_id.values(atcName);
//                    foreach(int id, atcIds) {
//                        data->moleculeIdToAtcId.insertMulti(codeMol, id);
//                        qWarning() << "ModelLinker Found" << codeMol << mol << id << atcName_id.key(id);
//                        data->unfoundMoleculeAssociations.removeAll(mol);
//                    }
//                    if (atcIds.count())
//                        ++modelFound;
//                }
//            }
//        }
//    }

//    // TODO: Try to find new associations via the COMPOSITION.LK_NATURE field
//    int natureLinkerNb = 0;
////    if (drugsDbUid == "FR_AFSSAPS") {
////        // TODO: code here */
////        QMap<int, QMultiHash<int, int> > cis_codeMol_lk;
////        QMap<int, QVector<MolLink> > cis_compo;
////        {
////            //            QString req = "SELECT `DID`, `MID`, `LK_NATURE` FROM `COMPOSITION` ORDER BY `DID`";
////            QString req = database->select(Table_COMPO, QList<int>()
////                                                             << COMPO_DID
////                                                             << COMPO_MID
////                                                             << COMPO_LK_NATURE
////                                                             );
////            req += QString(" ORDER BY `%1`").arg(database->fieldName(Table_COMPO, COMPO_DID));
////            if (query.exec(req)) {
////                while (query.next()) {
////                    QVector<MolLink> &receiver = cis_compo[query.value(0).toInt()];
////                    MolLink lk;
////                    lk.mol = query.value(1).toInt();
////                    lk.lk_nature = query.value(2).toInt();
////                    //                    lk.mol_form = query.value(3).toString();
////                    receiver.append(lk);
////                }
////            } else {
////                LOG_QUERY_ERROR(query);
////            }
////            QMutableMapIterator<int, QVector<MolLink> > i(cis_compo);
////            while (i.hasNext()) {
////                i.next();
////                const QVector<MolLink> &links = i.value();
////                QMultiHash<int, int> lk_mol;
////                QMultiHash<int, QString> lk_form;
////                foreach(const MolLink &lk, links) {
////                    lk_mol.insertMulti(lk.lk_nature, lk.mol);
////                    lk_form.insertMulti(lk.lk_nature, lk.mol_form);
////                }
////                foreach(int key, lk_mol.uniqueKeys()) {
////                    QStringList forms = lk_form.values(key);
////                    const QList<int> &mols = lk_mol.values(key);
////                    forms.removeDuplicates();
////                    if (forms.count()==1 && mols.count()==2) {
////                        // link molecules
////                        QMultiHash<int, int> &code_lk = cis_codeMol_lk[i.key()];
////                        code_lk.insertMulti(key, mols.at(0));
////                        code_lk.insertMulti(key, mols.at(1));
////                    }
////                }
////            }
////        }
////        // Computation
////        int nb;
////        do
////        {
////            nb=0;
////            QMutableMapIterator<int, QMultiHash<int, int> > i (cis_codeMol_lk);
////            while (i.hasNext()) {
////                i.next();
////                const QMultiHash<int, int> lk_codemol = i.value();
////                // for all molecule_codes
////                foreach(int lk, lk_codemol.uniqueKeys()) {
////                    if (lk_codemol.count(lk) == 2) {
////                        // Ok for computation
////                        int one, two;
////                        one = lk_codemol.values(lk).at(0);
////                        two = lk_codemol.values(lk).at(1);

////                        // if both molecule_codes are known or unknown --> continue
////                        if ((!data->moleculeIdToAtcId.keys().contains(one)) &&
////                                (!data->moleculeIdToAtcId.keys().contains(two)))
////                            continue;
////                        if ((data->moleculeIdToAtcId.keys().contains(one)) &&
////                                (data->moleculeIdToAtcId.keys().contains(two)))
////                            continue;

////                        // Associate unknown molecule_code with the known ATC
////                        if (data->moleculeIdToAtcId.keys().contains(one)) {
////                            // The second molecule is unknown
////                            const QList<int> &atcIds = data->moleculeIdToAtcId.values(one);
////                            foreach(int actId, atcIds) {
////                                data->moleculeIdToAtcId.insertMulti(two, actId);
////                                qWarning() << "LK_NATURE: Linked" << i.key() << mols.key(one) << mols.key(two) << lk << atcName_id.key(actId);
////                                data->unfoundMoleculeAssociations.removeAll(mols.key(two));
////                                data->moleculeIdToAtcId_forModel.insertMulti(mols.key(one), atcName_id.key(actId));
////                            }
////                        } else if (data->moleculeIdToAtcId.keys().contains(two)) {
////                            // The first is unknown
////                            const QList<int> &atcIds = data->moleculeIdToAtcId.values(two);
////                            foreach(int actId, atcIds) {
////                                data->moleculeIdToAtcId.insertMulti(one, actId);
////                                qWarning() << "LK_NATURE: Linked" << i.key() << mols.key(one) << mols.key(two) << lk << atcName_id.key(actId);
////                                data->unfoundMoleculeAssociations.removeAll(mols.key(one));
////                                data->moleculeIdToAtcId_forModel.insertMulti(mols.key(one), atcName_id.key(actId));
////                            }
////                        }
////                        ++nb;
////                        ++natureLinkerNb;
////                    }
////                }
////            }
////            LOG(QString("Link composition by nature: %1 total associations founded.").arg(nb));
////        }
////        while (nb > 0);
////    }

//    // Save completion percent in drugs database INFORMATION table
//    data->completionPercentage = ((double) (1.0 - ((double)(data->unfoundMoleculeAssociations.count() - reviewedWithoutAtcLink) / (double)knownComponentNames.count())) * 100.00);
//    LOG(QString("Molecule links completion: %1").arg(data->completionPercentage));
//    //    DrugsDB::Tools::executeSqlQuery(QString("UPDATE SOURCES SET MOL_LINK_COMPLETION=%1 WHERE SID=%2")
//    //                                 .arg(completion).arg(sid),
//    //                                 Core::Constants::MASTER_DATABASE_NAME, __FILE__, __LINE__);
//    QHash<int, QString> where;
//    where.insert(SOURCES_SID, QString("='%1'").arg(sid));
//    query.prepare(data->database->prepareUpdateQuery(Table_SOURCES, SOURCES_COMPLETION, where));
//    query.bindValue(0, data->completionPercentage);
//    if (!query.exec()) {
//        LOG_QUERY_ERROR_FOR("Tools", query);
//        return *result;
//    }
//    query.finish();

//    // Inform model of founded links
//    addAutoFoundMolecules(mol_atc_forModel, true);
//    mol_atc_forModel.clear();

//    qWarning()
//            << "\nNUMBER OF MOLECULES" << knownComponentNames.count()
//            << "\nCORRECTED BY NAME" << data->correctedByName.keys().count()
//            << "\nCORRECTED BY ATC" << data->correctedByAtcCode.uniqueKeys().count()
//            << "\nFOUNDED" << data->moleculeIdToAtcId.uniqueKeys().count()
//            << QString("\nLINKERMODEL (WithATC:%1;WithoutATC:%2) %3").arg(modelFound).arg(reviewedWithoutAtcLink).arg(modelFound + reviewedWithoutAtcLink)
//            << "\nLINKERNATURE" << natureLinkerNb
//            << "\nLEFT" << (data->unfoundMoleculeAssociations.count() - reviewedWithoutAtcLink)
//            << "\nCONFIDENCE INDICE" << data->completionPercentage
//            << "\n\n";

    return *result;
}

int ComponentAtcModel::removeUnreviewedMolecules()
{
//    int nb = 0;
//    int examined = 0;
//    QDomElement n = d->m_RootItem->node().firstChildElement();

//     while (!n.isNull()) {
//         if (n.attribute("review").compare("true", Qt::CaseInsensitive) != 0) {
//             QDomElement save = n.nextSiblingElement();
//             QDomNode rem = d->m_RootItem->node().removeChild(n);
//             if (!rem.isNull())
//                 ++nb;
//             n = save;
//             ++examined;
//             continue;
//         }
//         ++examined;
//         n = n.nextSiblingElement();
//     }
//     reset();
//     return nb;
    return 0;
}
