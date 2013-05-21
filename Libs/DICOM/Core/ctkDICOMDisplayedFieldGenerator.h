/*=========================================================================

  Library:   CTK

  Copyright (c) PerkLab 2013

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

#ifndef __ctkDICOMDisplayedFieldGenerator_h
#define __ctkDICOMDisplayedFieldGenerator_h

// Qt includes
#include <QObject>
#include <QStringList>

#include "ctkDICOMCoreExport.h"

class ctkDICOMDisplayedFieldGeneratorPrivate;
class ctkDICOMDatabase;

/// \ingroup DICOM_Core
///
/// \brief Generates displayable data fields from DICOM tags
///
class CTK_DICOM_CORE_EXPORT ctkDICOMDisplayedFieldGenerator : public QObject
{
  Q_OBJECT
public:
  explicit ctkDICOMDisplayedFieldGenerator(QObject *parent = 0);
  virtual ~ctkDICOMDisplayedFieldGenerator();

  ///
  /// \brief Gets the list of tags required by all the registered rules
  ///
  Q_INVOKABLE void setDatabase(ctkDICOMDatabase* database);
  Q_INVOKABLE QStringList getRequiredTags();
  Q_INVOKABLE void updateDisplayFieldsForInstance(QString sopInstanceUID,
                                                  QMap<QString, QString> &displayFieldsForCurrentSeries,
                                                  QMap<QString, QString> &displayFieldsForCurrentStudy,
                                                  QMap<QString, QString> &displayFieldsForCurrentPatient);

protected:
  QScopedPointer<ctkDICOMDisplayedFieldGeneratorPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(ctkDICOMDisplayedFieldGenerator);
  Q_DISABLE_COPY(ctkDICOMDisplayedFieldGenerator);
};

#endif
