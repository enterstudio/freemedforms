/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include <QStringList>

namespace XmlForms {
namespace Constants {

    /** \todo Add an attribute when creating Forms/Page --> don't present in MainWindow::centralWidget's TreeWidget */
    /** \todo Add an attribute when creating Scripts --> load script from file 'fileName/#anchor' so that file can contain a multitude of scripts marked with an anchor. */
    /** \todo Manage multilingual specs. */

const char* const DOCTYPE_NAME     = "freemedforms";
const char* const DOCTYPE_EXTENSION= "xml";

const char* const TAG_MAINXMLTAG   = "FreeMedForms";
const char* const TAG_NEW_FORM     = "MedForm";
const char* const TAG_NEW_PAGE     = "Page";
const char* const TAG_NEW_ITEM     = "Item";
const char* const TAG_ADDFILE      = "file";
const char* const TAG_NAME         = "name";

const char* const TAG_SPEC_AUTHORS      = "authors";
const char* const TAG_SPEC_CATEGORY     = "category";
const char* const TAG_SPEC_LICENSE      = "licence";
const char* const TAG_SPEC_CREATIONDATE = "cdate";
const char* const TAG_SPEC_BIBLIOGRAPHY = "bibliography";
const char* const TAG_SPEC_DESCRIPTION  = "description";
const char* const TAG_SPEC_HTMLDESCRIPTION  = "htmldescription";
const char* const TAG_SPEC_LABEL        = "label";
const char* const TAG_SPEC_PLUGINNAME   = "type";
const char* const TAG_SPEC_VERSION      = "version";
const char* const TAG_SPEC_ICON         = "icon";
const char* const TAG_SPEC_TOOLTIP      = "tooltip";

const char* const TAG_VALUE              = "value";
const char* const TAG_VALUE_UUID         = "uuid";
const char* const TAG_VALUE_NUMERICAL    = "numerical";
const char* const TAG_VALUE_POSSIBLE     = "possible";
const char* const TAG_VALUE_SCRIPT       = "script";
const char* const TAG_VALUE_DEPENDENCIES = "dependon";
const char* const TAG_VALUE_DEFAULT      = "default";

const char* const TAG_SCRIPT                    = "script";
const char* const TAG_SCRIPT_ONLOAD             = "onload";
const char* const TAG_SCRIPT_POSTLOAD           = "postload";
const char* const TAG_SCRIPT_ONDEMAND           = "ondemand";
const char* const TAG_SCRIPT_ONVALUECHANGED     = "onvaluechanged";
const char* const TAG_SCRIPT_ONVALUEREQUIERED   = "onvaluerequiered";
const char* const TAG_SCRIPT_ONDEPENDENCIESCHANGED = "ondependencieschanged";

const char* const TAG_DATAPATIENT                = "patientdata";
const char* const TAG_DATAPATIENT_WEIGHT         = "weight";
const char* const TAG_DATAPATIENT_HEIGHT         = "height";
const char* const TAG_DATAPATIENT_DRUGSALLERGIES = "drugs::allergies";
const char* const TAG_DATAPATIENT_DRUGSCHRONIC   = "drugs::chronic";

const char* const TAG_OPTIONS                   = "options";
const char* const TAG_OPTIONS_UNIQUE_EPISODE    = "unique";
const char* const TAG_OPTIONS_NO_EPISODE        = "noepisode";

const char* const ATTRIB_ID           = "id";
const char* const ATTRIB_COMPLETION   = "completion";
const char* const ATTRIB_OPTIONNAL    = "optional";
const char* const ATTRIB_LANGUAGE     = "lang";

const char* const OPTION_PLUGIN_NAME  = "type";

enum creationTagsEnum {
    CreateForm = 0,
    CreatePage,
    CreateItem
};

static const QStringList createTags =
        QStringList() << TAG_NEW_FORM << TAG_NEW_PAGE << TAG_NEW_ITEM;

enum uiTagsEnum {
    Option
};
static const QStringList uiTags =
        QStringList() << "option";

} // End Constants
} // End XmlForms
