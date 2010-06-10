/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                      *
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#ifndef IFORMIO_H
#define IFORMIO_H

#include <formmanagerplugin/formmanager_exporter.h>

#include <translationutils/constanttranslations.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTreeWidget>

/**
 * \file iformio.h
 * \author Eric MAEKER <eric.maeker@free.fr>
 * \version 0.4.0
 * \date 08 June 2010
*/

namespace Form {

class FORM_EXPORT IFormIO : public QObject
{
    Q_OBJECT
public:
    IFormIO(const QString &absFileName, QObject *parent=0) : QObject(parent) { Q_UNUSED(absFileName); }
    virtual ~IFormIO() {}

    virtual QString name() const = 0;

    virtual void muteUserWarnings(bool state) = 0;

    virtual bool setFileName(const QString &absFileName) = 0;

    virtual QStringList fileFilters() const = 0;

    virtual QString managedFileExtension() const = 0;
    virtual bool canReadFile() const = 0;

    // canReadFile() must be called first, no need to loadForm to get these informations
    virtual bool readFileInformations() = 0;
    virtual QString formAuthor() const = 0;
    virtual QString formVersion() const = 0;
    virtual QString formDescription(const QString &lang = Trans::Constants::ALL_LANGUAGE) const = 0;
    virtual void formDescriptionToTreeWidget(QTreeWidget *tree, const QString &lang = Trans::Constants::ALL_LANGUAGE) const = 0;

    virtual bool loadForm() = 0;
    virtual bool saveForm(QObject *treeRoot) = 0;

    virtual QString lastError() const = 0;
};

} // end Form

#endif // IFORMIO_H
