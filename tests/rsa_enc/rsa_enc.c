#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef unsigned char uchar;
#define NUM_BITS  512
#define PADDING   RSA_PKCS1_PADDING
#define PADDING2  RSA_NO_PADDING
#define PADDING3  RSA_PKCS1_OAEP_PADDING
 
int main() {
  char plain[] = "1234567890";
  char encrypted[NUM_BITS / 8];
  char plain2[sizeof plain];
  int len, rsa_len, out_len;
 
  RSA *rsa = RSA_new();
  BIGNUM *bn = BN_new();
  BN_set_word(bn, RSA_F4);
  // We are assuming the PRNG is automatically seeded
  // (should be the case if system has /dev/urandom)
  RSA_generate_key_ex(rsa, NUM_BITS, bn, NULL);
  BN_free(bn);
 
  len = strlen(plain);
  printf("Plain: %s\n", plain);
 
  rsa_len = RSA_public_encrypt(len, (uchar*)plain, (uchar*)encrypted,
                               rsa, PADDING);
  printf("Encrypted: %d\n", rsa_len);
//  Dump(encrypted, rsa_len);
 
  out_len = RSA_private_decrypt(rsa_len, (uchar*)encrypted, (uchar*)plain2,
                                rsa, PADDING);
  printf("Decrypted: %d\n", out_len);
  printf("Plain: %s\n", plain2);
  rsa_len = RSA_public_encrypt(len, (uchar*)plain, (uchar*)encrypted,
                               rsa, PADDING3);
  printf("Encrypted: %d\n", rsa_len);
//  Dump(encrypted, rsa_len);
 
  out_len = RSA_private_decrypt(rsa_len, (uchar*)encrypted, (uchar*)plain2,
                                rsa, PADDING3);
  printf("Decrypted: %d\n", out_len);
  printf("Plain: %s\n", plain2);
//  Dump(plain2, out_len);
  
  plain2[out_len] = '\0';
 
  if (strcmp(plain, plain2) != 0)
     printf( "RSA test failed\n");
  else {
    FILE *f = fopen("private.txt", "w");
    PEM_write_RSAPrivateKey(f, rsa, NULL, NULL, 0, NULL, NULL);
    f = freopen("public.txt", "w", f);
    PEM_write_RSAPublicKey(f, rsa);
    fclose(f);
  }
 
 
  RSA_free(rsa);
 
  return 0;
}
