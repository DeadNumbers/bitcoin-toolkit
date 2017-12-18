#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "version.h"
#include "mods/serialize.h"
#include "mods/hex.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define IP_ADDR_FIELD_LEN  16
#define USER_AGENT_MAX_LEN 1024

#define VERSION     70015
#define SERVICES    0x00
#define IP_ADDRESS  "00000000000000000000ffff7f000001"
#define PORT        8333
#define USER_AGENT  "/Bitcoin-Toolkit:0.1.0/"

struct Version {
	uint32_t  version;
	uint64_t services;
	uint64_t  timestamp;
	uint64_t addr_recv_services;
	unsigned char addr_recv_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_recv_port;
	uint64_t addr_trans_services;
	unsigned char addr_trans_ip_address[IP_ADDR_FIELD_LEN];
	uint16_t addr_trans_port;
	uint64_t nonce;
	uint64_t user_agent_bytes;
	char*    user_agent;
	uint32_t  start_height;
	uint8_t   relay;
};

Version version_new(void) {
	Version r;
	unsigned char *temp;
	
	NEW(r);
	
	r->version = VERSION;
	r->services = SERVICES;
	r->timestamp = time(NULL);
	r->addr_recv_services = SERVICES;

	temp = hex_str_to_uc(IP_ADDRESS);
	memcpy(r->addr_recv_ip_address, temp, IP_ADDR_FIELD_LEN);
	FREE(temp);
	
	r->addr_recv_port = PORT;
	r->addr_trans_services = SERVICES;
	
	temp = hex_str_to_uc(IP_ADDRESS);
	memcpy(r->addr_trans_ip_address, temp, IP_ADDR_FIELD_LEN);
	FREE(temp);
	
	r->addr_trans_port = PORT;
	r->nonce = 0x00;
	r->user_agent_bytes = (uint64_t)strlen(USER_AGENT);
	r->user_agent = USER_AGENT;
	r->start_height = 0x00;
	r->relay = 0x00;
	
	return r;
}

size_t version_serialize(Version v, unsigned char **s) {
	size_t len = 0;
	unsigned char *head, *ptr;
	
	ptr = head = ALLOC(sizeof(struct Version) + USER_AGENT_MAX_LEN);
	
	ptr = serialize_uint32(ptr, v->version, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->timestamp, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint64(ptr, v->addr_recv_services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uchar(ptr, v->addr_recv_ip_address, IP_ADDR_FIELD_LEN);
	ptr = serialize_uint16(ptr, v->addr_recv_port, SERIALIZE_ENDIAN_BIG);
	ptr = serialize_uint64(ptr, v->addr_trans_services, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uchar(ptr, v->addr_trans_ip_address, IP_ADDR_FIELD_LEN);
	ptr = serialize_uint16(ptr, v->addr_trans_port, SERIALIZE_ENDIAN_BIG);
	ptr = serialize_uint64(ptr, v->nonce, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_compuint(ptr, v->user_agent_bytes, SERIALIZE_ENDIAN_LIT);	
	ptr = serialize_char(ptr, v->user_agent, strlen(v->user_agent));
	ptr = serialize_uint32(ptr, v->start_height, SERIALIZE_ENDIAN_LIT);
	ptr = serialize_uint8(ptr, v->relay, SERIALIZE_ENDIAN_LIT);
	
	*s = head;
	
	while (head++ != ptr)
		len++;

	return len;
}

size_t version_new_serialize(unsigned char **s) {
	size_t r;
	Version v;
	
	v = version_new();
	
	r = version_serialize(v, s);
	
	version_free(v);
	
	return r;
}

size_t version_deserialize(unsigned char *src, Version *dest, size_t l) {
	
	assert(src);
	assert(l >= 54 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN);
	
	if (*dest == NULL)
		NEW0(*dest);
	
	src = deserialize_uint32(&((*dest)->version), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->timestamp), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint64(&((*dest)->addr_recv_services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uchar((*dest)->addr_recv_ip_address, src, IP_ADDR_FIELD_LEN);
	src = deserialize_uint16(&((*dest)->addr_recv_port), src, SERIALIZE_ENDIAN_BIG);
	src = deserialize_uint64(&((*dest)->addr_trans_services), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uchar((*dest)->addr_trans_ip_address, src, IP_ADDR_FIELD_LEN);
	src = deserialize_uint16(&((*dest)->addr_trans_port), src, SERIALIZE_ENDIAN_BIG);
	src = deserialize_uint64(&((*dest)->nonce), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_compuint(&((*dest)->user_agent_bytes), src, SERIALIZE_ENDIAN_LIT);
	if ((*dest)->user_agent_bytes) {
		assert(l >= 54 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN + (*dest)->user_agent_bytes);
		FREE((*dest)->user_agent);
		(*dest)->user_agent = ALLOC((*dest)->user_agent_bytes);
		src = deserialize_char((*dest)->user_agent, src, (*dest)->user_agent_bytes);
	} else {
		(*dest)->user_agent = NULL;
	}
	src = deserialize_uint32(&((*dest)->start_height), src, SERIALIZE_ENDIAN_LIT);
	src = deserialize_uint8(&((*dest)->relay), src, SERIALIZE_ENDIAN_LIT);
	
	return 61 + IP_ADDR_FIELD_LEN + IP_ADDR_FIELD_LEN + (*dest)->user_agent_bytes;
}

void version_free(Version v) {
	assert(v);
	FREE(v);
}
