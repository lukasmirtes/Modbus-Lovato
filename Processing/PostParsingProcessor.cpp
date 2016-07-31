#include "PostParsingProcessor.h"

#include <QDebug>
#include <QSettings>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QDateTime>

#include "Globals.h"
#include "Processing/RequestManager.h"
#include "Processing/ProcessingManager.h"

PostParsingProcessor::PostParsingProcessor(QSettings *settings, QString group) :
	_url(settings->value(REQUEST_PARSING_POST_URL_KEY).toString()),
	_reply(0),
	_delayedCount(0),
	_timeout(settings->value(REQUEST_PARSING_POST_TIMEOUT_KEY, 10000).toUInt())
{
	setObjectName(ProcessingManager::objectNameFromGroup(POST_PARSING_PROCESSOR_PREFIX, group));
	setOccurance(settings);
	if(!_url.isValid())
		_url = QUrl::fromUserInput(settings->value(REQUEST_PARSING_POST_URL_KEY).toString());

	qDebug() << "\t\tParsingProcessor will post to" << _url.url();
}

bool PostParsingProcessor::isValid() const
{
	return _url.isValid();
}

void PostParsingProcessor::process(RequestManager *rm)
{
	if(nextOccurance())
		return;

	if(_reply) {
		qDebug() << "URL" << _url.url() << "STILL WAITING FOR RESPONSE retry #" << ++_delayedCount;

		requestPriority_t priority = normalRequestPriority;
		if(rm->parsedItems().contains(PARSED_ITEM_ERROR_RESPONSE_CODE_KEY))
			priority = errorRequestPriority;
		else
#ifdef NO_RESPONSE_TYPE_KEY_USED
			if(rm->parsedItems().contains(PARSED_ITEM_RESPONSE_TYPE_KEY))
#else
			if(rm->parsedItems().value(PARSED_ITEM_RESPONSE_TYPE_KEY,
										RequestManager::parsedItem_t()).value
				== PARSED_ITEM_RESPONSE_TYPE_NULL_RESPONSE_VALUE)
#endif
			priority = errorRequestPriority;

		if(priority < _priority)
			return;
	}

	// Adapted from http://doc.qt.io/qt-5/qhttpmultipart.html#details
	_multiPart.reset(new QHttpMultiPart(QHttpMultiPart::FormDataType));

	QHttpPart textPart;
	textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
					   QStringLiteral("form-data; name=groupName"));
	textPart.setBody(rm->groupName().toUtf8());
	_multiPart->append(textPart);

	textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
					   QStringLiteral("form-data; name=responseTime"));
	textPart.setBody(QDateTime::currentDateTimeUtc().toString().toUtf8());
	_multiPart->append(textPart);

	for ( RequestManager::parsedItem_t item : rm->parsedItems()) {
		textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
						   QVariant(QString(QStringLiteral("form-data; name="))+item.def->name));
		textPart.setBody(item.value.toString().toUtf8());
		_multiPart->append(textPart);
	}
	// Adapted code end

	if(_reply)
		return;

	if(_delayedCount > 0) {
		textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
						   QStringLiteral("form-data; name=delayedCount"));
		textPart.setBody(QString(QStringLiteral("%1").arg(_delayedCount)).toUtf8());
		_multiPart->append(textPart);
	}

	_reply = networkAccessManager()->post(QNetworkRequest(_url), _multiPart.data());
	connect(_reply, &QNetworkReply::finished, this, &PostParsingProcessor::onFinished);
	_timer.start(_timeout, this);
}

 void PostParsingProcessor::onFinished() {
	 _timer.stop();
	 if(_reply->error() != 0)
		 qDebug() << "URL" << _url.url() << "SENDING ERROR: " << _reply->errorString();
	else {
		 qDebug() << "URL" << _url.url() << "SENT WITH RESULT" << _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
/// @todo Consider more sophisticated processing above - even signalling uplink

		 qDebug() << "HEADERS:";
		 foreach (QNetworkReply::RawHeaderPair header, _reply->rawHeaderPairs()) {
			 qDebug() << "\t" << header.first << "=" << header.second;
		 }
		 qDebug() << "DATA:\n" << _reply->readAll();

		 _reply->deleteLater();
		 _reply = 0;
		 _delayedCount = 0;
	 }
}

void PostParsingProcessor::timerEvent(QTimerEvent *) {
	qDebug() << "URL" << _url.url() << "SENDING TIMEOUT - ABORTING !!!";
	_reply->abort();
	_reply->deleteLater();
	_reply = 0;
	_delayedCount = 0;
}