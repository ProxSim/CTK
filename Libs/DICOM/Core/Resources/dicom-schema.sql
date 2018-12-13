--
-- A simple SQLITE3 database schema for modelling locally stored DICOM files
--
-- Note: the semicolon at the end is necessary for the simple parser to separate
--       the statements since the SQlite driver does not handle multiple
--       commands per QSqlQuery::exec call!
-- Note: be sure to update ctkDICOMDatabase and SchemaInfo Version
--       whenever you make a change to this schema
-- ;

DROP TABLE IF EXISTS 'SchemaInfo' ;
DROP TABLE IF EXISTS 'Images' ;
DROP TABLE IF EXISTS 'Patients' ;
DROP TABLE IF EXISTS 'Series' ;
DROP TABLE IF EXISTS 'Studies' ;
DROP TABLE IF EXISTS 'ColumnDisplayProperties' ;
DROP TABLE IF EXISTS 'Directories' ;

DROP INDEX IF EXISTS 'ImagesFilenameIndex' ;
DROP INDEX IF EXISTS 'ImagesSeriesIndex' ;
DROP INDEX IF EXISTS 'SeriesStudyIndex' ;
DROP INDEX IF EXISTS 'StudiesPatientIndex' ;

CREATE TABLE 'SchemaInfo' ( 'Version' VARCHAR(1024) NOT NULL );
INSERT INTO 'SchemaInfo' VALUES('0.6.0');

CREATE TABLE 'Images' (
  'SOPInstanceUID' VARCHAR(64) NOT NULL,
  'Filename' VARCHAR(1024) NOT NULL ,
  'SeriesInstanceUID' VARCHAR(64) NOT NULL ,
  'InsertTimestamp' VARCHAR(20) NOT NULL ,
  'DisplayedFieldsUpdatedTimestamp' DATETIME NULL ,
  PRIMARY KEY ('SOPInstanceUID') );
CREATE TABLE 'Patients' (
  'UID' INTEGER PRIMARY KEY AUTOINCREMENT,
  'PatientsName' VARCHAR(255) NULL ,
  'PatientID' VARCHAR(255) NULL ,
  'PatientsBirthDate' DATE NULL ,
  'PatientsBirthTime' TIME NULL ,
  'PatientsSex' VARCHAR(1) NULL ,
  'PatientsAge' VARCHAR(10) NULL ,
  'PatientsComments' VARCHAR(255) NULL ,
  'InsertTimestamp' VARCHAR(20) NOT NULL ,
  'DisplayedPatientsName' VARCHAR(255) NULL ,
  'DisplayedNumberOfStudies' INT NULL ,
  'DisplayedFieldsUpdatedTimestamp' DATETIME NULL );
CREATE TABLE 'Studies' (
  'StudyInstanceUID' VARCHAR(64) NOT NULL ,
  'PatientsUID' INT NOT NULL ,
  'StudyID' VARCHAR(255) NULL ,
  'StudyDate' DATE NULL ,
  'StudyTime' VARCHAR(20) NULL ,
  'StudyDescription' VARCHAR(255) NULL ,
  'AccessionNumber' VARCHAR(255) NULL ,
  'ModalitiesInStudy' VARCHAR(255) NULL ,
  'InstitutionName' VARCHAR(255) NULL ,
  'ReferringPhysician' VARCHAR(255) NULL ,
  'PerformingPhysiciansName' VARCHAR(255) NULL ,
  'InsertTimestamp' VARCHAR(20) NOT NULL ,
  'DisplayedNumberOfSeries' INT NULL ,
  'DisplayedFieldsUpdatedTimestamp' DATETIME NULL ,
  PRIMARY KEY ('StudyInstanceUID') );
CREATE TABLE 'Series' (
  'SeriesInstanceUID' VARCHAR(64) NOT NULL ,
  'StudyInstanceUID' VARCHAR(64) NOT NULL ,
  'SeriesNumber' INT NULL ,
  'SeriesDate' DATE NULL ,
  'SeriesTime' VARCHAR(20) NULL ,
  'SeriesDescription' VARCHAR(255) NULL ,
  'Modality' VARCHAR(20) NULL ,
  'BodyPartExamined' VARCHAR(255) NULL ,
  'FrameOfReferenceUID' VARCHAR(64) NULL ,
  'AcquisitionNumber' INT NULL ,
  'ContrastAgent' VARCHAR(255) NULL ,
  'ScanningSequence' VARCHAR(45) NULL ,
  'EchoNumber' INT NULL ,
  'TemporalPosition' INT NULL ,
  'InsertTimestamp' VARCHAR(20) NOT NULL ,
  'DisplayedNumberOfImages' INT NULL ,
  'DisplayedFieldsUpdatedTimestamp' DATETIME NULL ,
  PRIMARY KEY ('SeriesInstanceUID') );

CREATE UNIQUE INDEX IF NOT EXISTS 'ImagesFilenameIndex' ON 'Images' ('Filename');
CREATE INDEX IF NOT EXISTS 'ImagesSeriesIndex' ON 'Images' ('SeriesInstanceUID');
CREATE INDEX IF NOT EXISTS 'SeriesStudyIndex' ON 'Series' ('StudyInstanceUID');
CREATE INDEX IF NOT EXISTS 'StudiesPatientIndex' ON 'Studies' ('PatientsUID');

CREATE TABLE 'ColumnDisplayProperties' (
  'TableName' VARCHAR(64) NOT NULL,
  'FieldName' VARCHAR(64) NOT NULL ,
  'DisplayedName' VARCHAR(255) NULL ,
  'Visible' INT NULL DEFAULT 1 ,
  'Weight' INT NULL ,
  'Format' VARCHAR(255) NULL ,
  PRIMARY KEY ('TableName', 'FieldName') );

CREATE TABLE 'Directories' (
  'Dirname' VARCHAR(1024) ,
  PRIMARY KEY ('Dirname') );
