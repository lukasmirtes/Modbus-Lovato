#include "LogParsingProcessor.h"

#include <QDebug>
#include <QSettings>
#include <QDir>

#include "Globals.h"
#include "Processing/RequestManager.h"
#include "Processing/ProcessingManager.h"

LogParsingProcessor::LogParsingProcessor(QSettings *settings, QString group, QSharedPointer<LogServer> logServer) :
	_logServer(logServer),
	_logName(settings->value(REQUEST_PARSING_LOG_FILENAME_KEY).toString())
{
	setObjectName(ProcessingManager::objectNameFromGroup(LOG_PARSING_PROCESSOR_PREFIX, group));
	setOccurance(settings);

	if(_logName.isEmpty())
		_logName = group + QStringLiteral(".log");

	qDebug() << "\t\tLogParsingProcessor will write to" << _logName;
}

bool LogParsingProcessor::isValid() const {
	return _logServer->isValid();
}

void LogParsingProcessor::process(RequestManager *rm){
	if(nextOccurance())
		return;

	QString record(QStringLiteral(LOG_RECORD_VALUES)+rm->groupName()+QStringLiteral(LOG_SEPARATOR_ITEMS));
	foreach (RequestManager::parsedItem_t item, rm->parsedItems().values()) {
		record += item.def->name + QStringLiteral("=")
				  + item.value.toString() + QStringLiteral(LOG_SEPARATOR_ITEMS);

	}
	_logServer->log(_logName, record);
}
