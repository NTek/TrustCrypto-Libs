/* okcore.cpp
*/

/* Modifications by Tim Steiner
 * Copyright (c) 2016 , CryptoTrust LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *      
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OnlyKey Project
 *    (http://www.crp.to/ok)"
 *
 * 4. The names "OnlyKey" and "OnlyKey Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    admin@crp.to.
 *
 * 5. Products derived from this software may not be called "OnlyKey"
 *    nor may "OnlyKey" appear in their names without prior written
 *    permission of the OnlyKey Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OnlyKey Project
 *    (http://www.crp.to/ok)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OnlyKey PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OnlyKey PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *Original U2F Portion
 *Copyright (c) 2015, Yohanes Nugroho
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 *Redistributions in binary form must reproduce the above copyright notice,
 *this list of conditions and the following disclaimer in the documentation
 *and/or other materials provided with the distribution.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sha256.h"
#include <string.h>
#include <EEPROM.h>
#include <password.h>
#include "Time.h"
#include "onlykey.h"
#include "flashkinetis.h"
#include <RNG.h>
#include "T3MacLib.h"
/*************************************/
//Firmware Version Selection
/*************************************/
bool PDmode;
#ifdef US_VERSION
#include "yksim.h"
#include "uECC.h"
#include "ykcore.h"
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>
#endif
/*************************************/
uint32_t unixTimeStamp;
int PINSET = 0;
bool unlocked = false;
bool initialized = false;
/*************************************/
//yubikey
/*************************************/
#ifdef US_VERSION
yubikey_ctx_st ctx;
#endif
/*************************************/
//U2F Assignments
/*************************************/
byte expected_next_packet;
int large_data_len;
int large_data_offset;
byte large_buffer[1024];
byte large_resp_buffer[1024];
byte recv_buffer[64];
byte resp_buffer[64];
byte handle[64];
byte sha256_hash[32];
/*************************************/
//Password.cpp Assignments
/*************************************/
Password password = Password( "not used" );
extern uint8_t phash[32];
extern uint8_t sdhash[32];
extern uint8_t pdhash[32];
extern uint8_t nonce[32];
/*************************************/
//PIN assignments
/*************************************/
uint8_t BLINKPIN;
uint8_t TOUCHPIN1;
uint8_t TOUCHPIN2;
uint8_t TOUCHPIN3;
uint8_t TOUCHPIN4;
uint8_t TOUCHPIN5;
uint8_t TOUCHPIN6;
uint8_t ANALOGPIN1;
uint8_t ANALOGPIN2;
unsigned int touchread1;
unsigned int touchread2;
unsigned int touchread3;
unsigned int touchread4;
unsigned int touchread5;
unsigned int touchread6;
/*************************************/
//RNG Assignments
/*************************************/
size_t length = 48; // First block should wait for the pool to fill up.
/*************************************/
/*************************************/
const char stored_pub[] = "\x04\xC3\xC9\x1F\x25\x2E\x20\x10\x7B\x5E\x8D\xEA\xB1\x90\x20\x98\xF7\x28\x70\x71\xE4\x54\x18\xB8\x98\xCE\x5F\xF1\x7C\xA7\x25\xAE\x78\xC3\x3C\xC7\x01\xC0\x74\x60\x11\xCB\xBB\xB5\x8B\x08\xB6\x1D\x20\xC0\x5E\x75\xD5\x01\xA3\xF8\xF7\xA1\x67\x3F\xBE\x32\x63\xAE\xBE";

const char stored_priv[] = "\xD3\x0C\x9C\xAC\x7D\xA2\xB4\xA7\xD7\x1B\x00\x2A\x40\xA3\xB5\x9A\x96\xCA\x50\x8B\xA9\xC7\xDC\x61\x7D\x98\x2C\x4B\x11\xD9\x52\xE6";

const char stored_der[] = "\x30\x82\x01\xB4\x30\x82\x01\x58\xA0\x03\x02\x01\x02\x02\x01\x01\x30\x0C\x06\x08\x2A\x86\x48\xCE\x3D\x04\x03\x02\x05\x00\x30\x61\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x44\x45\x31\x26\x30\x24\x06\x03\x55\x04\x0A\x0C\x1D\x55\x6E\x74\x72\x75\x73\x74\x77\x6F\x72\x74\x68\x79\x20\x43\x41\x20\x4F\x72\x67\x61\x6E\x69\x73\x61\x74\x69\x6F\x6E\x31\x0F\x30\x0D\x06\x03\x55\x04\x08\x0C\x06\x42\x65\x72\x6C\x69\x6E\x31\x19\x30\x17\x06\x03\x55\x04\x03\x0C\x10\x55\x6E\x74\x72\x75\x73\x74\x77\x6F\x72\x74\x68\x79\x20\x43\x41\x30\x22\x18\x0F\x32\x30\x31\x34\x30\x39\x32\x34\x31\x32\x30\x30\x30\x30\x5A\x18\x0F\x32\x31\x31\x34\x30\x39\x32\x34\x31\x32\x30\x30\x30\x30\x5A\x30\x5E\x31\x0B\x30\x09\x06\x03\x55\x04\x06\x13\x02\x44\x45\x31\x21\x30\x1F\x06\x03\x55\x04\x0A\x0C\x18\x76\x69\x72\x74\x75\x61\x6C\x2D\x75\x32\x66\x2D\x6D\x61\x6E\x75\x66\x61\x63\x74\x75\x72\x65\x72\x31\x0F\x30\x0D\x06\x03\x55\x04\x08\x0C\x06\x42\x65\x72\x6C\x69\x6E\x31\x1B\x30\x19\x06\x03\x55\x04\x03\x0C\x12\x76\x69\x72\x74\x75\x61\x6C\x2D\x75\x32\x66\x2D\x76\x30\x2E\x30\x2E\x31\x30\x59\x30\x13\x06\x07\x2A\x86\x48\xCE\x3D\x02\x01\x06\x08\x2A\x86\x48\xCE\x3D\x03\x01\x07\x03\x42\x00\x04\xC3\xC9\x1F\x25\x2E\x20\x10\x7B\x5E\x8D\xEA\xB1\x90\x20\x98\xF7\x28\x70\x71\xE4\x54\x18\xB8\x98\xCE\x5F\xF1\x7C\xA7\x25\xAE\x78\xC3\x3C\xC7\x01\xC0\x74\x60\x11\xCB\xBB\xB5\x8B\x08\xB6\x1D\x20\xC0\x5E\x75\xD5\x01\xA3\xF8\xF7\xA1\x67\x3F\xBE\x32\x63\xAE\xBE\x30\x0C\x06\x08\x2A\x86\x48\xCE\x3D\x04\x03\x02\x05\x00\x03\x48\x00\x30\x45\x02\x21\x00\x8E\xB9\x20\x57\xA1\xF3\x41\x4F\x1B\x79\x1A\x58\xE6\x07\xAB\xA4\x66\x1C\x93\x61\xFB\xC4\xBA\x89\x65\x5C\x8A\x3B\xEC\x10\x68\xDA\x02\x20\x15\x90\xA8\x76\xF0\x80\x47\xDF\x60\x8E\x23\xB2\x2A\xA0\xAA\xD2\x4B\x0D\x49\xC9\x75\x33\x00\xAF\x32\xB6\x90\x73\xF0\xA1\xA4\xDB";

char attestation_pub[66];
char attestation_priv[33];
char attestation_der[768];
  
char handlekey[34] = {NULL};
#ifdef US_VERSION
const struct uECC_Curve_t * curve = uECC_secp256r1(); //P-256
#endif
uint8_t private_k[36]; //32
uint8_t public_k[68]; //64
uint8_t public_temp[64]; //64

struct ch_state {
  int cid;
  byte state;
  int last_millis;
};

ch_state channel_states[MAX_CHANNEL];

void U2Finit()
{
  SHA256_CTX hkey;
  sha256_init(&hkey);
  sha256_update(&hkey, nonce, 32); //Add nonce to hkey 
  sha256_update(&hkey, (byte*)ID, 36); //Add ID to hkey 
  sha256_final(&hkey, (byte*)handlekey); //Create hash and store in handlekey
#ifdef DEBUG
  Serial.println("HANDLE KEY =");
  Serial.println(handlekey); 
#endif
  uint8_t length[2];
  onlykey_eeget_U2Fcertlen(length);
  int length2 = length[0] << 8 | length[1];
  if (length2 != 0) {
  onlykey_flashget_U2F();
  } else {
  memcpy(attestation_pub, stored_pub, 66);
  memcpy(attestation_priv, stored_priv, 33);
#ifdef DEBUG
  for (int i = 0; i< sizeof(stored_priv); i++) {
    Serial.print(attestation_priv[i],HEX);
    }
#endif
  memcpy(attestation_der, stored_der, sizeof(stored_der));
#ifdef DEBUG
  for (int i = 0; i< sizeof(stored_der); i++) {
    Serial.print(attestation_der[i],HEX);
    }
#endif
  }
}

void cleanup_timeout()
{
	int i;
	for (i = 0;  i < MAX_CHANNEL; i++) {
		//free channel that is inactive
		ch_state &c = channel_states[i];
		int m = millis();
		if (c.state != STATE_CHANNEL_AVAILABLE) {
			if ((m - c.last_millis) > TIMEOUT_VALUE) {
				c.state = STATE_CHANNEL_AVAILABLE;
			}
		}
	}
}

int allocate_new_channel()
{
	int i;
	//alloace new channel_id
	int channel_id = 1;

	do {
		bool found = false;
		for (i = 0;  i < MAX_CHANNEL; i++) {
			if (channel_states[i].state != STATE_CHANNEL_AVAILABLE) {
				if (channel_states[i].cid == channel_id) {
					found = true;
					channel_id++;
					break;
				}
			}
		}
		if (!found)
			break;
	} while (true);
	return channel_id;
}

int allocate_channel(int channel_id)
{
	int i;
	if (channel_id==0) {
		channel_id =  allocate_new_channel();
	}

	bool has_free_slots = false;
	for (i = 0;  i < MAX_CHANNEL; i++) {
		if (channel_states[i].state == STATE_CHANNEL_AVAILABLE) {
			has_free_slots = true;
			break;
		}
	}
	if (!has_free_slots)
		cleanup_timeout();

	for (i = 0;  i < MAX_CHANNEL; i++) {
		ch_state &c = channel_states[i];
		if (c.state == STATE_CHANNEL_AVAILABLE) {
			c.cid = channel_id;
			c.state = STATE_CHANNEL_WAIT_PACKET;
			c.last_millis = millis();
			return channel_id;
		}
	}
	return 0;
}

int initResponse(byte *buffer)
{
#ifdef DEBUG
	Serial.print("INIT RESPONSE");
#endif
	int cid = *(int*)buffer;
#ifdef DEBUG
	Serial.println(cid, HEX);
#endif
	int len = buffer[5] << 8 | buffer[6];
	int i;
	memcpy(resp_buffer, buffer, 5);
	SET_MSG_LEN(resp_buffer, 17);
	memcpy(resp_buffer + 7, buffer + 7, len); //nonce
	i = 7 + len;
	if (cid==-1) {
		cid = allocate_channel(0);
	} else {
#ifdef DEBUG
		Serial.println("using existing CID");
#endif
		allocate_channel(cid);
	}
	memcpy(resp_buffer + i, &cid, 4);
	i += 4;
	resp_buffer[i++] = U2FHID_IF_VERSION;
	resp_buffer[i++] = 1; //major
	resp_buffer[i++] = 0;
	resp_buffer[i++] = 1; //build
	//resp_buffer[i++] = CAPABILITY_WINK; //capabilities
	resp_buffer[i++] = 0; //capabilities
#ifdef DEBUG
	Serial.println("SENT RESPONSE 1");
#endif	
	RawHID.send(resp_buffer, 100);
#ifdef DEBUG
	Serial.println(cid, HEX);
#endif	
	return cid;
}


void errorResponse(byte *buffer, int code)
{
        memcpy(resp_buffer, buffer, 4);
        resp_buffer[4] = U2FHID_ERROR;
        SET_MSG_LEN(resp_buffer, 1);
        resp_buffer[7] = code & 0xff;
#ifdef DEBUG
	Serial.print("SENT RESPONSE error:");
	Serial.println(code);
#endif
	RawHID.send(resp_buffer, 100);
}


//find channel index and update last access
int find_channel_index(int channel_id)
{
	int i;

	for (i = 0;  i < MAX_CHANNEL; i++) {
		if (channel_states[i].cid==channel_id) {
			channel_states[i].last_millis = millis();
			return i;
		}
	}

	return -1;
}





void respondErrorPDU(byte *buffer, int err)
{
	SET_MSG_LEN(buffer, 2); //len("") + 2 byte SW
	byte *datapart = buffer + 7;
	APPEND_SW(datapart, (err >> 8) & 0xff, err & 0xff);
	RawHID.send(buffer, 100);
}

void sendLargeResponse(byte *request, int len)
{
#ifdef DEBUG	
	Serial.print("Sending large response ");
	Serial.println(len);
	for (int i = 0; i < len; i++) {
		Serial.print(large_resp_buffer[i], HEX);
		Serial.print(" ");
	}
	Serial.println("\n--\n");
#endif	
	memcpy(resp_buffer, request, 4); //copy cid
	resp_buffer[4] = U2FHID_MSG;
	int r = len;
	if (r>MAX_INITIAL_PACKET) {
		r = MAX_INITIAL_PACKET;
	}

	SET_MSG_LEN(resp_buffer, len);
	memcpy(resp_buffer + 7, large_resp_buffer, r);

	RawHID.send(resp_buffer, 100);
	len -= r;
	byte p = 0;
	int offset = MAX_INITIAL_PACKET;
	while (len > 0) {
		//memcpy(resp_buffer, request, 4); //copy cid, doesn't need to recopy
		resp_buffer[4] = p++;
		memcpy(resp_buffer + 5, large_resp_buffer + offset, MAX_CONTINUATION_PACKET);
		RawHID.send(resp_buffer, 100);
		len-= MAX_CONTINUATION_PACKET;
		offset += MAX_CONTINUATION_PACKET;
		delayMicroseconds(2500);
	}
}



int getCounter() {
  unsigned int eeAddress = EEpos_U2Fcounter; //EEPROM address to start reading from
  unsigned int counter;
  EEPROM.get( eeAddress, counter );
  return counter;
}

void setCounter(int counter)
{
  unsigned int eeAddress = EEpos_U2Fcounter; //EEPROM address to start reading from
  EEPROM.put( eeAddress, counter );
}


int u2f_button = 0;


void processMessage(byte *buffer)
{
  int len = buffer[5] << 8 | buffer[6];
#ifdef DEBUG  
  Serial.println(F("Got message"));
  Serial.println(len);
  Serial.println(F("Data:"));
#endif  
  byte *message = buffer + 7;
#ifdef DEBUG
  for (int i = 7; i < 7+len; i++) {
    Serial.print(buffer[i], HEX);
  }
  Serial.println(F(""));
#endif  
  //todo: check CLA = 0
  byte CLA = message[0];

  if (CLA!=0) {
#ifdef DEBUG 
    Serial.println("U2F Error SW_CLA_NOT_SUPPORTED 366");
#endif 
    respondErrorPDU(buffer, SW_CLA_NOT_SUPPORTED);
    return;
  }

  byte INS = message[1];
  byte P1 = message[2];
  byte P2 = message[3];
  int reqlength = (message[4] << 16) | (message[5] << 8) | message[6];

  switch (INS) {
  case U2F_INS_REGISTER:
    {
      if (reqlength!=64) {
#ifdef DEBUG 
		Serial.println("U2F Error SW_WRONG_LENGTH 382");
#endif 
        respondErrorPDU(buffer, SW_WRONG_LENGTH);
        return;
      }

   
      if (!u2f_button) {
#ifdef DEBUG 
		Serial.println("U2F Error SW_CONDITIONS_NOT_SATISFIED 391");
#endif 
        respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
		return;
        }
      else {
#ifdef DEBUG
          Serial.println("U2F button pressed for register");
#endif
      }
    

      byte *datapart = message + 7;
      byte *challenge_parameter = datapart;
      byte *application_parameter = datapart+32;

      memset(public_k, 0, sizeof(public_k));
      memset(private_k, 0, sizeof(private_k));
      #ifdef US_VERSION
      uECC_make_key(public_k + 1, private_k, curve); //so we ca insert 0x04
      #endif
      public_k[0] = 0x04;
#ifdef DEBUG
      Serial.println(F("Public K"));
      for (int i =0; i < sizeof(public_k); i++) {
        Serial.print(public_k[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
      Serial.println(F("Private K"));
      for (int i =0; i < sizeof(private_k); i++) {
        Serial.print(private_k[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif      
      //construct hash

      memcpy(handle, application_parameter, 32);
      memcpy(handle+32, private_k, 32);
#ifdef DEBUG
      Serial.println("Unencrypted handle");
      for (int i =0; i<sizeof(handle); i++) {
      Serial.print(handle[i],HEX);
      }
#endif
      SHA256_CTX IV;
      sha256_init(&IV);
      sha256_update(&IV, application_parameter, 32);
      sha256_final(&IV, sha256_hash);
      #ifdef US_VERSION
      aes_gcm_encrypt2(handle, (uint8_t*)sha256_hash, (uint8_t*)handlekey, 64);
      #endif 
#ifdef DEBUG
      Serial.println();
      Serial.println("Encrypted handle");
      for (int i =0; i<sizeof(handle); i++) {
      Serial.print(handle[i],HEX);
      }
#endif
      SHA256_CTX ctx;
      sha256_init(&ctx);
      large_resp_buffer[0] = 0x00;
      sha256_update(&ctx, large_resp_buffer, 1);
#ifdef DEBUG      
      Serial.println(F("App Parameter:"));
      for (int i =0; i < 32; i++) {
        Serial.print(application_parameter[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif
      sha256_update(&ctx, application_parameter, 32);
#ifdef DEBUG
      Serial.println(F("Chal Parameter:"));
      for (int i =0; i < 32; i++) {
        Serial.print(challenge_parameter[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif
      sha256_update(&ctx, challenge_parameter, 32);
      sha256_update(&ctx, handle, 64);
      sha256_update(&ctx, public_k, 65);
      sha256_final(&ctx, sha256_hash);
#ifdef DEBUG
      Serial.println(F("Hash:"));
      for (int i =0; i < 32; i++) {
        Serial.print(sha256_hash[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif

      uint8_t *signature = resp_buffer; //temporary
#ifdef US_VERSION
	  uint8_t tmp[32 + 32 + 64];
	  SHA256_HashContext ectx = {{&init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp}};
	  if (!uECC_sign_deterministic((uint8_t *)attestation_priv,
						sha256_hash,
						32,
						&ectx.uECC,
						signature,
						curve)) {
#endif         
#ifdef DEBUG
      Serial.println("ECC Signature Failed Register");
	  //respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
      //return;
#endif
      }
      //if (!uECC_verify((uint8_t *)attestation_pub+1, sha256_hash, 32, signature, curve)) {
#ifdef DEBUG
      //Serial.println("ECC Verify Signature Failed Register");
      //respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
      //return;

#endif
      //}

      int len = 0;
      large_resp_buffer[len++] = 0x05;
      memcpy(large_resp_buffer + len, public_k, 65);
      len+=65;
      large_resp_buffer[len++] = 64; //length of handle
      memcpy(large_resp_buffer+len, handle, 64);
      len += 64;
#ifdef DEBUG
      Serial.println("len = ");
      Serial.println(len);
#endif
      uint8_t length[2];
      onlykey_eeget_U2Fcertlen(length);
      int length2 = length[0] << 8 | length[1];
      if (length2 == 0) length2 = sizeof(stored_der) - 1;
#ifdef DEBUG
      Serial.println("copy attestation_der to buffer, length = ");
      Serial.println(length2);
#endif
      memcpy(large_resp_buffer+len, attestation_der, length2);
      len += length2;
      //convert signature format
      //http://bitcoin.stackexchange.com/questions/12554/why-the-signature-is-always-65-13232-bytes-long
      large_resp_buffer[len++] = 0x30; //header: compound structure
	  uint8_t *total_len = &large_resp_buffer[len];
      large_resp_buffer[len++] = 0x44; //total length (32 + 32 + 2 + 2)
      large_resp_buffer[len++] = 0x02;  //header: integer

			if (signature[0]>0x7f) {
			   	large_resp_buffer[len++] = 33;  //33 byte
				large_resp_buffer[len++] = 0;
				(*total_len)++; //update total length
			}  else {
				large_resp_buffer[len++] = 32;  //32 byte      
		    }
	  memcpy(large_resp_buffer+len, signature, 32); //R value
      len +=32;
      large_resp_buffer[len++] = 0x02;  //header: integer

			if (signature[32]>0x7f) {
				large_resp_buffer[len++] = 33;  //32 byte
				large_resp_buffer[len++] = 0;
				(*total_len)++;	//update total length
			} else {
				large_resp_buffer[len++] = 32;  //32 byte
			}
      memcpy(large_resp_buffer+len, signature+32, 32); //R value
      len +=32;

      byte *last = large_resp_buffer+len;
      APPEND_SW_NO_ERROR(last);
      len += 2;
     
      u2f_button = 0;
      sendLargeResponse(buffer, len);
      large_data_offset = 0;
    }

    break;
  case U2F_INS_AUTHENTICATE:
    {

      //minimum is 64 + 1 + 64
      if (reqlength!=(64+1+64)) {
#ifdef DEBUG
		Serial.print("Error SW wrong length");
#endif
        respondErrorPDU(buffer, SW_WRONG_LENGTH);
        return;
      }

      byte *datapart = message + 7;
      byte *challenge_parameter = datapart;
      byte *application_parameter = datapart+32;
      byte handle_len = datapart[64];
      byte *client_handle = datapart+65;

      if (handle_len!=64) {
        //not from this device
#ifdef DEBUG
		Serial.print("Error not from this device");
#endif
        respondErrorPDU(buffer, SW_WRONG_DATA);
        return;
      }
     
      if (!u2f_button) {
#ifdef DEBUG
		Serial.print("Error U2F Button Not Pressed");
#endif
        respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
		return;
        }
      else { 
#ifdef DEBUG
        Serial.println("U2F button pressed for authenticate");
#endif
      }

      memcpy(handle, client_handle, 64);
      SHA256_CTX IV2;
      sha256_init(&IV2);
      sha256_update(&IV2, application_parameter, 32);
      sha256_final(&IV2, sha256_hash);
#ifdef DEBUG
      Serial.println("Encrypted handle");
      for (int i =0; i<sizeof(handle); i++) {
      Serial.print(handle[i]);
      }
#endif
#ifdef US_VERSION
      aes_gcm_decrypt2(handle, (uint8_t*)sha256_hash, (uint8_t*)handlekey, 64);
#endif 
#ifdef DEBUG
      Serial.println();
      Serial.println("Unencrypted handle");
      for (int i =0; i<sizeof(handle); i++) {
      Serial.print(handle[i]);
      }
#endif
      uint8_t *key = handle + 32;

      if (memcmp(handle, application_parameter, 32)!=0) {
        //this handle is not from us
#ifdef DEBUG
		Serial.println("U2F Error SW_WRONG_DATA");
#endif
        respondErrorPDU(buffer, SW_WRONG_DATA);
        return;
      }

      if (P1==0x07) { //check-only
#ifdef DEBUG
		Serial.println("U2F Error SW_CONDITIONS_NOT_SATISFIED");
#endif
        respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
      } else if (P1==0x03) { //enforce-user-presence-and-sign
        int counter = getCounter();
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, application_parameter, 32);
        large_resp_buffer[0] = 0x01; // user_presence

        int ctr = ((counter>>24)&0xff) | // move byte 3 to byte 0
          ((counter<<8)&0xff0000) | // move byte 1 to byte 2
          ((counter>>8)&0xff00) | // move byte 2 to byte 1
          ((counter<<24)&0xff000000); // byte 0 to byte 3

        memcpy(large_resp_buffer + 1, &ctr, 4);

        sha256_update(&ctx, large_resp_buffer, 5); //user presence + ctr

        sha256_update(&ctx, challenge_parameter, 32);
        sha256_final(&ctx, sha256_hash);

        uint8_t *signature = resp_buffer; //temporary
        
        #ifdef US_VERSION
        uint8_t tmp[32 + 32 + 64];
		SHA256_HashContext ectx = {{&init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp}};
        if (!uECC_sign_deterministic((uint8_t *)key,
							sha256_hash,
							32,
							&ectx.uECC,
							signature,
							curve)) {
#ifdef DEBUG
      	Serial.println("ECC Signature Failed Authenticate");
		//respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
      	//return;
#endif
      	}
		
      	//if (!uECC_verify((uint8_t *)attestation_pub+1, sha256_hash, 32, signature, curve)) {
#ifdef DEBUG
      	//Serial.println("ECC Verify Signature Failed Authenticate");
#endif
      	//respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
      	//return;
      	//}
	#endif	

        int len = 5;

        //convert signature format
        //http://bitcoin.stackexchange.com/questions/12554/why-the-signature-is-always-65-13232-bytes-long
        large_resp_buffer[len++] = 0x30; //header: compound structure
        uint8_t *total_len = &large_resp_buffer[len];				
				large_resp_buffer[len++] = 0x44; //total length (32 + 32 + 2 + 2)
				large_resp_buffer[len++] = 0x02;  //header: integer

				if (signature[0]>0x7f) {
			   	   large_resp_buffer[len++] = 33;  //33 byte
				   large_resp_buffer[len++] = 0;
				   (*total_len)++; //update total length
				} else {
				   large_resp_buffer[len++] = 32;  //32 byte
				}
        memcpy(large_resp_buffer+len, signature, 32); //R value
        len +=32;
        large_resp_buffer[len++] = 0x02;  //header: integer

				if (signature[32]>0x7f) {
				    large_resp_buffer[len++] = 33;  //32 byte
				    large_resp_buffer[len++] = 0;
				    (*total_len)++;	//update total length
				} else {
				    large_resp_buffer[len++] = 32;  //32 byte
				}
        memcpy(large_resp_buffer+len, signature+32, 32); //R value
        len +=32;
        byte *last = large_resp_buffer+len;
        APPEND_SW_NO_ERROR(last);
        len += 2;
#ifdef DEBUG
        Serial.print("Len to send ");
        Serial.println(len);
#endif
        u2f_button = 0;
        sendLargeResponse(buffer, len);
        setCounter(counter+1);
      } else {
#ifdef DEBUG
        Serial.println("return error");
#endif
      }
    }
    break;
  case U2F_INS_VERSION:
    {
      if (reqlength!=0) {
#ifdef DEBUG
		Serial.println("U2F Error SW_WRONG_LENGTH 636");
#endif
        respondErrorPDU(buffer, SW_WRONG_LENGTH);
        return;
      }
      //reuse input buffer for sending
      SET_MSG_LEN(buffer, 8); //len("U2F_V2") + 2 byte SW
      byte *datapart = buffer + 7;
      memcpy(datapart, "U2F_V2", 6);
      datapart += 6;
      APPEND_SW_NO_ERROR(datapart);
      RawHID.send(buffer, 100);
    }
    break;
  default:
    {
#ifdef DEBUG
	  Serial.println("U2F Error SW_INS_NOT_SUPPORTED 651");
#endif
      respondErrorPDU(buffer, SW_INS_NOT_SUPPORTED);
    }
    ;
  }

}

void processPacket(byte *buffer)
{
#ifdef DEBUG  
  Serial.print("Process CMD ");
#endif
  char cmd = buffer[4]; //cmd or continuation
#ifdef DEBUG
  Serial.println((int)cmd, HEX);
#endif

  int len = buffer[5] << 8 | buffer[6];
  if (cmd > U2FHID_INIT || cmd==U2FHID_LOCK) {
#ifdef DEBUG
	Serial.println("U2F Error ERR_INVALID_CMD 671");
#endif
    errorResponse(recv_buffer, ERR_INVALID_CMD);
    return;
  }
  if (cmd==U2FHID_PING) {
    if (len <= MAX_INITIAL_PACKET) {
#ifdef DEBUG      
      Serial.println("Sending ping response");
#endif      
      RawHID.send(buffer, 100);
    } else {
      //large packet
      //send first one
#ifdef DEBUG      
      Serial.println("SENT RESPONSE 3");
#endif      
      RawHID.send(buffer, 100);
      len -= MAX_INITIAL_PACKET;
      byte p = 0;
      int offset = 7 + MAX_INITIAL_PACKET;
      while (len > 0) {
        memcpy(resp_buffer, buffer, 4); //copy cid
        resp_buffer[4] = p++;
        memcpy(resp_buffer + 5, buffer + offset, MAX_CONTINUATION_PACKET);
        RawHID.send(resp_buffer, 100);
        len-= MAX_CONTINUATION_PACKET;
        offset += MAX_CONTINUATION_PACKET;
        delayMicroseconds(2500);
      }
#ifdef DEBUG      
      Serial.println("Sending large ping response");
#endif      
    }
  }
  if (cmd==U2FHID_MSG) {
    processMessage(buffer);
  }

}

void setOtherTimeout()
{
  //we can process the data
  //but if we find another channel is waiting for continuation, we set it as timeout
  for (int i = 0; i < MAX_CHANNEL; i++) {
    if (channel_states[i].state==STATE_CHANNEL_WAIT_CONT) {
#ifdef DEBUG      
      Serial.println("Set other timeout");
#endif      
      channel_states[i].state= STATE_CHANNEL_TIMEOUT;
    }
  }

}

int cont_start = 0;

void recvmsg() {
  int n;
  int c;
  int z;
  
  n = RawHID.recv(recv_buffer, 0); // 0 timeout = do not wait
 
  if (n > 0) {
#ifdef DEBUG    
    Serial.print(F("\n\nReceived packet"));
    for (z=0; z<64; z++) {
        Serial.print(recv_buffer[z], HEX);
    }
	
#endif    
    int cid = *(int*)recv_buffer;
#ifdef DEBUG    
    Serial.println(cid, HEX);
#endif    
    if (cid==0) { 
#ifdef DEBUG
	  Serial.println("U2F Error ERR_INVALID_CID 753");
#endif
      errorResponse(recv_buffer, ERR_INVALID_CID);
      return;
    }
	   //Support for additional vendor defined commands
	char cmd_or_cont = recv_buffer[4]; //cmd or continuation
    int len = (recv_buffer[5]) << 8 | recv_buffer[6];
	
	  switch (cmd_or_cont) {
      case OKSETPIN:
      if(!PDmode) {
      SETPIN(recv_buffer);
      } else {
      SETPDPIN(recv_buffer);
      }
      return;
      break;
      case OKSETSDPIN:
      SETSDPIN(recv_buffer);
      return;
      break;
      case OKSETPDPIN:
      SETPDPIN(recv_buffer);
      return;
      break;
      case OKSETTIME:
      SETTIME(recv_buffer);
      return;
      break;
      case OKGETLABELS:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("Error No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		GETLABELS(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      return;
      break;
      case OKSETSLOT:
	   if(initialized==false && unlocked==true) 
	   {
		if (recv_buffer[6] == 12) {
		SETSLOT(recv_buffer);
		} else {
		hidprint("Error No PIN set, You must set a PIN first");
		}
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		SETSLOT(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }
      return;
      break;
      case OKWIPESLOT:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("Error No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		WIPESLOT(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      return;
      break;
      case OKSETU2FPRIV:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		SETU2FPRIV(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      return;
      break;
      case OKWIPEU2FPRIV:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("Error No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		WIPEU2FPRIV(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      break;
      case OKSETU2FCERT:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("Error No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		SETU2FCERT(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      return;
      break;
      case OKWIPEU2FCERT:
	   if(initialized==false && unlocked==true) 
	   {
		hidprint("Error No PIN set, You must set a PIN first");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
		WIPEU2FCERT(recv_buffer);
	   }
	   else
	   {
	   hidprint("ERROR DEVICE LOCKED");
	   return;
	   }	
      return;
      break;
      default: 
      break;
    }

#ifdef DEBUG
    if (IS_NOT_CONTINUATION_PACKET(cmd_or_cont)) {
      Serial.print(F("LEN "));
      Serial.println((int)len);
    }
#endif
 
    //don't care about cid
    if (cmd_or_cont==U2FHID_INIT) {
      setOtherTimeout();
      cid = initResponse(recv_buffer);
      int cidx = find_channel_index(cid);
      channel_states[cidx].state= STATE_CHANNEL_WAIT_PACKET;
      return;
    }

    if (cid==-1) {
#ifdef DEBUG
	  Serial.println("U2F Error ERR_INVALID_CID 907");
#endif
      errorResponse(recv_buffer, ERR_INVALID_CID);
      return;
    }

    int cidx = find_channel_index(cid);

    if (cidx==-1) {
#ifdef DEBUG      
      Serial.println("allocating new CID");
#endif      
      allocate_channel(cid);
      cidx = find_channel_index(cid);
      if (cidx==-1) {
#ifdef DEBUG
		Serial.println("U2F Error ERR_INVALID_CID 921");
#endif
        errorResponse(recv_buffer, ERR_INVALID_CID);
        return;
      }

    }

    if (IS_NOT_CONTINUATION_PACKET(cmd_or_cont)) {

      if (len > MAX_TOTAL_PACKET) {
#ifdef DEBUG
	    Serial.println("U2F Error ERR_INVALID_LEN 931");
#endif
        errorResponse(recv_buffer, ERR_INVALID_LEN); //invalid length
        return;
      }

      if (len > MAX_INITIAL_PACKET) {
        //if another channel is waiting for continuation, we respond with busy
        for (int i = 0; i < MAX_CHANNEL; i++) {
          if (channel_states[i].state==STATE_CHANNEL_WAIT_CONT) {
            if (i==cidx) {
              #ifdef DEBUG 
              Serial.println("U2F Error ERR_INVALID_SEQ 942");
              #endif 
              errorResponse(recv_buffer, ERR_INVALID_SEQ); //invalid sequence
              channel_states[i].state= STATE_CHANNEL_WAIT_PACKET;
            } else {
              #ifdef DEBUG 
              Serial.println("U2F Error ERR_CHANNEL_BUSY 948");
              #endif 
              errorResponse(recv_buffer, ERR_CHANNEL_BUSY);
              return;
            }

            return;
          }
        }
        //no other channel is waiting
        channel_states[cidx].state=STATE_CHANNEL_WAIT_CONT;
        cont_start = millis();
        memcpy(large_buffer, recv_buffer, 64);
        large_data_len = len;
        large_data_offset = MAX_INITIAL_PACKET;
        expected_next_packet = 0;
        return;
      }

      setOtherTimeout();
      processPacket(recv_buffer);
      channel_states[cidx].state= STATE_CHANNEL_WAIT_PACKET;
    } else {

      if (channel_states[cidx].state!=STATE_CHANNEL_WAIT_CONT) {
#ifdef DEBUG        
        Serial.println("ignoring stray packet");
        Serial.println(cid, HEX);
#endif        
        return;
      }

      //this is a continuation
      if (cmd_or_cont != expected_next_packet) {
#ifdef DEBUG 
        Serial.println("U2F Error ERR_INVALID_SEQ 984");
#endif 
        errorResponse(recv_buffer, ERR_INVALID_SEQ); //invalid sequence
        channel_states[cidx].state= STATE_CHANNEL_WAIT_PACKET;
        return;
      } else {

        memcpy(large_buffer + large_data_offset + 7, recv_buffer + 5, MAX_CONTINUATION_PACKET);
        large_data_offset += MAX_CONTINUATION_PACKET;

        if (large_data_offset < large_data_len) {
          expected_next_packet++;
#ifdef DEBUG          
          Serial.println("Expecting next cont");
#endif          
          return;
        }
#ifdef DEBUG        
        Serial.println("Completed");
#endif        
        channel_states[cidx].state= STATE_CHANNEL_WAIT_PACKET;
        processPacket(large_buffer);
        return;
      }
    }
  } else {

    for (int i = 0; i < MAX_CHANNEL; i++) {
      if (channel_states[i].state==STATE_CHANNEL_TIMEOUT) {
#ifdef DEBUG        
        Serial.println("send timeout");
        Serial.println(channel_states[i].cid, HEX);
#endif        
        memcpy(recv_buffer, &channel_states[i].cid, 4);
#ifdef DEBUG
		Serial.println("U2F Error ERR_MSG_TIMEOUT 1017");
#endif
        errorResponse(recv_buffer, ERR_MSG_TIMEOUT);
        channel_states[i].state= STATE_CHANNEL_WAIT_PACKET;

      }
      if (channel_states[i].state==STATE_CHANNEL_WAIT_CONT) {

        int now = millis();
        if ((now - channel_states[i].last_millis)>500) {
#ifdef DEBUG          
          Serial.println("SET timeout");
#endif          
          channel_states[i].state=STATE_CHANNEL_TIMEOUT;
        }
      }
    }
  }
}



void SETPIN (byte *buffer)
{
#ifdef DEBUG
      Serial.println("OKSETPIN MESSAGE RECEIVED");
#endif
	  
switch (PINSET) {
      case 0:
      password.reset();
#ifdef DEBUG
      Serial.println("Enter PIN");
#endif
      hidprint("OnlyKey is ready, enter your PIN");
      PINSET = 1;
      return;
      case 1:
      PINSET = 2;
      if(strlen(password.guess) > 6 && strlen(password.guess) < 11)
      {
#ifdef DEBUG
        Serial.println("Storing PIN");
#endif
        hidprint("Successful PIN entry");
		static char passguess[10];
      for (int i =0; i <= strlen(password.guess); i++) {
		passguess[i] = password.guess[i];
      }
		password.set(passguess);
        password.reset();
      }
      else
      {
#ifdef DEBUG
		Serial.println("Error PIN is not between 7 - 10 characters");
#endif
		hidprint("Error PIN is not between 7 - 10 characters");
        password.reset();
		PINSET = 0;
      }
      
      return;
      case 2:
#ifdef DEBUG
      Serial.println("Confirm PIN");
#endif
      hidprint("OnlyKey is ready, re-enter your PIN to confirm");
      PINSET = 3;
      return;
      case 3:
	  PINSET = 0;
       if(strlen(password.guess) >= 7 && strlen(password.guess) < 11)
      {
	  
          if (password.evaluate()) {
#ifdef DEBUG
            Serial.println("Both PINs Match");
#endif
            //hidprint("Both PINs Match");
			uint8_t temp[32];
			uint8_t *ptr;
			ptr = temp;
			//Copy characters to byte array
			for (int i =0; i <= strlen(password.guess); i++) {
			temp[i] = (byte)password.guess[i];
			}
			SHA256_CTX pinhash;
			sha256_init(&pinhash);
			sha256_update(&pinhash, temp, strlen(password.guess)); //Add new PIN to hash
			if (!initialized) {
			RNG2(ptr, 32); //Fill temp with random data
#ifdef DEBUG
			Serial.println("Generating NONCE");
#endif
			onlykey_flashset_noncehash (ptr); //Store in flash
			}
			else {
#ifdef DEBUG
			Serial.println("Getting NONCE");
#endif
			onlykey_flashget_noncehash (ptr, 32); 
			}
			
			sha256_update(&pinhash, temp, 32); //Add nonce to hash
			sha256_final(&pinhash, temp); //Create hash and store in temp
#ifdef DEBUG
			Serial.println("Hashing PIN and storing to Flash");
#endif
			onlykey_flashset_pinhash (ptr);

	  		initialized = true;
#ifdef DEBUG
	  		Serial.println();
			Serial.println("Successfully set PIN, remove and reinsert OnlyKey");
#endif
			hidprint("Successfully set PIN");
          }
          else {
#ifdef DEBUG
            Serial.println("Error PINs Don't Match");
#endif
			hidprint("Error PINs Don't Match");
			PINSET = 0;
          }
      }
      else
      {
#ifdef DEBUG
        Serial.println("Error PIN is not between 7 - 10 characters");
#endif
		hidprint("Error PIN is not between 7 - 10 characters");
		PINSET = 0;
      }
      password.reset(); 
      blink(3);
      return;
}
}

void SETSDPIN (byte *buffer)
{
#ifdef DEBUG
      Serial.println("OKSETSDPIN MESSAGE RECEIVED");
#endif
	  
      switch (PINSET) {
      case 0:
      password.reset();
#ifdef DEBUG
      Serial.println("Enter PIN");
#endif
      hidprint("OnlyKey is ready, enter your self-destruct PIN");
      PINSET = 4;
      return;
      case 4:
	  PINSET = 5;
      if(strlen(password.guess) >= 7 && strlen(password.guess) < 11)
      {
#ifdef DEBUG
        Serial.println("Storing PIN");
#endif
        hidprint("Successful PIN entry");
		static char passguess[10];
      for (int i =0; i <= strlen(password.guess); i++) {
		passguess[i] = password.guess[i];
      }
		password.set(passguess);
        password.reset();
      }
      else
      {
#ifdef DEBUG
		Serial.println("Error PIN is not between 7 - 10 characters");
#endif
		hidprint("Error PIN is not between 7 - 10 characters");
        password.reset();
		PINSET = 0;
      }
      
      return;
      case 5:
#ifdef DEBUG
      Serial.println("Confirm PIN");
#endif
      hidprint("OnlyKey is ready, re-enter your PIN to confirm");
      PINSET = 6;
      return;
      case 6:
	  PINSET = 0;
       if(strlen(password.guess) >= 7 && strlen(password.guess) < 11)
      {
	  
          if (password.evaluate() == true) {
#ifdef DEBUG
            Serial.println("Both PINs Match");
#endif
            //hidprint("Both PINs Match");
		uint8_t temp[32];
		uint8_t *ptr;
		ptr = temp;
		//Copy characters to byte array
		for (int i =0; i <= strlen(password.guess); i++) {
		temp[i] = (byte)password.guess[i];
		}
		SHA256_CTX pinhash;
		sha256_init(&pinhash);
		sha256_update(&pinhash, temp, strlen(password.guess)); //Add new PIN to hash
#ifdef DEBUG
		Serial.println("Getting NONCE");
#endif
		onlykey_flashget_noncehash (ptr, 32); 
	
		sha256_update(&pinhash, temp, 32); //Add nonce to hash
		sha256_final(&pinhash, temp); //Create hash and store in temp
#ifdef DEBUG
		Serial.println("Hashing SDPIN and storing to Flash");
#endif
		onlykey_flashset_selfdestructhash (ptr);
		hidprint("Successfully set PIN");
          }
          else {
#ifdef DEBUG
            Serial.println("Error PINs Don't Match");
#endif
	    hidprint("Error PINs Don't Match");		
          }
      }
      else
      {
#ifdef DEBUG
        Serial.println("Error PIN is not between 7 - 10 characters");
#endif
	hidprint("Error PIN is not between 7 - 10 characters");
      }
      password.reset();
      blink(3);
      return;
}
}

void SETPDPIN (byte *buffer)
{
#ifdef DEBUG
      Serial.println("OKSETPDPIN MESSAGE RECEIVED");
#endif
	  
	switch (PINSET) {
      case 0:
      password.reset();
#ifdef DEBUG
      Serial.println("Enter PIN");
#endif
      hidprint("OnlyKey is ready, enter your PIN");
      PINSET = 7;
      return;
      case 7:
      PINSET = 8;
      if(strlen(password.guess) >= 7 && strlen(password.guess) < 11)
      {
#ifdef DEBUG
        Serial.println("Storing PIN");
#endif
        hidprint("Successful PIN entry");
		static char passguess[10];
      for (int i =0; i <= strlen(password.guess); i++) {
		passguess[i] = password.guess[i];
      }
	password.set(passguess);
        password.reset();
      }
      else
      {
#ifdef DEBUG
	Serial.println("Error PIN is not between 7 - 10 characters");
#endif
	hidprint("Error PIN is not between 7 - 10 characters");
        password.reset();
	PINSET = 0;
      }
      return;
      case 8:
#ifdef DEBUG
      Serial.println("Confirm PIN");
#endif
      hidprint("OnlyKey is ready, re-enter your PIN to confirm");
      PINSET = 9;
      return;
      case 9:
      PINSET = 0;
       if(strlen(password.guess) >= 7 && strlen(password.guess) < 11)
      {
	  
          if (password.evaluate()) {
#ifdef DEBUG
	    Serial.println("Both PINs Match");
#endif
            //hidprint("Both PINs Match");
			uint8_t temp[32];
			uint8_t *ptr;
			ptr = temp;
			//Copy characters to byte array
			for (int i =0; i <= strlen(password.guess); i++) {
			temp[i] = (byte)password.guess[i];
			}
			SHA256_CTX pinhash;
			sha256_init(&pinhash);
			sha256_update(&pinhash, temp, strlen(password.guess)); //Add new PIN to hash
			if (!initialized) {
			RNG2(ptr, 32); //Fill temp with random data
#ifdef DEBUG
			Serial.println("Generating NONCE");
#endif
			onlykey_flashset_noncehash (ptr); //Store in flash
			}
			else {
#ifdef DEBUG
			Serial.println("Getting NONCE");
#endif
			onlykey_flashget_noncehash (ptr, 32); 
			}
			
			sha256_update(&pinhash, temp, 32); //Add nonce to hash
			sha256_final(&pinhash, temp); //Create hash and store in temp
#ifdef DEBUG
			Serial.println("Hashing PIN and storing to Flash");
#endif
			onlykey_flashset_plausdenyhash (ptr);

	  		initialized = true;
#ifdef DEBUG
	  		Serial.println();
			Serial.println("Successfully set PIN, remove and reinsert OnlyKey");
#endif
			hidprint("Successfully set PIN");
          }
          else {
#ifdef DEBUG
            Serial.println("Error PINs Don't Match");
#endif
	    hidprint("Error PINs Don't Match");
	    PINSET = 0;
          }
      }
      else
      {
#ifdef DEBUG
        Serial.println("Error PIN is not between 7 - 10 characters");
#endif
	hidprint("Error PIN is not between 7 - 10 characters");
	PINSET = 0;
      }
      password.reset();
      blink(3);
      return;
}
}

void SETTIME (byte *buffer)
{
#ifdef DEBUG
      	  Serial.println();
	  Serial.println("OKSETTIME MESSAGE RECEIVED"); 
#endif       
	   if(initialized==false && unlocked==true) 
	   {
#ifdef DEBUG
		Serial.print("UNINITIALIZED");
#endif
		hidprint("UNINITIALIZED");
		return;
	   }else if (initialized==true && unlocked==true) 
	   {
#ifdef DEBUG
		Serial.print("UNLOCKED");
#endif
		hidprint("UNLOCKED");
	if (timeStatus() == timeNotSet) {  
    int i, j;                
    for(i=0, j=3; i<4; i++, j--){
    unixTimeStamp |= ((uint32_t)buffer[j + 5] << (i*8) );

	
#ifdef DEBUG
    Serial.println(buffer[j+5], HEX);
#endif
    }
	unixTimeStamp = unixTimeStamp + ((millis())/1000);                
      time_t t2 = unixTimeStamp;
#ifdef DEBUG
      Serial.print(F("Received Unix Epoch Time: "));
      Serial.println(unixTimeStamp, HEX); 
#endif
      setTime(t2); 
#ifdef DEBUG
      Serial.print(F("Current Time Set to: "));
#endif
      digitalClockDisplay();  
	  } else {
	  #ifdef DEBUG
      Serial.print(F("Time Already Set"));
	  #endif  
	  }
	  }
	  else
	  {
#ifdef DEBUG
	    Serial.print("FLASH ERROR");
#endif
		factorydefault();
	  }
      RawHID.send(resp_buffer, 0);
      blink(3);
      return;
}

void GETLABELS (byte *buffer)
{
#ifdef DEBUG
      	  Serial.println();
	  Serial.println("OKGETLABELS MESSAGE RECEIVED");
#endif
	  uint8_t label[EElen_label+3];
	  uint8_t *ptr;
	  char labelchar[EElen_label+3];
	  int offset  = 0;
	  ptr=label+2;
	  if (PDmode) offset = 12;
	  
	  onlykey_eeget_label(ptr, (offset + 1));
	  label[0] = (byte)0x01;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
	  hidprint(labelchar);
	  delay(20);
	  
	  onlykey_eeget_label(ptr, (offset   + 2));
	  label[0] = (byte)0x02;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
      	  hidprint(labelchar);
      	  delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 3));
	  label[0] = (byte)0x03;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 4));
	  label[0] = (byte)0x04;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 5));
	  label[0] = (byte)0x05;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 6));
	  label[0] = (byte)0x06;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 7));
	  label[0] = (byte)0x07;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 8));
	  label[0] = (byte)0x08;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 9));
	  label[0] = (byte)0x09;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 10));
	  label[0] = (byte)0x10;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 11));
	  label[0] = (byte)0x11;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
	  onlykey_eeget_label(ptr, (offset  + 12));
	  label[0] = (byte)0x12;
	  label[1] = (byte)0x7C;
	  ByteToChar(label, labelchar, EElen_label+3);
#ifdef DEBUG
	  Serial.println(labelchar);
#endif
          hidprint(labelchar);
          delay(20);
	  
      blink(3);
      return;
}



void SETSLOT (byte *buffer)
{
	
      char cmd = buffer[4]; //cmd or continuation
      int slot = buffer[5];
      int value = buffer[6];
      int length;
#ifdef DEBUG
      Serial.print("OKSETSLOT MESSAGE RECEIVED:");
      Serial.println((int)cmd - 0x80, HEX);
      Serial.print("Setting Slot #");
      Serial.println((int)slot, DEC);
      Serial.print("Value #");
      Serial.println((int)value, DEC);
#endif
     for (int z = 0; buffer[z + 7] + buffer[z + 8] + buffer[z + 9] + buffer[z + 10 ] != 0x00; z++) {
        length = z + 1;
#ifdef DEBUG
        Serial.print(buffer[z + 7], HEX);
#endif
       }
#ifdef DEBUG
      Serial.println(); //newline
      Serial.print("Length = ");
      Serial.println(length);
#endif
	if (PDmode) slot = slot + 12;
            switch (value) {
            case 1:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Label Value to EEPROM...");
#endif
            onlykey_eeset_label(buffer + 7, length, slot);
			hidprint("Successfully set Label");
            return;
            //break;
            case 2:
            //Encrypt and Set value in EEPROM
#ifdef DEBUG
            Serial.println("Writing Username Value to EEPROM...");
#endif
            if (!PDmode) {
#ifdef DEBUG
            Serial.println("Unencrypted");
            for (int z = 0; z < 32; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif 
#ifdef US_VERSION
      	    aes_gcm_encrypt((buffer + 7), (uint8_t*)('u'+ID[34]+slot), phash, length);
#endif 
#ifdef DEBUG
      	    Serial.println("Encrypted");
            for (int z = 0; z < 32; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif     
            }
            onlykey_eeset_username(buffer + 7, length, slot);
	    hidprint("Successfully set Username");
            return;
            //break;
            case 3:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Additional Character1 to EEPROM...");
#endif
            onlykey_eeset_addchar1(buffer + 7, slot);
#ifdef DEBUG
			Serial.print(buffer[7]);
#endif
	    hidprint("Successfully set Character1");
            return;
            //break;
            case 4:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Delay1 to EEPROM...");
#endif
            buffer[7] = (buffer[7] -'0');
            onlykey_eeset_delay1(buffer + 7, slot);
	    hidprint("Successfully set Delay1");
            return;
            //break;
            case 5:
            //Encrypt and Set value in EEPROM
#ifdef DEBUG
            Serial.println("Writing Password to EEPROM...");
#endif
            if (!PDmode) {
#ifdef DEBUG
            Serial.println("Unencrypted");
            for (int z = 0; z < 32; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif  
#ifdef US_VERSION
            aes_gcm_encrypt((buffer + 7), (uint8_t*)('p'+ID[34]+slot), phash, length);
#endif 
#ifdef DEBUG
      	    Serial.println("Encrypted");
            for (int z = 0; z < 32; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif 
            }
            onlykey_eeset_password(buffer + 7, length, slot);
	    hidprint("Successfully set Password");
            return;
            //break;
            case 6:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Additional Character2 to EEPROM...");
#endif
            onlykey_eeset_addchar2(buffer + 7, slot);
#ifdef DEBUG
			Serial.print(buffer[7]);
#endif
	    hidprint("Successfully set Character2");
            return;
            //break;
            case 7:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Delay2 to EEPROM...");
#endif
            buffer[7] = (buffer[7] -'0');
            onlykey_eeset_delay2(buffer + 7, slot);
	    hidprint("Successfully set Delay2");
            return;
            //break;
            case 8:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing 2FA Type to EEPROM...");
#endif
            onlykey_eeset_2FAtype(buffer + 7, slot);
	    hidprint("Successfully set 2FA Type");
            return;
            //break;
            case 9:
            //Encrypt and Set value in EEPROM
#ifdef DEBUG
            Serial.println("Writing TOTP Key to Flash...");
            Serial.println("Unencrypted");
            for (int z = 0; z < 64; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif 
#ifdef US_VERSION
            if (!PDmode) {
            aes_gcm_encrypt((buffer + 7), (uint8_t*)('t'+ID[34]+slot), phash, length);
            }
#endif
#ifdef DEBUG
	    Serial.println("Encrypted");
            for (int z = 0; z < 64; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif    
            onlykey_flashset_totpkey(buffer + 7, length, slot);
	    hidprint("Successfully set TOTP Key");
            return;
            //break;
            case 10:
            if (!PDmode) {
            //Encrypt and Set value in EEPROM
#ifdef DEBUG
            Serial.println("Writing AES Key, Private ID, and Public ID to EEPROM...");
            Serial.println("Unencrypted Public ID");
            for (int z = 0; z < 6; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println("Unencrypted Private ID");
            for (int z = 0; z < 6; z++) {
      	    Serial.print(buffer[z + 7 + 6], HEX);
            }
            Serial.println("Unencrypted AES Key");
            for (int z = 0; z < 16; z++) {
      	    Serial.print(buffer[z + 7 + 12], HEX);
            }
            Serial.println();
#endif 
#ifdef US_VERSION
            aes_gcm_encrypt((buffer + 7), (uint8_t*)('y'+ID[34]), phash, length);
#endif 
#ifdef DEBUG
      	    Serial.println("Encrypted");
            for (int z = 0; z < 32; z++) {
      	    Serial.print(buffer[z + 7], HEX);
            }
            Serial.println();
#endif
            uint16_t counter  = 0x0000;
            uint8_t *ptr;
  	    ptr = (uint8_t *) &counter;
  	    yubikey_eeset_counter(ptr); 
            onlykey_eeset_public((buffer + 7), EElen_public);
            onlykey_eeset_private((buffer + 7 + EElen_public));
            onlykey_eeset_aeskey((buffer + 7 + EElen_public + EElen_private), EElen_aeskey);
            yubikeyinit();
	    hidprint("Successfully set AES Key, Private ID, and Public ID");
	    }
            return;
            case 11:
            //Set value in EEPROM 
#ifdef DEBUG
            Serial.println(); //newline
            Serial.println("Writing idle timeout to EEPROM...");
#endif 
            buffer[7] = (buffer[7] -'0');
            onlykey_eeset_timeout(buffer + 7);
	    hidprint("Successfully set idle timeout");
            return;
            case 12:
            //Set value in EEPROM
#ifdef DEBUG
            Serial.println(); //newline
            Serial.println("Writing wipemode to EEPROM...");
#endif 
            buffer[7] = (buffer[7] -'0');
            onlykey_eeset_wipemode(buffer + 7);
	    hidprint("Successfully set wipe mode");
            return;
            //break;
            default: 
            return;
          }
      blink(3);
      return;
}

void WIPESLOT (byte *buffer)
{
      char cmd = buffer[4]; //cmd or continuation
      int slot = buffer[5];
      int value = buffer[6];
      int length;
#ifdef DEBUG
      Serial.print("OKWIPESLOT MESSAGE RECEIVED:");
      Serial.println((int)cmd - 0x80, HEX);
      Serial.print("Wiping Slot #");
      Serial.println((int)slot, DEC);
      Serial.print("Value #");
      Serial.println((int)value, DEC);
#endif 

      for (int z = 7; z < 64; z++) {
        buffer[z] = 0x00;
#ifdef DEBUG
        Serial.print(buffer[z], HEX);
#endif 
        }
#ifdef DEBUG
     Serial.print("Overwriting slot with 0s");
#endif 
	 if (value==0x0A) {
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping OnlyKey AES Key, Private ID, and Public ID...");
#endif 
            onlykey_eeset_aeskey((buffer + 7), 0);
            onlykey_eeset_private((buffer + 7 + EElen_aeskey));
            onlykey_eeset_public((buffer + 7 + EElen_aeskey + EElen_private), 0);
            hidprint("Successfully wiped AES Key, Private ID, and Public ID");
			return;
	 }
   	if (PDmode) slot = slot+12;
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping Label Value...");
#endif 
            onlykey_eeset_label((buffer + 7), 0, slot);
            hidprint("Successfully wiped Label");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping Username Value...");
#endif 
            onlykey_eeset_username((buffer + 7), 0, slot);
            hidprint("Successfully wiped Username");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping Additional Character1 Value...");
#endif 
            onlykey_eeset_addchar1((buffer + 7), slot);
            hidprint("Successfully wiped Additional Character 1");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Writing Delay1 to EEPROM...");
#endif 
            onlykey_eeset_delay1((buffer + 7), slot);
            hidprint("Successfully wiped Delay 1");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping Password Value...");
#endif 
            onlykey_eeset_password((buffer + 7), 0, slot);
            hidprint("Successfully wiped Password");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping Additional Character2 Value...");
#endif 
            onlykey_eeset_addchar2((buffer + 7), slot);
            hidprint("Successfully wiped Additional Character 2");
#ifdef DEBUG
	    Serial.println(); //newline
            Serial.print("Wiping Delay2 Value...");
#endif 
            onlykey_eeset_delay2((buffer + 7), slot);
            hidprint("Successfully wiped Delay 2");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping 2FA Type Value...");
#endif 
            onlykey_eeset_2FAtype((buffer + 7), slot);
            hidprint("Successfully wiped 2FA Type");
#ifdef DEBUG
            Serial.println(); //newline
            Serial.print("Wiping TOTP Key from Flash...");
#endif 
            onlykey_flashset_totpkey((buffer + 7), 0, slot);
            hidprint("Successfully wiped TOTP Key");

      blink(3);
      return;
}

void digitalClockDisplay(){
  // digital clock display of the time 
#ifdef DEBUG
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
#endif 

}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
#ifdef DEBUG
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
#endif 
}

void blink(int times){
  
  int i;
  for(i = 0; i < times; i++){
    analogWrite(BLINKPIN, 255);
    delay(100);
    analogWrite(BLINKPIN, 0);
    delay(100);
  }
}

void fadein(){
          // fade in from min to max in increments of 5 points:
          for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
          // sets the value (range from 0 to 255):
          analogWrite(BLINKPIN, fadeValue);
          delay(9);
          }
}

void fadeout(){
          // fade out from max to min in increments of 5 points:
          for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
          // sets the value (range from 0 to 255):
          analogWrite(BLINKPIN, fadeValue);
          delay(9);
          }
}

int RNG2(uint8_t *dest, unsigned size) {
	// Generate output whenever 32 bytes of entropy have been accumulated.
    // The first time through, we wait for 48 bytes for a full entropy pool.
    while (!RNG.available(length)) {
      //Serial.println("waiting for random number");
	  rngloop(); //Gather entropy
    }
    RNG.rand(dest, size);
    length = 32;
    //Serial.println("Random number =");
    //printHex(dest, size);
    //Serial.println("Size =");
    //Serial.println(size);
    return 1;
}

/*************************************/
//RNG Loop
/*************************************/
void rngloop() {
    // Stir the touchread and analog read values into the entropy pool.
    touchread1 = touchRead(TOUCHPIN1);
    //Serial.println(touchread1);
    RNG.stir((uint8_t *)touchread1, sizeof(touchread1), sizeof(touchread1));
    touchread2 = touchRead(TOUCHPIN2);
    //Serial.println(touchread2);
    RNG.stir((uint8_t *)touchread2, sizeof(touchread2), sizeof(touchread2));
    touchread3 = touchRead(TOUCHPIN3);
    //Serial.println(touchread3);
    RNG.stir((uint8_t *)touchread3, sizeof(touchread3), sizeof(touchread3));
    touchread4 = touchRead(TOUCHPIN4);
    //Serial.println(touchread4);
    RNG.stir((uint8_t *)touchread4, sizeof(touchread4), sizeof(touchread4));
    touchread5 = touchRead(TOUCHPIN5);
    //Serial.println(touchread5);
    RNG.stir((uint8_t *)touchread5, sizeof(touchread5), sizeof(touchread5));
    touchread6 = touchRead(TOUCHPIN6);
    //Serial.println(touchread6);
    RNG.stir((uint8_t *)touchread6, sizeof(touchread6), sizeof(touchread6));
    unsigned int analog1 = analogRead(ANALOGPIN1);
    //Serial.println(analog1);
    RNG.stir((uint8_t *)analog1, sizeof(analog1), sizeof(analog1)*4);
    unsigned int analog2 = analogRead(ANALOGPIN2);
    //Serial.println(analog2);
    RNG.stir((uint8_t *)analog2, sizeof(analog2), sizeof(analog2)*4);
    // Perform regular housekeeping on the random number generator.
    RNG.loop();
}

void printHex(const byte *data, unsigned len)
{
    static char const hexchars[] = "0123456789ABCDEF";
    while (len > 0) {
        int b = *data++;
#ifdef DEBUG
        Serial.print(hexchars[(b >> 4) & 0x0F]);
        Serial.print(hexchars[b & 0x0F]);
#endif 

        --len;
    } 
#ifdef DEBUG
    Serial.println();
#endif 
}

void ByteToChar(byte* bytes, char* chars, unsigned int count){
    for(unsigned int i = 0; i < count; i++)
    	 chars[i] = (char)bytes[i];
}

void CharToByte(char* chars, byte* bytes, unsigned int count){
    for(unsigned int i = 0; i < count; i++)
    	bytes[i] = (byte)chars[i];
}

void ByteToChar2(byte* bytes, char* chars, unsigned int count, unsigned int index){
    for(unsigned int i = 0; i < count; i++)
    	 chars[i+index] = (char)bytes[i];
}

void CharToByte2(char* chars, byte* bytes, unsigned int count, unsigned int index){
    for(unsigned int i = 0; i < count; i++)
    	bytes[i+index] = (byte)chars[i];
}

void hidprint(char const * chars) 
{ 
int i=0;
while(*chars) {
     resp_buffer[i] = (byte)*chars;
     chars++;
	 i++;
  }
  RawHID.send(resp_buffer, 0);
  memset(resp_buffer, 0, sizeof(resp_buffer));
}

void factorydefault() {
	uint8_t mode;
	uintptr_t adr = 0x0;
	onlykey_eeget_wipemode(&mode);
	if (mode == 0x00) {
	wipeflash(); //Wipe flash first need eeprom address for flash to wipe
	wipeEEPROM();
	} else {
	//FULLWIPE Mode
	flashEraseAll();
#ifdef DEBUG
        for (int i = 0; i < 7000; i++)
        {
        Serial.printf("0x%X", adr);
        Serial.printf(" 0x%X", *((unsigned int*)adr));
        Serial.println();
        adr=adr+4;
        }
#endif 
}
	initialized = false;
	unlocked = true;
#ifdef DEBUG
	Serial.println("factory reset has been completed");
#endif 
}

void wipeEEPROM() {
	//Erase all EEPROM values
	uint8_t value;
#ifdef DEBUG
	Serial.println("Current EEPROM Values"); //TODO remove debug
#endif 
	for (int i=0; i<2048; i++) {
	value=EEPROM.read(i);
#ifdef DEBUG
	Serial.print(i);
  	Serial.print("\t");
  	Serial.print(value, DEC);
  	Serial.println();
#endif 
	}
	value=0x00;
	for (int i=0; i<2048; i++) {
	EEPROM.write(i, value);
	}
#ifdef DEBUG
	Serial.println("EEPROM set to 0s");//TODO remove debug
#endif 
	for (int i=0; i<2048; i++) {
	value=EEPROM.read(i);
#ifdef DEBUG
	Serial.print(i);
  	Serial.print("\t");
  	Serial.print(value, DEC);
  	Serial.println();
#endif 
	}
#ifdef DEBUG
	Serial.println("EEPROM erased");//TODO remove debug
#endif 

}

void wipeflash() {
	uint8_t addr[2];
	onlykey_eeget_hashpos(addr);
	uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	//Erase flash sectors used
#ifdef DEBUG
	Serial.printf("Erase Sector 0x%X ",adr);
#endif 
	if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
	Serial.printf("successful\r\n");
#endif 
	adr=adr+2048; //Next Sector
#ifdef DEBUG 
	Serial.printf("Erase Sector 0x%X ",adr);
#endif 
	if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
	Serial.printf("successful\r\n");
#endif 
	adr=adr+4096; //Next Sector
#ifdef DEBUG 
	Serial.printf("Erase Sector 0x%X ",adr);
#endif 
	if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
	Serial.printf("successful\r\n");
#endif 
	adr=adr+6144; //Next Sector
#ifdef DEBUG 
	Serial.printf("Erase Sector 0x%X ",adr);
#endif 
	if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
	Serial.printf("successful\r\n");
	Serial.println("Flash Sectors erased");//TODO remove debug
#endif 
}


void aes_gcm_encrypt (uint8_t * state, uint8_t * iv1, const uint8_t * key, int len) {
	#ifdef US_VERSION
	GCM<AES256> gcm; 
	uint8_t iv2[12];
	uint8_t tag[16];
	uint8_t *ptr;
	ptr = iv2;
	onlykey_flashget_noncehash(ptr, 12);
		for(int i =0; i<=12; i++) {
		  iv2[i]=iv2[i]^*iv1;
		}
	gcm.clear ();
	gcm.setKey(key, sizeof(key));
	gcm.setIV(iv2, 12);
	gcm.encrypt(state, state, len);
	gcm.computeTag(tag, sizeof(tag)); 
	#endif
}

int aes_gcm_decrypt (uint8_t * state, uint8_t * iv1, const uint8_t * key, int len) {
        #ifdef US_VERSION
	GCM<AES256> gcm; 
	uint8_t iv2[12];
	uint8_t tag[16];
	uint8_t *ptr;
	ptr = iv2;
	onlykey_flashget_noncehash(ptr, 12);
		for(int i =0; i<=12; i++) {
		  iv2[i]=iv2[i]^*iv1;
		}
	gcm.clear ();
	gcm.setKey(key, sizeof(key));
	gcm.setIV(iv2, 12);
	gcm.decrypt(state, state, len);
	if (!gcm.checkTag(tag, sizeof(tag))) {
		return 1;
	}
	#endif
}

void aes_gcm_encrypt2 (uint8_t * state, uint8_t * iv1, const uint8_t * key, int len) {
	#ifdef US_VERSION
	GCM<AES256> gcm; 
	uint8_t tag[16];
	gcm.clear ();
	gcm.setKey(key, sizeof(key));
	gcm.setIV(iv1, 12);
	gcm.encrypt(state, state, len);
	gcm.computeTag(tag, sizeof(tag)); 
	#endif
}

int aes_gcm_decrypt2 (uint8_t * state, uint8_t * iv1, const uint8_t * key, int len) {
        #ifdef US_VERSION
	GCM<AES256> gcm; 
	uint8_t tag[16];
	gcm.clear ();
	gcm.setKey(key, sizeof(key));
	gcm.setIV(iv1, 12);
	gcm.decrypt(state, state, len);
	if (!gcm.checkTag(tag, sizeof(tag))) {
		return 1;
	}
	#endif
}

/*************************************/
void onlykey_flashget_common (uint8_t *ptr, unsigned long *adr, int len) {
    for( int z = 0; z <= len-4; z=z+4){
        //Serial.printf(" 0x%X", (adr));
        *ptr = (uint8_t)((*(adr) >> 24) & 0xFF);
        //Serial.printf(" 0x%X", *ptr);
        ptr++;
 	*ptr = (uint8_t)((*(adr) >> 16) & 0xFF);
 	//Serial.printf(" 0x%X", *ptr);
 	ptr++;
 	*ptr = (uint8_t)((*(adr) >> 8) & 0xFF);
 	//Serial.printf(" 0x%X", *ptr);
 	ptr++;
 	*ptr = (uint8_t)((*(adr) & 0xFF));
 	//Serial.printf(" 0x%X", *ptr);
 	//Serial.println();
 	ptr++;
 	adr++;
	}
	return;
}

void onlykey_flashset_common (uint8_t *ptr, unsigned long *adr, int len) {
	for( int z = 0; z <= len-4; z=z+4){
	unsigned long data = (uint8_t)*(ptr+z+3) | ((uint8_t)*(ptr+z+2) << 8) | ((uint8_t)*(ptr+z+1) << 16) | ((uint8_t)*(ptr+z) << 24);
	//Write long to sector 
	//Serial.println();
	//Serial.printf("Writing to Sector 0x%X, value 0x%X ", adr, data);
	if ( flashProgramWord((unsigned long*)adr, &data) ) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
	adr++;
	}
	return;
}
/*********************************/

int onlykey_flashget_noncehash (uint8_t *ptr, int size) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return 0;
    }
    else {
    unsigned long address = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    onlykey_flashget_common(ptr, (unsigned long*)address, size);
    return 1;
    }
}
void onlykey_flashset_noncehash (uint8_t *ptr) {
    uint8_t addr[2];
    uintptr_t adr;
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //First time setting pinhash
	adr = 0x20004;
#ifdef DEBUG 
	Serial.printf("First empty Sector is 0x%X\r\n", flashFirstEmptySector());
#endif 
	addr[0] = (uint8_t)((adr >> 8) & 0XFF); //convert long to array
#ifdef DEBUG 
	Serial.print("addr 1 = "); //TODO debug remove
	Serial.println(addr[0]); //TODO debug remove
#endif 
	addr[1] = (uint8_t)((adr & 0XFF));
#ifdef DEBUG 
	Serial.print("addr 2 = "); //TODO debug remove
	Serial.println(addr[1]); //TODO debug remove
#endif 
	onlykey_eeset_hashpos(addr); //Set the starting position for hash
	onlykey_flashset_common(ptr, (unsigned long*)adr, EElen_noncehash);
#ifdef DEBUG 
	Serial.print("Nonce hash address =");
    	Serial.println(adr, HEX);
#endif 
    onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_noncehash);
    }
    else {
	  uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	  uint8_t temp[255];
	  uint8_t *tptr;
	  tptr=temp;
	  //Get current flash contents
	  onlykey_flashget_common(tptr, (unsigned long*)adr, 254);
	  //Add new flash contents
		for( int z = 0; z <= 31; z++){
		temp[z] = ((uint8_t)*(ptr+z));
		}
	  //Erase flash sector
#ifdef DEBUG 
	  Serial.printf("Erase Sector 0x%X ",adr);
#endif 
	  if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
	  Serial.printf("successful\r\n");
#endif 
	  //Write buffer to flash
	  onlykey_flashset_common(tptr, (unsigned long*)adr, 254);
#ifdef DEBUG 
	  Serial.print("Nonce hash address =");
	  Serial.println(adr, HEX);
	  Serial.print("Nonce hash value =");
#endif 
	  onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_noncehash);
  }    
}


int onlykey_flashget_pinhash (uint8_t *ptr, int size) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return 0;
    }
    else {
      uintptr_t adr =  (0x02 << 16L) | (addr[0] << 8L) | addr[1];
      adr=adr+32;
      onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_pinhash);
      }
    return 1;
}
void onlykey_flashset_pinhash (uint8_t *ptr) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	  uint8_t temp[255];
	  uint8_t *tptr;
	  tptr=temp;
	  //Copy current flash contents to buffer
      onlykey_flashget_common(tptr, (unsigned long*)adr, 254);
	  //Add new flash contents to buffer
		for( int z = 0; z <= 31; z++){
		temp[z+32] = ((uint8_t)*(ptr+z));
		}
	//Erase flash sector
#ifdef DEBUG 
      Serial.printf("Erase Sector 0x%X ",adr);
#endif 
      if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
      Serial.printf("successful\r\n");
#endif 
      //Write buffer to flash
      onlykey_flashset_common(tptr, (unsigned long*)adr, 254);
#ifdef DEBUG 
      Serial.print("Pin hash address =");
#endif 
      adr=adr+32;
#ifdef DEBUG 
      Serial.println(adr, HEX);
      Serial.print("Pin hash value =");
#endif 
      onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_pinhash);
      }  
}
/*********************************/
/*********************************/

int onlykey_flashget_selfdestructhash (uint8_t *ptr) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return 0;
    }
    else {
    uintptr_t adr =  (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    adr=adr+64;
    onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_selfdestructhash);
    }
}
void onlykey_flashset_selfdestructhash (uint8_t *ptr) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
     uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
     uint8_t temp[255];
     uint8_t *tptr;
     tptr=temp;
     //Get current flash contents
     onlykey_flashget_common(tptr, (unsigned long*)adr, 254);
     //Add new flash contents
     for( int z = 0; z < EElen_selfdestructhash; z++){
     temp[z+64] = ((uint8_t)*(ptr+z));
     }
     //Erase flash sector
#ifdef DEBUG 
     Serial.printf("Erase Sector 0x%X ",adr);
#endif 
     if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
     Serial.printf("successful\r\n");
#endif 
     //Write buffer to flash
     onlykey_flashset_common(tptr, (unsigned long*)adr, 254);
#ifdef DEBUG 
     Serial.println("SD hash address =");
#endif 
     adr=adr+64;
#ifdef DEBUG 
     Serial.print(adr, HEX);
     Serial.print("SD hash value =");
#endif 
     onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_selfdestructhash);
     }  
}

/*********************************/
/*********************************/

int onlykey_flashget_plausdenyhash (uint8_t *ptr) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return 0;
    }
    else {
    uintptr_t adr =  (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    adr=adr+96;
    onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_plausdenyhash);
    }
}
void onlykey_flashset_plausdenyhash (uint8_t *ptr) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    uint8_t temp[255];
    uint8_t *tptr;
    tptr=temp;
    //Get current flash contents
    onlykey_flashget_common(tptr, (unsigned long*)adr, 254);
    //Add new flash contents
    for( int z = 0; z < EElen_plausdenyhash; z++){
    temp[z+96] = ((uint8_t)*(ptr+z));
#ifdef DEBUG 
    Serial.println(*(tptr+z), HEX);
#endif 
    }
    //Erase flash sector
#ifdef DEBUG 
    Serial.printf("Erase Sector 0x%X ",adr);
#endif 
    if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
    Serial.printf("successful\r\n");
#endif 
    //Write buffer to flash
    onlykey_flashset_common(tptr, (unsigned long*)adr, 254);
#ifdef DEBUG 
    Serial.println("PD hash address =");
#endif 
    adr=adr+96;
#ifdef DEBUG 
    Serial.print(adr, HEX);
    Serial.print("PD hash value =");
#endif 
    onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_plausdenyhash);
    }     
}


int onlykey_flashget_totpkey (uint8_t *ptr, int slot) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return 0;
    }
    else {
    uintptr_t adr =  (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    adr=adr+2048; //Next Sector

	switch (slot) {
		uint8_t length;
		int size;
        	case 1:
			onlykey_eeget_totpkeylen1(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 2:
			onlykey_eeget_totpkeylen2(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 3:
			onlykey_eeget_totpkeylen3(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 4:
			onlykey_eeget_totpkeylen4(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 5:
			onlykey_eeget_totpkeylen5(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 6:
			onlykey_eeget_totpkeylen6(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 7:
			onlykey_eeget_totpkeylen7(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 8:
			onlykey_eeget_totpkeylen8(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 9:
			onlykey_eeget_totpkeylen9(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 10:
			onlykey_eeget_totpkeylen10(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 11:
			onlykey_eeget_totpkeylen11(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 12:
			onlykey_eeget_totpkeylen12(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 13:
			onlykey_eeget_totpkeylen13(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 14:
			onlykey_eeget_totpkeylen14(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 15:
			onlykey_eeget_totpkeylen15(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 16:
			onlykey_eeget_totpkeylen16(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 17:
			onlykey_eeget_totpkeylen17(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 18:
			onlykey_eeget_totpkeylen18(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 19:
			onlykey_eeget_totpkeylen19(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 20:
			onlykey_eeget_totpkeylen20(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 21:
			onlykey_eeget_totpkeylen21(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 22:
			onlykey_eeget_totpkeylen22(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 23:
			onlykey_eeget_totpkeylen23(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;
		case 24:
			onlykey_eeget_totpkeylen24(&length);
			size = (int) length;
			if (size > EElen_totpkey) size = EElen_totpkey;
			adr=adr+((EElen_totpkey*slot)-EElen_totpkey);
			onlykey_flashget_common(ptr, (unsigned long*)adr, EElen_totpkey);
			return size;
            break;	
	}
    }

}

void onlykey_flashset_totpkey (uint8_t *ptr, int size, int slot) {
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr =  (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    adr=adr+2048;
    uint8_t temp[1536];
    uint8_t *tptr;
    tptr=temp;
    //Copy current flash contents to buffer
    onlykey_flashget_common(tptr, (unsigned long*)adr, 1536);
    //Add new flash contents to buffer
    for( int z = 0; z <= EElen_totpkey; z++){
    temp[z+((EElen_totpkey*slot)-EElen_totpkey)] = ((uint8_t)*(ptr+z));
    }
    //Erase flash sector
#ifdef DEBUG 
    Serial.printf("Erase Sector 0x%X ",adr);
#endif 
    if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
    Serial.printf("successful\r\n");
#endif 
		switch (slot) {
			uint8_t length;
        	case 1:
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen1(&length);
            	break;
		case 2:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen2(&length);
            	break;
		case 3:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen3(&length);
            break;
		case 4:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen4(&length);
            break;
		case 5:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen5(&length);
            break;
		case 6:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen6(&length);
            break;
		case 7:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen7(&length);
            break;
		case 8:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen8(&length);
            break;
		case 9:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen9(&length);
            break;
		case 10:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen10(&length);
            break;
		case 11:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen11(&length);
            break;
		case 12:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen12(&length);
            break;
            	case 13:
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen13(&length);
            	break;
		case 14:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen14(&length);
            	break;
		case 15:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen15(&length);
            break;
		case 16:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen16(&length);
            break;
		case 17:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen17(&length);
            break;
		case 18:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen18(&length);
            break;
		case 19:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen19(&length);
            break;
		case 20:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen20(&length);
            break;
		case 21:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen21(&length);
            break;
		case 22:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen22(&length);
            break;
		case 23:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen23(&length);
            break;
		case 24:
		if (size > EElen_totpkey) size = EElen_totpkey;
			if (size > EElen_totpkey) size = EElen_totpkey;
			//Write buffer to flash
    		onlykey_flashset_common(tptr, (unsigned long*)adr, 1536);
			length = (uint8_t) size;
			onlykey_eeset_totpkeylen24(&length);
            break;
	}
    }
}

/*********************************/
void onlykey_flashget_U2F ()
{
if (PDmode) return;
#ifdef DEBUG 
    Serial.println("Flashget U2F");
#endif 
    uint8_t addr[2];
    uint8_t length[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
    adr=adr+4096; //3rd flash sector
    onlykey_flashget_common((uint8_t*)attestation_priv, (unsigned long*)adr, 32); 
#ifdef DEBUG 
    Serial.print("attestation priv =");
#endif 
    for (int i = 0; i< sizeof(attestation_priv); i++) {
#ifdef DEBUG 
    Serial.println(attestation_priv[i],HEX);
#endif 
    }
    adr=adr+2048; //4th flash sector
    onlykey_eeget_U2Fcertlen(length);
    int length2 = length[0] << 8 | length[1];
#ifdef DEBUG 
    Serial.print("attestation der length=");
    Serial.println(length2);
#endif 
    onlykey_flashget_common((uint8_t*)attestation_der, (unsigned long*)adr, length2); 
#ifdef DEBUG 
    Serial.print("attestation der =");
    for (int i = 0; i< sizeof(attestation_der); i++) {
    Serial.print(attestation_der[i],HEX);
    }
#endif 
    return;
    }
}

/*********************************/
void SETU2FPRIV (byte *buffer)
{
if (PDmode) return;
#ifdef DEBUG 
    Serial.println("OKSETU2FPRIV MESSAGE RECEIVED");
#endif 
    uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	adr=adr+4096; //3rd flash sector
	uint8_t *ptr;
	//Erase flash sector
#ifdef DEBUG 
    Serial.printf("Erase Sector 0x%X ",adr);
#endif 
    if (flashEraseSector((unsigned long*)adr)) 
#ifdef DEBUG     
    {
	Serial.printf("NOT ");
    }
    Serial.printf("successful\r\n"); 
#endif 
	//Write buffer to flash
	ptr=buffer+6;
    onlykey_flashset_common(ptr, (unsigned long*)adr, 32);
#ifdef DEBUG
    Serial.print("U2F Private address =");
    Serial.println(adr, HEX);
#endif
    onlykey_flashget_common(ptr, (unsigned long*)adr, 32); 
#ifdef DEBUG
    Serial.print("U2F Private value =");
#endif
    for (int i=0; i<32; i++) {
    attestation_priv[i] = *(buffer + 5 + i);
#ifdef DEBUG
    Serial.print(attestation_priv[i],HEX);
#endif
    }
    hidprint("Successfully set U2F Private");
	}
  blink(3);
  return;
}
    

void WIPEU2FPRIV (byte *buffer)
{
if (PDmode) return;
#ifdef DEBUG
    Serial.println("OKWIPEU2FPRIV MESSAGE RECEIVED");
#endif
	uint8_t addr[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	adr=adr+4096;
	//Erase flash sector
#ifdef DEBUG
		Serial.printf("Erase Sector 0x%X ",adr);
#endif
		if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
		Serial.printf("successful\r\n");
#endif 
		hidprint("Successfully wiped U2F Private");
    blink(3);
    return;
}
}

void SETU2FCERT (byte *buffer)
{
if (PDmode) return;
#ifdef DEBUG 
    Serial.println("OKSETU2FCERT MESSAGE RECEIVED");
#endif 
    uint8_t addr[2];
    uint8_t length[2];
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	adr=adr+6144;
	uint8_t *ptr;
	if (buffer[5]==0xFF) //Not last packet
	{
		if (large_data_offset <= 710) {
			memcpy(large_buffer+large_data_offset, buffer+6, 58);
			large_data_offset = large_data_offset + 58;
		} else {
			hidprint("Error U2F Cert larger than 768 bytes");
		}
		return;
	} else { //Last packet
		if (large_data_offset <= 710 && buffer[5] <= 58) {
			memcpy(large_buffer+large_data_offset, buffer+6, buffer[5]);
			large_data_offset = large_data_offset + buffer[5];
		} else {
			hidprint("Error U2F Cert larger than 768 bytes");
		}
		length[0] = large_data_offset >> 8  & 0xFF;
		length[1] = large_data_offset       & 0xFF;
		//Set U2F Certificate size
		onlykey_eeset_U2Fcertlen(length); 
#ifdef DEBUG 

		Serial.print("Length of U2F certificate = ");
        Serial.println(large_data_offset);
		//Erase flash sector
		Serial.printf("Erase Sector 0x%X ",adr);
#endif 
		if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
		Serial.printf("successful\r\n");
#endif
		//Write buffer to flash
		ptr=large_buffer;
    	onlykey_flashset_common(ptr, (unsigned long*)adr, large_data_offset);
    	       

	}
    memcpy(attestation_der, large_buffer, 768);
#ifdef DEBUG 
    Serial.print("U2F Cert value =");
    for (int i = 0; i<large_data_offset; i++) {
    Serial.print(attestation_der[i],HEX);
    }
#endif
	large_data_offset = 0;
	hidprint("Successfully set U2F Certificate");
      blink(3);
      return;
	}
}

void WIPEU2FCERT (byte *buffer)
{
if (PDmode) return;
#ifdef DEBUG 
    Serial.println("OKWIPEU2FCERT MESSAGE RECEIVED");
#endif
	uint8_t addr[2];
	uint8_t length[2] = {0x00,0x00};
    onlykey_eeget_hashpos(addr);
    if (addr[0]+addr[1] == 0) { //pinhash not set
    	return;
    }
    else {
    uintptr_t adr = (0x02 << 16L) | (addr[0] << 8L) | addr[1];
	adr=adr+6144; //4th flash sector
	//Erase flash sector
#ifdef DEBUG 
		Serial.printf("Erase Sector 0x%X ",adr);
#endif
		if (flashEraseSector((unsigned long*)adr)) {
#ifdef DEBUG 
	Serial.printf("NOT ");
#endif 
	}
#ifdef DEBUG 
		Serial.printf("successful\r\n");
		
#endif
	onlykey_eeset_U2Fcertlen(length); 
	hidprint("Successfully wiped U2F Certificate");
    blink(3);
    return;
}
}

/*************************************/
//Initialize Yubico OTP
/*************************************/
void yubikeyinit() {
#ifdef US_VERSION
  uint32_t seed;
  uint8_t *ptr = (uint8_t *)&seed;
  RNG2(ptr, 32); //Seed the onlyKey with random data

  uint8_t temp[32];
  uint8_t aeskey[16];
  uint8_t privID[6];
  uint8_t pubID[16];
  uint16_t counter;
  char public_id[32+1];
  char private_id[12+1];

#ifdef DEBUG 
  Serial.println("Initializing onlyKey ...");
#endif
  memset(temp, 0, 32); //Clear temp buffer
  
  ptr = temp;
  onlykey_eeget_public(ptr);
  
  ptr = (temp+EElen_public);
  onlykey_eeget_private(ptr);
  
  ptr = (temp+EElen_public+EElen_private);
  onlykey_eeget_aeskey(ptr);
  
  aes_gcm_decrypt(temp, (uint8_t*)('y'+ID[34]), phash, (EElen_aeskey+EElen_private+EElen_aeskey));
#ifdef DEBUG 
  Serial.println("public_id");
#endif
  for (int i = 0; i < EElen_public; i++) {
    pubID[i] = temp[i];
#ifdef DEBUG 
    Serial.print(pubID[i],HEX);
#endif
  }
#ifdef DEBUG 
  Serial.println("private_id");
#endif
  for (int i = 0; i < EElen_private; i++) {
    privID[i] = temp[i+EElen_public];
#ifdef DEBUG 
    Serial.print(privID[i],HEX);
#endif
  }
#ifdef DEBUG 
  Serial.println("aes key");
#endif
    for (int i = 0; i < EElen_aeskey; i++) {
    aeskey[i] = temp[i+EElen_public+EElen_private];
#ifdef DEBUG 
    Serial.print(aeskey[i],HEX);
#endif
  }
  
  memset(temp, 0, 32); //Clear temp buffer
  
  ptr = (uint8_t*) &counter;
  yubikey_eeget_counter(ptr);

  yubikey_hex_encode(private_id, (char *)privID, 6);
  yubikey_hex_encode(public_id, (char *)pubID, 6);
#ifdef DEBUG 
    Serial.println("public_id");
  Serial.println(public_id);
    Serial.println("private_id");
  Serial.println(private_id);
    Serial.println("counter");
  Serial.println(counter);
#endif
  uint32_t time = 0x010203; 
  
  yubikey_init1(&ctx, aeskey, public_id, private_id, counter, time, seed);
 
  yubikey_incr_counter(&ctx);
 
  ptr = (uint8_t*) &(ctx.counter);
  yubikey_eeset_counter(ptr);
#endif
}
/*************************************/
//Generate Yubico OTP
/*************************************/
void yubikeysim(char *ptr) {
	#ifdef US_VERSION
	yubikey_simulate1(ptr, &ctx);
        yubikey_incr_usage(&ctx);
        #endif
}
/*************************************/
//Increment Yubico timestamp
/*************************************/
void yubikey_incr_time() {
	#ifdef US_VERSION
	yubikey_incr_timestamp(&ctx);
	#endif
}

