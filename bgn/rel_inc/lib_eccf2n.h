/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/
#ifndef _LIB_ECCF2N_H
#define _LIB_ECCF2N_H

#include "lib_type.h"

/**
*   for test only
*
*   to query the status of ECCF2N Module
*
**/
void print_ecc_f2n_status(LOG *log);

/**
*
* start ECCF2N module
*
**/
UINT32 ecc_f2n_start( const BIGINT *f_x,
                    const ECF2N_CURVE *curve,
                    const BIGINT *cofactor,
                    const BIGINT *order,
                    const EC_CURVE_POINT *base_point,
                    const UINT32 ( *random_generator)( BIGINT * random),
                    const UINT32 (*do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash)
                    );

/**
*
* end ECCF2N module
*
**/
void ecc_f2n_end(const UINT32 eccf2n_md_id);

/**
*
*   compute the public key according to the given private key
*   on the elliptic curve(ec) refered by the Module ID ecf2n_md_id
*
*   Note:
*       public key = private key * basepoint
*   where basepoint is on the elliptic curve(ec)
*   and refered by the Module ID ecf2n_md_id
*
**/
UINT32 ecc_f2n_get_public_key(const UINT32 eccf2n_md_id, const BIGINT *privatekey,EC_CURVE_POINT *publickey);

/**
*
*   default random generator function to generate a random
*
**/
UINT32 ecc_f2n_random_generator_default( BIGINT * random);

/**
*
*   call the random generator function to generate a random as the privatekey
*
*   note:
*       the bigint privatekey should be less than the order of the ec
*
**/
void ecc_f2n_rnd_private_key(const UINT32 eccf2n_md_id, BIGINT * privatekey);

/**
*
*   generate a random key pair of ECDSA
*   the private key of the key pair is a random number which is less than the ec order
*   and the public key = private key * basepoint
*
**/
UINT32 ecc_f2n_generate_keypair(const UINT32 eccf2n_md_id, ECC_KEYPAIR *keypair);

/**
*
*   the message (plaintxt) length is not more than (deg(fx) - nlossbits).
*   where nlossbits is the number of loss bits.
*   let m = message,then
*   consider {k * m + i, i = 0... (k-1)}
*   where k = 2 ^{nlossbits}
*   select the minmum from the set s.t.
*   when regard it as the x-coordinate of a point, the point is on the subgroup of ec
*   which is defined and refered by the module id eccf2n_md_id and where the order of
*   the subgroup is eccf2n_order.
*
*   return this point
*
*   note:
*       here let k = 4 which means we have to loss 2 bits of the message
*   relative to the length of the x-coordinate of the ec point.
*
**/
int ecc_f2n_encoding(const UINT32 eccf2n_md_id, const UINT8 * message,const UINT32 messagelen, EC_CURVE_POINT * msgpoint);

/**
*
*   from the encoding rule, we know the message is embedded as
*   the x-coordinate of the ec point;
*   furthermore, the message is the higher bits of the x-coordinate.
*
*   so, the decoding rule is to copy the higher bits the x-coordinate
*   of the point to message.
*
*   note:
*   the number of higher bits is determined by the degree of generator polynomial
*   of the base field F_{2^n},i.e, ecc_f2n_n
*
*   return the message ( plaintxt )
*
**/
void ecc_f2n_decoding(const UINT32 eccf2n_md_id, const EC_CURVE_POINT * msgpoint, const UINT32 maxmessagelen,UINT8 *message, UINT32 *messagelen );

/**
*
* Encryption:
*   ( kG, m + kP )
* where
*   k : random number
*   G : base point over EC
*   r : private key, integer number
*   P : public key ( = rG )
*   m : message point/ plain text
*
* define
*   c1 = kG
*   c2 = m + kP
*
*   return (c1,c2)
**/
void ecc_f2n_encryption(const UINT32 eccf2n_md_id,
                        const EC_CURVE_POINT * publickey,
                        const EC_CURVE_POINT * msgpoint,
                        EC_CURVE_POINT * c1,
                        EC_CURVE_POINT * c2);

/**
*
* Decryption:
*   m = c2 - r * c1
* where c1 = kG, c2 = m + kP and
*   k : random number
*   G : base point over EC
*   r : private key, integer number
*   P : public key ( = rG )
*   m : message point/ plain text
*
**/
void ecc_f2n_decryption(const UINT32 eccf2n_md_id,
                        const BIGINT * privatekey,
                        const EC_CURVE_POINT * c1,
                        const EC_CURVE_POINT * c2,
                        EC_CURVE_POINT * msgpoint);

/**
*
*   return a constant....
*   (this function should be replaced by a user defined interface)
*
**/
UINT32 ecc_f2n_do_hash_default(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

/**
*
*   generate the ecc signature of the message with the private key
*   return the signature
*
*   Algorithm:
*   Input: message m , private key d , elliptic curve ec and the base point G
*   Output: signature
*   0.  let the order of the ec be n
*   1.  compute the hash value h of m
*   2.  generate a random number k where 1 <= k < n
*   3.  compute kG = (x1,y1)
*   4.  compute r = x1 mod n
*   5.  compute s = (h + d * r)/k mod n
*   6.  if r = 0 or s = 0, then goto 2.; else goto 7.
*   7.  signature = (r, s)
*   8.  return signature
*
**/
UINT32  ecc_f2n_signate(const UINT32 eccf2n_md_id,
                    const BIGINT * privatekey,
                    const UINT8 * message,
                    const UINT32 messagelen,
                    ECC_SIGNATURE *signature);

/**
*
*   verify the ecc signature of the message with the public key
*   return TRUE or FALSE
*
*   Algorithm:
*   Input: message m , public key Q , elliptic curve ec and the base point G
*   Output: TRUE or FALSE
*   0.  let the order of the ec be n
*   1.  compute the hash value h of m
*   2.  compute w = 1/s mod n
*   3.  compute u1 = h * w mod n
*   4.  compute u2 = r * w mod n
*   5.  compute R = u1*G + u2 * Q = (x1,y1)
*   6.  if R is the infinite point, then return FALSE.
*   7.  compute v = x1 mod n
*   8.  if v = r, then return TRUE; else return FALSE.
*
**/
EC_BOOL ecc_f2n_verify(const UINT32 eccf2n_md_id,
                const EC_CURVE_POINT * publickey,
                const UINT8 * message,
                const UINT32 messagelen,
                const ECC_SIGNATURE *signature);

#endif/*_LIB_ECCF2N_H*/
#ifdef __cplusplus
}
#endif/*__cplusplus*/
