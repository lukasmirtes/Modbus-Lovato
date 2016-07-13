#include "DataUnits.h"

#include <QDebug>

#include "CrcPolynomial.h"

// ----------------------- Modbus PDU --------------------------------------

ProtocolDataUnit::ProtocolDataUnit(std::initializer_list<char> l) :
	QByteArray(l.begin(), l.size())
{}

ProtocolDataUnit::ProtocolDataUnit(QByteArray byteArray) :
	QByteArray(byteArray)
{}

qint16 ProtocolDataUnit::commandResolutionSize() {
	switch(at(aduPrefixSize())) {
	case 0x03:
		return 1+1;

	case 0x83:
		return 0;

	default:
		return -1;
	}

}

qint16 ProtocolDataUnit::commandResponseSize() {
	switch(at(aduPrefixSize())) {
	case 0x03:
		return 1+1+at(aduPrefixSize()+1);

	case 0x83:
		return 1;

	default:
		return -1;
	}
}

bool ProtocolDataUnit::isValid() {
	return bytesToRead() == 0;
}

qint16 ProtocolDataUnit::bytesToRead(void)
{
	if(size() <= aduPrefixSize())
		return aduPrefixSize()+1-size();

	if(size() <= commandResolutionSize())
		return aduPrefixSize()+commandResolutionSize()-size();

	qint16 s(commandResponseSize());
	if(s < 0)
		return s;
	else
		return aduPrefixSize()+s+aduPostfixSize()-size();

}

qint16 ProtocolDataUnit::aduPrefixSize()
{
	return 0;
}

qint16 ProtocolDataUnit::aduPostfixSize()
{
	return 0;
}


// ----------------------- Modbus ADU - serial -----------------------------

ApplicationDataUnitSerial::ApplicationDataUnitSerial(std::initializer_list<char> l) :
	ProtocolDataUnit(l)
{}

ApplicationDataUnitSerial::ApplicationDataUnitSerial(quint8 address, PDUSharedPtr_t pdu) :
	ProtocolDataUnit(*pdu)
{
	prepend(address);
}

ApplicationDataUnitSerial::ApplicationDataUnitSerial(QByteArray qba) :
	ProtocolDataUnit(qba)
{}

bool ApplicationDataUnitSerial::isValid() {
	return (bytesToRead() == 0) && isCrcValid() ;
}

quint16 ApplicationDataUnitSerial::crc(qint16 adjustSize) {
	CrcPolynomial crc;
	crc << left(size()+adjustSize);
	return crc;
}


bool ApplicationDataUnitSerial::isCrcValid() {
	return ((crc(-2) >> 8) == at(size()-1)) && ((crc(-2) & 0xFF) == at(size()-2));
}


qint16 ApplicationDataUnitSerial::aduPrefixSize()
{
	return 1;
}

qint16 ApplicationDataUnitSerial::aduPostfixSize()
{
	return 2;
}
