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

#ifndef __ctkDICOMDisplayedFieldGeneratorRadiotherapySeriesDescriptionRule_h
#define __ctkDICOMDisplayedFieldGeneratorRadiotherapySeriesDescriptionRule_h

// Qt includes
#include <QStringList>

#include "ctkDICOMDisplayedFieldGeneratorAbstractRule.h"

#define EMPTY_SERIES_DESCRIPTION_RTPLAN "Unnamed RT Plan"
#define EMPTY_SERIES_DESCRIPTION_RTSTRUCT "Unnamed RT Structure Set"
#define EMPTY_SERIES_DESCRIPTION_RTIMAGE "Unnamed RT Image"

/// \ingroup DICOM_Core
///
/// Default rule for generating displayed fields from DICOM fields
class CTK_DICOM_CORE_EXPORT ctkDICOMDisplayedFieldGeneratorRadiotherapySeriesDescriptionRule : public ctkDICOMDisplayedFieldGeneratorAbstractRule
{
public:
  /// \brief TODO
  virtual QStringList getRequiredDICOMTags()
  {
    QStringList requiredTags;

    requiredTags << dicomTagToString(DCM_Modality);
    
    requiredTags << dicomTagToString(DCM_RTPlanName);
    requiredTags << dicomTagToString(DCM_RTPlanLabel);    
    //requiredTags << dicomTagToString(DCM_RTPlanDate);
    //requiredTags << dicomTagToString(DCM_RTPlanTime);
    //requiredTags << dicomTagToString(DCM_RTPlanDescription);    
    
    requiredTags << dicomTagToString(DCM_StructureSetName);
    requiredTags << dicomTagToString(DCM_StructureSetLabel);
    //requiredTags << dicomTagToString(DCM_StructureSetDescription);
    //requiredTags << dicomTagToString(DCM_StructureSetDate);
    //requiredTags << dicomTagToString(DCM_StructureSetTime);
          
    requiredTags << dicomTagToString(DCM_RTImageName);
    requiredTags << dicomTagToString(DCM_RTImageLabel);
    requiredTags << dicomTagToString(DCM_RTImageDescription);
          
    return requiredTags;
  }

  /// \brief TODO
  virtual void registerEmptyFieldNames(QMap<QString, QString> emptyFieldsDisplaySeries, QMap<QString, QString> emptyFieldsDisplayStudies, QMap<QString, QString> emptyFieldsDisplayPatients)
  {
    emptyFieldsDisplaySeries.insertMulti("SeriesDescription", EMPTY_SERIES_DESCRIPTION_RTPLAN);
    emptyFieldsDisplaySeries.insertMulti("SeriesDescription", EMPTY_SERIES_DESCRIPTION_RTSTRUCT);
    emptyFieldsDisplaySeries.insertMulti("SeriesDescription", EMPTY_SERIES_DESCRIPTION_RTIMAGE);
  }

  /// \brief TODO
  virtual void getDisplayFieldsForInstance(QMap<QString, QString> cachedTags, QMap<QString, QString> &displayFieldsForCurrentSeries, QMap<QString, QString> &displayFieldsForCurrentStudy, QMap<QString, QString> &displayFieldsForCurrentPatient)
  {
    QString modality = cachedTags[dicomTagToString(DCM_Modality)];
    if (!modality.compare("RTPLAN"))
    {
      if (!cachedTags[dicomTagToString(DCM_RTPlanName)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_RTPlanName)];
      }
      else if (!cachedTags[dicomTagToString(DCM_RTPlanLabel)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_RTPlanLabel)];
      }
      else
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = QString(EMPTY_SERIES_DESCRIPTION_RTPLAN);
      }
    }
    else if (!modality.compare("RTSTRUCT"))
    {
      if (!cachedTags[dicomTagToString(DCM_StructureSetName)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_StructureSetName)];
      }
      else if (!cachedTags[dicomTagToString(DCM_StructureSetLabel)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_StructureSetLabel)];
      }
      else
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = QString(EMPTY_SERIES_DESCRIPTION_RTSTRUCT);
      }
    }
    else if (!modality.compare("RTIMAGE"))
    {
      if (!cachedTags[dicomTagToString(DCM_RTImageName)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_RTImageName)];
      }
      else if (!cachedTags[dicomTagToString(DCM_RTImageLabel)].isEmpty())
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = cachedTags[dicomTagToString(DCM_RTImageLabel)];
      }
      else
      {
        displayFieldsForCurrentSeries["SeriesDescription"] = QString(EMPTY_SERIES_DESCRIPTION_RTIMAGE);
      }
    }
  }

  /// \brief TODO
  virtual void mergeDisplayFieldsForInstance(
    const QMap<QString, QString> &initialFieldsSeries, const QMap<QString, QString> &initialFieldsStudy, const QMap<QString, QString> &initialFieldsPatient,
    const QMap<QString, QString> &newFieldsSeries, const QMap<QString, QString> &newFieldsStudy, const QMap<QString, QString> &newFieldsPatient,
    QMap<QString, QString> &mergedFieldsSeries, QMap<QString, QString> &mergedFieldsStudy, QMap<QString, QString> &mergedFieldsPatient,
    const QMap<QString, QString> &emptyFieldsSeries, const QMap<QString, QString> &emptyFieldsStudy, const QMap<QString, QString> &emptyFieldsPatient
    )
  {
    mergeConcatenate("SeriesDescription", initialFieldsSeries, newFieldsSeries, mergedFieldsSeries, emptyFieldsSeries);
  }

};

#endif
