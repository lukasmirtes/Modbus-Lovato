#include <QCoreApplication>

#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QDateTime>
#include <QNetworkReply>
#include <QSharedPointer>

#include <iostream>
#include <cctype>
#include <limits>
#include <unistd.h>

#include "Console/KeyboardScanner.h"

#include "Globals.h"
#include "Modbus/DataUnits.h"
#include "Modbus/ModbusSerialMaster.h"
#include "Processing/ProcessingManager.h"
#include "Processing/RequestManager.h"
#include "Network/NetworkAccessBase.h"
#include "Network/NetworkSender.h"
#include "Log/LogReader.h"
#include "Commands/CommandsProcessor.h"
#include "Commands/CommandsList.h"

ProcessingManager *processingManager;
KeyboardScanner keyboardScanner;
CommandsProcessor commandsProcessor;

// Early declaration of various signal receiver functions, defined below main()
void processKey(char c);
void onHTTPreply(QSharedPointer<QNetworkReply> reply);

int main(int argc, char *argv[])
{
	/// @warning The code assumes Linux OS to be used, as QSettings::setDefaultFormat(...INI...)
	/// does not behave properly - at least it reads no groups/values on construction.
	QCoreApplication a(argc, argv);
	QCoreApplication::setOrganizationName("PMCS");
	QCoreApplication::setOrganizationDomain("mendl.info");
	QCoreApplication::setApplicationName("LovatoModbus");

	processingManager = new ProcessingManager(&a);

	QObject::connect(&keyboardScanner, &KeyboardScanner::KeyPressed, &a, &processKey);
	QObject::connect(NetworkSender::commandsDistributor(), &CommandsDistributor::commandReplyReceived,
					 &onHTTPreply);
	QObject::connect(NetworkSender::commandsDistributor(), &CommandsDistributor::commandReplyReceived,
					 &commandsProcessor, &CommandsProcessor::processHttpReply);
	QObject::connect(&keyboardScanner, &KeyboardScanner::finished, &a, &QCoreApplication::quit);

	if(Q_BYTE_ORDER == Q_BIG_ENDIAN)
		qDebug() << "Host uses big endianness.";
	else
		qDebug() << "Host uses little endianness.";

	if(std::numeric_limits<float>::is_iec559)
		qDebug() << "Host complies to IEEE 754.";
	else
		qDebug() << "Host does not comply to IEEE 754.";

	keyboardScanner.start();
	qDebug() << "\nModbus application started...";
	int result = a.exec();
	qDebug() << "Modbus application quited...\n";
	return result;
}


void onHTTPreply(QSharedPointer<QNetworkReply> reply) {
	qDebug() << "HTTP response received" << reply.data();
	if(reply->error() != 0)
		qDebug() << "\tURL" << reply->url() << "SENDING ERROR: " << reply->errorString();
   else {
		qDebug() << "\tURL" << reply->url() << "SENT WITH RESULT" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
/// @todo Consider more sophisticated processing above - even signalling uplink
		qDebug() << "\tNetworkSender HEADERS:";
		foreach (QNetworkReply::RawHeaderPair header, reply->rawHeaderPairs()) {
			qDebug() << "\t\t" << header.first << "=" << header.second;
		}
//		 qDebug() << "\tDATA:\n" << reply->readAll();
		qDebug() << "\tDATA SIZE:" << reply->bytesAvailable();

		qDebug() << "End of HTTP response data";

	}

}

void processKey(char c)
{
	qDebug() << "\n--------------- COMMAND:" << c << "---------------";
	switch (toupper(c)) {
	case 'Q':
		std::cout << "Quitting...\n";
		keyboardScanner.finish();
		break;

	case 'L': // Log reader test
	{
		new LogReader("http://mirtes.wz.cz/import.php",
					  processingManager->logServer()->pathname("Common.log"), true,
					  QDateTime::fromString("Sun Jul 31 12:00:00 2016 GMT"),
					  //										QDateTime::fromString(""),
					  QDateTime::fromString("Sun Jul 31 15:30:00 2016 GMT"),
					  "Gr.*_.?"
					  );

		qDebug() << "Leaving 'L' command...";
		break;
	}

	case 'H': // Log "headers" test
	{
		new LogReader("http://mirtes.wz.cz/import.php",
					  processingManager->logServer()->pathname("Common.log"), false,
					  "Posílání pouhých hlaviček",
					  QDateTime::fromString("Sun Jul 31 12:00:00 2016 GMT"),
					  //										QDateTime::fromString(""),
					  QDateTime::fromString("Sun Jul 31 15:30:00 2016 GMT"),
					  "Gr.*_.?"
					  );

		qDebug() << "Leaving 'H' command...";
		break;
	}

	case 'E': // Test event loop
	{
		qDebug() << "Entering QEventLoop test...";
		QEventLoop loop;
		QObject::connect(&keyboardScanner, &KeyboardScanner::KeyPressed, &loop, &QEventLoop::quit);
		loop.exec();
		qDebug() << "Exiting QEventLoop test...";
		break;
	}

	/* IMPORTANT !!!
	 *
	 * As following command empirically confirmed, root of system settings of Qt
	 * on Raspberry Pi lies NOT on / but rather on /usr/local/Qt-5.5.1-raspberry
	 * i.e. master Modbus cfg file is the
	 * /usr/local/Qt-5.5.1-raspberry/etc/xdg/PMCS/LovatoModbus.conf
	 */

	case 'W': // Write INI
	{
		QSettings settings;
		QTextStream in(stdin);
		QString str, value;

		keyboardScanner.setDetection(false);
		const QRegularExpression pattern("(.*)=(.*)");
		QRegularExpressionMatch match;
		forever {
			qDebug() << "Enter KEY=VALUE:";
			str=in.readLine();
			if(str.isEmpty())
			break;

			if((match=pattern.match(str)).hasMatch()) {

				settings.setValue(match.captured(1), match.captured(2));
				qDebug().noquote() << "\twritten: " << match.captured(1) << "=" << settings.value(match.captured(1));
			}
		}
		keyboardScanner.setDetection(true);
		break;
	}

	case 'R': // Read INI
	{
		QSettings settings;
		QTextStream in(stdin);
		QString str;

		keyboardScanner.setDetection(false);
			qDebug() << "Enter KEY:";
			str=in.readLine();
			qDebug().noquote() << "\tread: " << str << "=" << settings.value(str);
		keyboardScanner.setDetection(true);
		break;
	}

	case 'C': // Test commands processing
	{
		QByteArray cmd("\n\
Log\n\
param1=A alpha\n\
;comment for parameter 2\n\
param2=B\n\
command=hackery\n\
\n\
;general comment\n\
copY\n\
wrong=parameter=with=equals\n\
right=\"Quoted parameter\"\n\
");
		QBuffer buff(&cmd);
		if(!buff.open(QIODevice::ReadOnly)) {
			qDebug() << "Error opening buffer!";
			break;
		}
/*
		for (CommandDescriptor descr : CommandsList(&buff)) {
			qDebug() << "\tSingle command:"<< descr;
		}
*/

		commandsProcessor.processCommandDevice(&buff);
	}

	// Check the ~/.config/PMCS/LovatoModbus.conf generated by this command
	case 'S': // Settings
	{
		QSettings settings;
		settings.beginGroup("Test");
		settings.setValue("true",true);
		settings.setValue("false",false);
		settings.endGroup();
	}
		break;

	case 'G': // Groups
	{
		QSettings active, group;
		active.beginGroup("Active");
		for(QString s : active.childKeys()) {
			qDebug() << "Group:" << s;
			group.beginGroup(s);
			qDebug() <<  group.childKeys();
			qDebug() <<  group.childGroups();
			group.endGroup();
		}
		active.endGroup();

	}
		break;


//----------------------------------

	}
}
