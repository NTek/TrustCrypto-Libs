/* okcore.cpp
*/

/*
 * Modifications by Tim Steiner
 * Copyright (c) 2016 , CryptoTrust LLC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *Original 
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
 
#include <EEPROM.h>
#include <Time.h>
#include "sha256.h"
#include "flashkinetis.h"
#include "uecc.h"
#include "ykcore.h"
#include "yksim.h"
#include "onlykey.h"

uint32_t unixTimeStamp;
const int ledPin = 13;
byte expected_next_packet;
int large_data_len;
int large_data_offset;
byte large_buffer[1024];
byte large_resp_buffer[1024];
byte recv_buffer[64];
byte resp_buffer[64];
byte handle[64];
byte sha256_hash[32];

const char attestation_key[] = "\xf3\xfc\xcc\x0d\x00\xd8\x03\x19\x54\xf9"
  "\x08\x64\xd4\x3c\x24\x7f\x4b\xf5\xf0\x66\x5c\x6b\x50\xcc"
  "\x17\x74\x9a\x27\xd1\xcf\x76\x64";

const char attestation_der[] = "\x30\x82\x01\x3c\x30\x81\xe4\xa0\x03\x02"
  "\x01\x02\x02\x0a\x47\x90\x12\x80\x00\x11\x55\x95\x73\x52"
  "\x30\x0a\x06\x08\x2a\x86\x48\xce\x3d\x04\x03\x02\x30\x17"
  "\x31\x15\x30\x13\x06\x03\x55\x04\x03\x13\x0c\x47\x6e\x75"
  "\x62\x62\x79\x20\x50\x69\x6c\x6f\x74\x30\x1e\x17\x0d\x31"
  "\x32\x30\x38\x31\x34\x31\x38\x32\x39\x33\x32\x5a\x17\x0d"
  "\x31\x33\x30\x38\x31\x34\x31\x38\x32\x39\x33\x32\x5a\x30"
  "\x31\x31\x2f\x30\x2d\x06\x03\x55\x04\x03\x13\x26\x50\x69"
  "\x6c\x6f\x74\x47\x6e\x75\x62\x62\x79\x2d\x30\x2e\x34\x2e"
  "\x31\x2d\x34\x37\x39\x30\x31\x32\x38\x30\x30\x30\x31\x31"
  "\x35\x35\x39\x35\x37\x33\x35\x32\x30\x59\x30\x13\x06\x07"
  "\x2a\x86\x48\xce\x3d\x02\x01\x06\x08\x2a\x86\x48\xce\x3d"
  "\x03\x01\x07\x03\x42\x00\x04\x8d\x61\x7e\x65\xc9\x50\x8e"
  "\x64\xbc\xc5\x67\x3a\xc8\x2a\x67\x99\xda\x3c\x14\x46\x68"
  "\x2c\x25\x8c\x46\x3f\xff\xdf\x58\xdf\xd2\xfa\x3e\x6c\x37"
  "\x8b\x53\xd7\x95\xc4\xa4\xdf\xfb\x41\x99\xed\xd7\x86\x2f"
  "\x23\xab\xaf\x02\x03\xb4\xb8\x91\x1b\xa0\x56\x99\x94\xe1"
  "\x01\x30\x0a\x06\x08\x2a\x86\x48\xce\x3d\x04\x03\x02\x03"
  "\x47\x00\x30\x44\x02\x20\x60\xcd\xb6\x06\x1e\x9c\x22\x26"
  "\x2d\x1a\xac\x1d\x96\xd8\xc7\x08\x29\xb2\x36\x65\x31\xdd"
  "\xa2\x68\x83\x2c\xb8\x36\xbc\xd3\x0d\xfa\x02\x20\x63\x1b"
  "\x14\x59\xf0\x9e\x63\x30\x05\x57\x22\xc8\xd8\x9b\x7f\x48"
  "\x88\x3b\x90\x89\xb8\x8d\x60\xd1\xd9\x79\x59\x02\xb3\x04"
  "\x10\xdf";

//key handle: (private key + app parameter) ^ this array
//TODO generate this from root key
const char handlekey[] = "-YOHANES-NUGROHO-YOHANES-NUGROHO-";

const struct uECC_Curve_t * curve = uECC_secp256r1(); //P-256
uint8_t private_k[36]; //32
uint8_t public_k[68]; //64

struct ch_state {
  int cid;
  byte state;
  int last_millis;
};

ch_state channel_states[MAX_CHANNEL];



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
  int retry = 2;
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
  unsigned int eeAddress = 0; //EEPROM address to start reading from
  unsigned int counter;
  EEPROM.get( eeAddress, counter );
  return counter;
}

void setCounter(int counter)
{
  unsigned int eeAddress = 0; //EEPROM address to start reading from
  EEPROM.put( eeAddress, counter );
}

#ifdef SIMULATE_BUTTON
//for now just simulate this
int button_pressed = 0;
#endif

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
        respondErrorPDU(buffer, SW_WRONG_LENGTH);
        return;
      }

#ifdef SIMULATE_BUTTON      
      if (button_pressed==0) {
        respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
        if (touchRead(1) > 1000){
          Serial.println("U2F button pressed for register");
         delay(1000);
         
        button_pressed = 1;
        }
        return;
      }
#endif      

      byte *datapart = message + 7;
      byte *challenge_parameter = datapart;
      byte *application_parameter = datapart+32;

      memset(public_k, 0, sizeof(public_k));
      memset(private_k, 0, sizeof(private_k));
      uECC_make_key(public_k + 1, private_k, curve); //so we ca insert 0x04
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
      for (int i =0; i < 64; i++) {
        handle[i] ^= handlekey[i%(sizeof(handlekey)-1)];
      }

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
#ifdef DEBUG
      Serial.println(F("Handle Parameter:"));
      for (int i =0; i < 64; i++) {
        Serial.print(handle[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif
      sha256_update(&ctx, handle, 64);
      sha256_update(&ctx, public_k, 65);
#ifdef DEBUG      
      Serial.println(F("Public key:"));
      for (int i =0; i < 65; i++) {
        Serial.print(public_k[i], HEX);
        Serial.print(" ");
      }
      Serial.println("");
#endif
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
      
      //TODO add uECC_sign_deterministic need to create *hash_context
      uECC_sign((uint8_t *)attestation_key,
          sha256_hash,
          32,
          signature,
          curve);

      int len = 0;
      large_resp_buffer[len++] = 0x05;
      memcpy(large_resp_buffer + len, public_k, 65);
      len+=65;
      large_resp_buffer[len++] = 64; //length of handle
      memcpy(large_resp_buffer+len, handle, 64);
      len += 64;
      memcpy(large_resp_buffer+len, attestation_der, sizeof(attestation_der));
      len += sizeof(attestation_der)-1;
      //convert signature format
      //http://bitcoin.stackexchange.com/questions/12554/why-the-signature-is-always-65-13232-bytes-long
      large_resp_buffer[len++] = 0x30; //header: compound structure
      large_resp_buffer[len++] = 0x44; //total length (32 + 32 + 2 + 2)
      large_resp_buffer[len++] = 0x02;  //header: integer
      large_resp_buffer[len++] = 32;  //32 byte
      memcpy(large_resp_buffer+len, signature, 32); //R value
      len +=32;
      large_resp_buffer[len++] = 0x02;  //header: integer
      large_resp_buffer[len++] = 32;  //32 byte
      memcpy(large_resp_buffer+len, signature+32, 32); //R value
      len +=32;

      byte *last = large_resp_buffer+len;
      APPEND_SW_NO_ERROR(last);
      len += 2;
#ifdef SIMULATE_BUTTON      
      button_pressed = 0;
#endif      
      sendLargeResponse(buffer, len);
    }

    break;
  case U2F_INS_AUTHENTICATE:
    {

      //minimum is 64 + 1 + 64
      if (reqlength!=(64+1+64)) {
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
        respondErrorPDU(buffer, SW_WRONG_DATA);
        return;
      }
#ifdef SIMULATE_BUTTON      
      if (button_pressed==0) {
        respondErrorPDU(buffer, SW_CONDITIONS_NOT_SATISFIED);
        if (touchRead(1) > 1000) { 
                    Serial.println("U2F button pressed for authenticate");
                 button_pressed = 1; 
        }
        return;
      }
#endif

      memcpy(handle, client_handle, 64);
      for (int i =0; i < 64; i++) {
        handle[i] ^= handlekey[i%(sizeof(handlekey)-1)];
      }
      uint8_t *key = handle + 32;

      if (memcmp(handle, application_parameter, 32)!=0) {
        //this handle is not from us
        respondErrorPDU(buffer, SW_WRONG_DATA);
        return;
      }

      if (P1==0x07) { //check-only
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

        //TODO add uECC_sign_deterministic need to create *hash_context
        uECC_sign((uint8_t *)key,
            sha256_hash,
            32,
            signature,
            curve);

        int len = 5;

        //convert signature format
        //http://bitcoin.stackexchange.com/questions/12554/why-the-signature-is-always-65-13232-bytes-long
        large_resp_buffer[len++] = 0x30; //header: compound structure
        large_resp_buffer[len++] = 0x44; //total length (32 + 32 + 2 + 2)
        large_resp_buffer[len++] = 0x02;  //header: integer
        large_resp_buffer[len++] = 32;  //32 byte
        memcpy(large_resp_buffer+len, signature, 32); //R value
        len +=32;
        large_resp_buffer[len++] = 0x02;  //header: integer
        large_resp_buffer[len++] = 32;  //32 byte
        memcpy(large_resp_buffer+len, signature+32, 32); //R value
        len +=32;
        byte *last = large_resp_buffer+len;
        APPEND_SW_NO_ERROR(last);
        len += 2;
#ifdef DEBUG
        Serial.print("Len to send ");
        Serial.println(len);
#endif
#ifdef SIMULATE_BUTTON              
        button_pressed = 0;
#endif        
        sendLargeResponse(buffer, len);

        setCounter(counter+1);
      } else {
        //return error
      }
    }
    break;
  case U2F_INS_VERSION:
    {
      if (reqlength!=0) {
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
  //Serial.print("");
  n = RawHID.recv(recv_buffer, 0); // 0 timeout = do not wait
#ifdef DEBUG   
/*
    Serial.print(F("U2F Counter = "));
    EEPROM.get(0, c );
     delay(100);
    Serial.println(c);
    */
#endif 
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
     Serial.println("Invalid CID 0");
     #endif 
      errorResponse(recv_buffer, ERR_INVALID_CID);
      return;
    }

    char cmd_or_cont = recv_buffer[4]; //cmd or continuation


    int len = (recv_buffer[5]) << 8 | recv_buffer[6];

#ifdef DEBUG
    if (IS_NOT_CONTINUATION_PACKET(cmd_or_cont)) {
      Serial.print(F("LEN "));
      Serial.println((int)len);
    }
#endif
    //Support for additional vendor defined commands

  switch (cmd_or_cont) {
      case OKSETPIN:
      SETPIN(recv_buffer);
      return;
      break;
      case OKSETTIME:
      SETTIME(recv_buffer);
      return;
      break;
      case OKGETLABELS:
      GETLABELS(recv_buffer);
      return;
      break;
      case OKSETSLOT:
      SETSLOT(recv_buffer);
      return;
      break;
      case OKWIPESLOT:
      WIPESLOT(recv_buffer);
      return;
      break;
      case OKSETU2FPRIV:
      SETU2FPRIV(recv_buffer);
      return;
      break;
      case OKWIPEU2FPRIV:
      WIPEU2FPRIV(recv_buffer);
      break;
      case OKSETU2FCERT:
      SETU2FCERT(recv_buffer);
      return;
      break;
      case OKWIPEU2FCERT:
      WIPEU2FCERT(recv_buffer);
      return;
      break;
      default: 
      break;
    }

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
      Serial.println("Invalid CID -1");
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
        errorResponse(recv_buffer, ERR_INVALID_CID);
        return;
      }

    }

    if (IS_NOT_CONTINUATION_PACKET(cmd_or_cont)) {

      if (len > MAX_TOTAL_PACKET) {
        #ifdef DEBUG 
        Serial.println("Invalid Length");
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
              Serial.println("Invalid Sequence");
              #endif 
              errorResponse(recv_buffer, ERR_INVALID_SEQ); //invalid sequence
              channel_states[i].state= STATE_CHANNEL_WAIT_PACKET;
            } else {
              #ifdef DEBUG 
              Serial.println("Channel Busy");
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
        errorResponse(recv_buffer, ERR_INVALID_SEQ); //invalid sequence
        #ifdef DEBUG 
        Serial.println("Invalid Sequence");
        #endif 
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
        errorResponse(recv_buffer, ERR_MSG_TIMEOUT);
        #ifdef DEBUG 
        Serial.println("Message Timeout");
        #endif 
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
      Serial.println("OKSETPIN MESSAGE RECEIVED");
      char cmd = buffer[4]; //cmd or continuation
#ifdef DEBUG
      Serial.println((int)cmd, HEX);
#endif
      blink(3);
      return;
}

void SETTIME (byte *buffer)
{
      Serial.println("OKSETTIME MESSAGE RECEIVED");
      char cmd = buffer[4]; //cmd or continuation
#ifdef DEBUG
      Serial.println((int)cmd, HEX);
#endif
    int i, j;                
    for(i=0, j=3; i<4; i++, j--){
    unixTimeStamp |= ((uint32_t)buffer[j + 5] << (i*8) );
    Serial.println(buffer[j+5], HEX);
    }
                      
      time_t t2 = unixTimeStamp;
      Serial.print(F("Received Unix Epoch Time: "));
      Serial.println(unixTimeStamp, HEX); 
      setTime(t2); 
      Serial.print(F("Current Time Set to: "));
      digitalClockDisplay();  
            //TODO Check if PIN is set first
      //if pin set 
     resp_buffer[0] = 0x49;
      resp_buffer[1] = 0x4e;
      resp_buffer[2] = 0x49;
      resp_buffer[3] = 0x54;
      resp_buffer[4] = 0x49;
      resp_buffer[5] = 0x41;
      resp_buffer[6] = 0x4c;
      resp_buffer[7] = 0x49;
      resp_buffer[8] = 0x5a;
      resp_buffer[9] = 0x45;
      resp_buffer[10] = 0x44;
      //else
      /*
      resp_buffer[0] = 0x55;
      resp_buffer[1] = 0x4e;
      resp_buffer[0] = 0x49;
      resp_buffer[1] = 0x4e;
      resp_buffer[2] = 0x49;
      resp_buffer[3] = 0x54;
      resp_buffer[4] = 0x49;
      resp_buffer[5] = 0x41;
      resp_buffer[6] = 0x4c;
      resp_buffer[7] = 0x49;
      resp_buffer[8] = 0x5a;
      resp_buffer[9] = 0x45;
      resp_buffer[10] = 0x44;
      */
      RawHID.send(resp_buffer, 0);
      blink(3);
      return;
}

void GETLABELS (byte *buffer)
{
      Serial.println("OKGETLABELS MESSAGE RECEIVED");
      char cmd = buffer[4]; //cmd or continuation
#ifdef DEBUG
      Serial.println((int)cmd, HEX);
#endif
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
      delay(1000);
#endif

      uint8_t aeskey1[16] = "abcdefghijklmno"; //128 Bit Key
      //TODO change to root key generated on first startup
#ifdef DEBUG 
      //Encrypt each block
      Serial.print("Using AES Key = ");
      for (int z = 0; z < 16; z++) {
        Serial.print(aeskey1[z], HEX);
        }
     Serial.println(); //newline
     Serial.print("Encrypting and Setting Value = ");
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
      Serial.print("Encrypted packet = ");
#endif
      yubikey_aes_encrypt ((buffer + 7), aeskey1);
      yubikey_aes_encrypt ((buffer + 23), aeskey1);
#ifdef DEBUG
      for (int z = 0; z < 32; z++) {
      Serial.print(buffer[z + 7], HEX);
        }
      
#endif      
            switch (value) {
            case 1:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Label Value to EEPROM...");
            yubikey_eeset_label((buffer + 7), EElen_label, slot);
            return;
            break;
            case 2:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Username Value to EEPROM...");
            yubikey_eeset_username((buffer + 7), EElen_username, slot);
            return;
            break;
            case 3:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Additional Character1 to EEPROM...");
            yubikey_eeset_addchar1((buffer + 7), slot);
            return;
            break;
            case 4:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Delay1 to EEPROM...");
            yubikey_eeset_delay1((buffer + 7), slot);
            return;
            break;
            case 5:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Password to EEPROM...");
            yubikey_eeset_password((buffer + 7), length, slot);
            return;
            break;
            case 6:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Additional Character2 to EEPROM...");
            yubikey_eeset_addchar2((buffer + 7), slot);
            return;
            break;
            case 7:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Delay2 to EEPROM...");
            yubikey_eeset_delay2((buffer + 7), slot);
            return;
            break;
            case 8:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing 2FA Type to EEPROM...");
            yubikey_eeset_2FAtype((buffer + 7), slot);
            return;
            break;
            case 9:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing TOTP Key to EEPROM...");
            yubikey_eeset_totpkey((buffer + 7), EElen_totpkey, slot);
            return;
            break;
            case 10:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Yubikey AES Key, Priviate ID, and Public ID to EEPROM...");
            yubikey_eeset_aeskey((buffer + 7), EElen_aeskey);
            yubikey_eeset_private((buffer + 7 + EElen_aeskey));
            yubikey_eeset_public((buffer + 7 + EElen_aeskey + EElen_private), EElen_public);
            return;
            break;
            default: 
            break;
          }


      /*
#ifdef DEBUG    
      uint8_t password1[32];
      uint8_t *ptr;
      char pass_id1[32+1];
      
      //Get value from EEPROM
      ptr = password1;
      yubikey_eeget_password1(ptr);
      yubikey_hex_encode(pass_id1, (char *)password1, EElen_password1);
      Serial.println(); //newline
      Serial.print("Read from EEPROM = ");
      Serial.println(pass_id1);

       //Decrypt each block
      Serial.print("Using AES Key = ");
      for (int z = 0; z < 16; z++) {
        Serial.print(aeskey1[z], HEX);
        }
      Serial.println(); //newline
      Serial.print("Decrypted EEPROM = ");
      yubikey_aes_decrypt (password1, aeskey1);
      yubikey_aes_decrypt ((password1 + 16), aeskey1);
      //Serial.print("Typing it out on the keyboard...");
      //delay(1000);
      for (int z = 0; z < 32; z++) {
      Serial.print((int)password1[z], HEX);
      //Keyboard.print(password1[z], HEX);
        }
      Serial.println(); //newline
#endif
      //delay(4000);
      */
      blink(3);
      return;
}

void WIPESLOT (byte *buffer)
{
      char cmd = buffer[4]; //cmd or continuation
      int slot = buffer[5];
      int value = buffer[6];
      int length;
      Serial.print("OKWIPESLOT MESSAGE RECEIVED:");
      Serial.println((int)cmd - 0x80, HEX);
      Serial.print("Wiping Slot #");
      Serial.println((int)slot, DEC);
      Serial.print("Value #");
      Serial.println((int)value, DEC);
      delay(1000);
      for (int z = 7; z < 64; z++) {
        buffer[z] = 0x00;
        Serial.print(buffer[z], HEX);
        }
     Serial.print("Overwriting slot with 0s");
   
            switch (value) {
            case 1:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Label Value...");
            yubikey_eeset_label((buffer + 7), EElen_label, slot);
            return;
            break;
            case 2:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Username Value...");
            yubikey_eeset_username((buffer + 7), EElen_username, slot);
            return;
            break;
            case 3:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Additional Character1 Value...");
            yubikey_eeset_addchar1((buffer + 7), slot);
            return;
            break;
            case 4:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing Delay1 to EEPROM...");
            yubikey_eeset_delay1((buffer + 7), slot);
            return;
            break;
            case 5:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Password Value...");
            yubikey_eeset_password((buffer + 7), length, slot);
            return;
            break;
            case 6:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Additional Character2 Value...");
            yubikey_eeset_addchar2((buffer + 7), slot);
            return;
            break;
            case 7:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Delay2 Value...");
            yubikey_eeset_delay2((buffer + 7), slot);
            return;
            break;
            case 8:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping 2FA Type Value...");
            yubikey_eeset_2FAtype((buffer + 7), slot);
            return;
            break;
            case 9:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Writing TOTP Key to EEPROM...");
            yubikey_eeset_totpkey((buffer + 7), EElen_totpkey, slot);
            return;
            break;
            case 10:
            //Set value in EEPROM
            Serial.println(); //newline
            Serial.print("Wiping Yubikey AES Key, Priviate ID, and Public ID...");
            yubikey_eeset_aeskey((buffer + 7), EElen_aeskey);
            yubikey_eeset_private((buffer + 7 + EElen_aeskey));
            yubikey_eeset_public((buffer + 7 + EElen_aeskey + EElen_private), EElen_public);
            return;
            break;
            default: 
            break;
          }
      blink(3);
      return;
}

void SETU2FPRIV (byte *buffer)
{
      Serial.println("OKSETU2FPRIV MESSAGE RECEIVED");

//Set pointer to first empty flash sector
  uintptr_t adr = flashFirstEmptySector();
  unsigned int length = buffer[5];
  Serial.print("Length of U2F private = ");
  Serial.println(length);
 
  yubikey_eeset_U2Fprivlen(buffer + 5); //length is number of bytes
  uint8_t addr[2];
  uint8_t *ptr;
      addr[0] = (int)((adr >> 8) & 0XFF); //convert long to array
      addr[1] = (int)((adr & 0XFF));
  ptr = addr;
  yubikey_eeset_U2Fprivpos(ptr); //Set the starting position for U2F Priv

  for( int z = 6; z <= length && z <= 58; z=z+4, adr=adr+4){
  unsigned long sector = buffer[z] | (buffer[z+1] << 8L) | (buffer[z+2] << 16L) | (buffer[z+3] << 24L);
   //Write long to empty sector 
  Serial.println();
  Serial.printf("Writing to Sector 0x%X, value 0x%X ", adr, sector);
  if ( flashProgramWord((unsigned long*)adr, &sector) ) Serial.printf("NOT ");
  //Serial.printf("successful. Read Value:0x%X\r\n", *((unsigned int*)adr));
  }
  blink(3);
      return;
}

void WIPEU2FPRIV (byte *buffer)
{
      Serial.println("OKWIPEU2FPRIV MESSAGE RECEIVED");
      uint8_t addr[2];
      uint8_t *ptr;
      ptr = addr;
      yubikey_eeget_U2Fprivlen(ptr); //Get the length for U2F private
      int length = addr[0] | (addr[1] << 8);
      Serial.println(length);
      
      yubikey_eeget_U2Fprivpos(ptr); //Get the starting position for U2F private
      //Set adr to position of U2F private in flash
      unsigned long address = addr[1] | (addr[0] << 8L);
      uintptr_t adr = address;
      unsigned long sector = 0x00;
      Serial.println(address);
      for( int z = 0; z <= length; z=z+4, adr=adr+4){
     //Write long to sector 
        Serial.println();
        Serial.printf("Writing to Sector 0x%X, value 0x%X ", adr, sector);
        if ( flashProgramWord((unsigned long*)adr, &sector) ) Serial.printf("NOT ");
        //Serial.printf("successful. Read Value:0x%X\r\n", *((unsigned int*)adr));
      }
      blink(3);
      return;
}

void SETU2FCERT (byte *buffer)
{
      Serial.println("OKSETU2FCERT MESSAGE RECEIVED");
  //Set pointer to first empty flash sector
  uintptr_t adr = flashFirstEmptySector();
  int length = buffer[5] | (buffer[6] << 8);
    if(length <= CERTMAXLENGTH){ 
    Serial.print("Length of certificate = ");
    Serial.println(length);
    yubikey_eeset_U2Fcertlen((buffer + 5)); //length is number of bytes
    }
    else {
      return;
    }
  uint8_t addr[2];
  uint8_t *ptr;
      addr[0] = (int)((adr >> 8) & 0XFF); //convert long to array
      addr[1] = (int)((adr & 0XFF));
  ptr = addr;
  yubikey_eeset_U2Fcertpos(ptr); //Set the starting position for U2F Cert


  //Write packets to flash up to reaching length of certificate

  int n;
  int x;
  int written = 0;
  while(written <= length){ 
    n = RawHID.recv(recv_buffer, 0); // 0 timeout = do not wait
    if (n > 0) {  
      Serial.print(F("\n\nReceived packet"));
      for (x=0; x<64; x++) {
          Serial.print(recv_buffer[x], HEX);
      }
  
      for( int z = 0; z <= 64; z=z+4, adr=adr+4, written=written+4){
         unsigned long sector = buffer[z] | (buffer[z+1] << 8L) | (buffer[z+2] << 16L) | (buffer[z+3] << 24L);
     //Write long to empty sector 
        Serial.println();
        Serial.printf("Writing to Sector 0x%X, value 0x%X ", adr, sector);
        if ( flashProgramWord((unsigned long*)adr, &sector) ) Serial.printf("NOT ");
        //Serial.printf("successful. Read Value:0x%X\r\n", *((unsigned int*)adr));
      }
  }
  }
  
      blink(3);
      return;
}

void WIPEU2FCERT (byte *buffer)
{
      Serial.println("OKWIPEU2FCERT MESSAGE RECEIVED");
      uint8_t addr[2];
      uint8_t *ptr;
      ptr = addr;
      yubikey_eeget_U2Fcertlen(ptr); //Get the length for U2F Cert
      int length = addr[0] | (addr[1] << 8);
      Serial.println(length);
      
      yubikey_eeget_U2Fcertpos(ptr); //Get the starting position for U2F Cert
      //Set adr to position of U2F Cert in flash
      unsigned long address = addr[1] | (addr[0] << 8L);
      uintptr_t adr = address;
      unsigned long sector = 0x00;
      Serial.println(address);
      for( int z = 0; z <= length; z=z+4, adr=adr+4){
     //Write long to sector 
        Serial.println();
        Serial.printf("Writing to Sector 0x%X, value 0x%X ", adr, sector);
        if ( flashProgramWord((unsigned long*)adr, &sector) ) Serial.printf("NOT ");
        //Serial.printf("successful. Read Value:0x%X\r\n", *((unsigned int*)adr));
      }
      blink(3);
      return;
}

void digitalClockDisplay(){
  // digital clock display of the time
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
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void blink(int times){
  
  int i;
  for(i = 0; i < times; i++){
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}

int RNG2(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size) {
      uint8_t val = 0;
      for (unsigned i = 0; i < 8; ++i) {
        int init = analogRead(0);
        int count = 0;
        while (analogRead(0) == init) {
          ++count;
        }

        if (count == 0) {
          val = (val << 1) | (init & 0x01);
        } else {
          val = (val << 1) | (count & 0x01);
        }
      }
      *dest = val;
      ++dest;
      --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
  }


