#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>
#include <QList>

#include "Processing/RequestDefinition.h"

class RequestManager : public QObject
{
	Q_OBJECT
public:
	explicit RequestManager(class QSettings &settings, QObject *parent = 0);

	bool isActive() const;

signals:

public slots:

private:
	bool _active;
	quint8 _device;
	const quint8 _command; // Reserved for further extensions. Const 0x03 so far.
	quint16 _address;
	quint8 _bytesPerItem,_registerCount;
	QList<dataItemDefinition_t> _itemDefinition;
};

#endif // REQUESTMANAGER_H