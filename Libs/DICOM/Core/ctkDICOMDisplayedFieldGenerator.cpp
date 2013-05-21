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

// Qt includes
#include <QStringList>

// ctkDICOM includes
#include "ctkLogger.h"
#include "ctkDICOMDisplayedFieldGenerator.h"
#include "ctkDICOMDisplayedFieldGenerator_p.h"

#include "ctkDICOMDatabase.h"
#include "ctkDICOMDisplayedFieldGeneratorDefaultRule.h"
#include "ctkDICOMDisplayedFieldGeneratorRadiotherapySeriesDescriptionRule.h"

//------------------------------------------------------------------------------
static ctkLogger logger("org.commontk.dicom.DICOMDisplayedFieldGenerator" );
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// ctkDICOMDisplayedFieldGeneratorPrivate methods

//------------------------------------------------------------------------------
ctkDICOMDisplayedFieldGeneratorPrivate::ctkDICOMDisplayedFieldGeneratorPrivate(ctkDICOMDisplayedFieldGenerator& o) : q_ptr(&o)
  ,Database(NULL)
{
  // register commonly used rules
  AllRules.append(new ctkDICOMDisplayedFieldGeneratorDefaultRule);
  AllRules.append(new ctkDICOMDisplayedFieldGeneratorRadiotherapySeriesDescriptionRule);

  foreach(ctkDICOMDisplayedFieldGeneratorAbstractRule* rule, AllRules)
  {
    rule->registerEmptyFieldNames(EmptyFieldNamesDisplaySeries, EmptyFieldNamesDisplayStudies, EmptyFieldNamesDisplayPatients);
  }
}

//------------------------------------------------------------------------------
ctkDICOMDisplayedFieldGeneratorPrivate::~ctkDICOMDisplayedFieldGeneratorPrivate()
{
  foreach(ctkDICOMDisplayedFieldGeneratorAbstractRule* rule, AllRules)
  {
    delete rule;
  }
  AllRules.clear();
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ctkDICOMDisplayedFieldGenerator methods

//------------------------------------------------------------------------------
ctkDICOMDisplayedFieldGenerator::ctkDICOMDisplayedFieldGenerator(QObject *parent):d_ptr(new ctkDICOMDisplayedFieldGeneratorPrivate(*this))
{
  Q_UNUSED(parent);
}

//------------------------------------------------------------------------------
ctkDICOMDisplayedFieldGenerator::~ctkDICOMDisplayedFieldGenerator()
{
}

//------------------------------------------------------------------------------
QStringList ctkDICOMDisplayedFieldGenerator::getRequiredTags()
{
  Q_D(ctkDICOMDisplayedFieldGenerator);

  QStringList requiredTags;
  foreach(ctkDICOMDisplayedFieldGeneratorAbstractRule* rule, d->AllRules)
  {
    requiredTags << rule->getRequiredDICOMTags();
  }

  // TODO: remove duplicates from requiredTags (maybe also sort)
  return requiredTags;
}

//------------------------------------------------------------------------------
void ctkDICOMDisplayedFieldGenerator::updateDisplayFieldsForInstance( QString sopInstanceUID,
  QMap<QString, QString> &displayFieldsForCurrentSeries, QMap<QString, QString> &displayFieldsForCurrentStudy, QMap<QString, QString> &displayFieldsForCurrentPatient )
{
  Q_D(ctkDICOMDisplayedFieldGenerator);

  QMap<QString, QString> cachedTags;
  d->Database->getCachedTags(sopInstanceUID, cachedTags);

  QMap<QString, QString> newFieldsSeries;
  QMap<QString, QString> newFieldsStudy;
  QMap<QString, QString> newFieldsPatient;
  foreach(ctkDICOMDisplayedFieldGeneratorAbstractRule* rule, d->AllRules)
  {
    rule->getDisplayFieldsForInstance(cachedTags,newFieldsSeries,newFieldsStudy,newFieldsPatient);   
  }
  QMap<QString, QString> initialFieldsSeries=displayFieldsForCurrentSeries;
  QMap<QString, QString> initialFieldsStudy=displayFieldsForCurrentStudy;
  QMap<QString, QString> initialFieldsPatient=displayFieldsForCurrentPatient;   
  foreach(ctkDICOMDisplayedFieldGeneratorAbstractRule* rule, d->AllRules)
  {
    rule->mergeDisplayFieldsForInstance(
      initialFieldsSeries, initialFieldsStudy, initialFieldsPatient, // original DB contents
      newFieldsSeries, newFieldsStudy, newFieldsPatient, // new value
      displayFieldsForCurrentSeries, displayFieldsForCurrentStudy, displayFieldsForCurrentPatient, // new DB contents
      d->EmptyFieldNamesDisplaySeries, d->EmptyFieldNamesDisplayStudies, d->EmptyFieldNamesDisplayPatients // empty field names defined by all the rules
      );   
  }
}

//------------------------------------------------------------------------------
void ctkDICOMDisplayedFieldGenerator::setDatabase(ctkDICOMDatabase* database)
{
  Q_D(ctkDICOMDisplayedFieldGenerator);
  d->Database=database;
}
