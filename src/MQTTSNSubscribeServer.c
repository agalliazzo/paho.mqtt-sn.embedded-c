/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "StackTrace.h"
#include "MQTTSNPacket.h"
#include <string.h>

int MQTTSNDeserialize_subscribe(int* dup, int* qos, unsigned short* packetid,
        MQTTSN_topicid* topicFilter, unsigned char* buf, int buflen)
{
	MQTTSNFlags flags;
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen = 0;

	FUNC_ENTRY;
	curdata += (rc = MQTTSNPacket_decode(curdata, buflen, &mylen)); /* read length */
	enddata = buf + mylen;
	if (enddata - curdata > buflen)
		goto exit;

	if (readChar(&curdata) != MQTTSN_SUBSCRIBE)
		goto exit;

	flags.all = readChar(&curdata);
	*dup = flags.bits.dup;
	*qos = flags.bits.QoS;

	*packetid = readInt(&curdata);

	topicFilter->type = flags.bits.topicIdType;

	if (topicFilter->type == MQTTSN_TOPIC_TYPE_NORMAL)
	{
		topicFilter->data.long_.len = enddata - curdata;
		topicFilter->data.long_.name = (char*)curdata;
	}
	else if (topicFilter->type == MQTTSN_TOPIC_TYPE_PREDEFINED)
		topicFilter->data.id = readInt(&curdata);
	else if (topicFilter->type == MQTTSN_TOPIC_TYPE_SHORT)
	{
		topicFilter->data.short_name[0] = readChar(&curdata);
		topicFilter->data.short_name[1] = readChar(&curdata);
	}

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


int MQTTSNSerialize_suback(unsigned char* buf, int buflen, int qos, unsigned short topicid, unsigned short packetid,
		unsigned char returncode)
{
	MQTTSNFlags flags;
	unsigned char *ptr = buf;
	int len = 0;
	int rc = 0;

	FUNC_ENTRY;
	if ((len = MQTTSNPacket_len(7)) > buflen)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	ptr += MQTTSNPacket_encode(ptr, len); /* write length */
	writeChar(&ptr, MQTTSN_SUBACK);      /* write message type */

	flags.all = 0;
	flags.bits.QoS = qos;
	writeChar(&ptr, flags.all);

	writeInt(&ptr, topicid);
	writeInt(&ptr, packetid);
	writeChar(&ptr, returncode);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}

