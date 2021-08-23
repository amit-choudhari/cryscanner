/**
  AES encryption/decryption demo program using OpenSSL EVP apis
  gcc -Wall openssl_aes.c -lcrypto

  this is public domain code. 

  Saju Pillai (saju.pillai@gmail.com)
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>

#define AES_BLOCK_SIZE 128

/**
 * Create a 256 bit key and IV using the supplied key_data. salt can be added for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 **/
int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, unsigned char *key, unsigned char *iv)
{
  int i, nrounds = 5;
  
  /*
   * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
   * nrounds is the number of times the we hash the material. More rounds are more secure but
   * slower.
   */
  i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);

  if (i != 32) {
    printf("Key size is %d bits - should be 256 bits\n", i);
    return -1;
  }
  /*
  for(int i=0; i<32; i++)
	  printf("%x",key[i]);
  printf("\n");
  for(int i=0; i<32; i++)
	  printf("%x",iv[i]);
  printf("\n");
  */

  EVP_CIPHER_CTX_init(e_ctx);

  return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
  /* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
  int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
  unsigned char *ciphertext = malloc(c_len);

  /* update ciphertext, c_len is filled with the length of ciphertext generated,
    *len is the size of plaintext in bytes */
  EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

  /* update ciphertext with the final remaining bytes */
  EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

  *len = c_len + f_len;
  return ciphertext;
}

/*
 * Decrypt *len bytes of ciphertext
 */
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
  /* plaintext will always be equal to or lesser than length of ciphertext*/
  int p_len = *len, f_len = 0;
  unsigned char *plaintext = malloc(p_len);
  
  EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
  EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

  *len = p_len + f_len;
  return plaintext;
}

int encrypt(unsigned int *salt, char *input, unsigned char **ciphertext, int *len, unsigned char *key_data, int key_data_len)
{
  EVP_CIPHER_CTX *en;
  int olen;
  unsigned char key[32], iv[32];

  en = EVP_CIPHER_CTX_new();

  /* gen key and iv. init the cipher ctx object */
  if (aes_init(key_data, key_data_len, (unsigned char *)&salt, en, key, iv)) {
    printf("Couldn't initialize AES cipher\n");
    return -1;
  }
  EVP_EncryptInit_ex(en, EVP_aes_256_cbc(), NULL, key, iv);

  *len = strlen(input)+1;
  
  *ciphertext = aes_encrypt(en, (unsigned char *)input, len);

  EVP_CIPHER_CTX_free(en);
  return 0;
}

int decrypt(unsigned int *salt, char *ciphertext, unsigned char **plaintext, int *len, unsigned char *key_data, int key_data_len)
{
  EVP_CIPHER_CTX *de;
  unsigned char key[32], iv[32];
    
  de = EVP_CIPHER_CTX_new();

  /* gen key and iv. init the cipher ctx object */
  if (aes_init(key_data, key_data_len, (unsigned char *)&salt, de, key, iv)) {
    printf("Couldn't initialize AES cipher\n");
    return -1;
  }
  EVP_DecryptInit_ex(de, EVP_aes_256_cbc(), NULL, key, iv);

  *plaintext = (char *)aes_decrypt(de, ciphertext, len);

  EVP_CIPHER_CTX_free(de);
  return 0;
}

int main(int argc, char **argv)
{
  unsigned int salt[] = {0xdead, 0xbeef};
  char *input[] = {"a", "abcd", "this is a test", "this is a bigger test", 
                   "\nWho are you ?\nI am the 'Doctor'.\n'Doctor' who ?\nPrecisely!",
                   NULL};
  unsigned char *ciphertext;
  unsigned char *plaintext;
  unsigned char *key_data;
  int key_data_len, len, olen;

  /* the key_data is read from the argument list */
  key_data = (unsigned char *)argv[1];
  key_data_len = strlen(argv[1]);

  for (int i = 0; input[i]; i++) {
    encrypt(salt, input[i], &ciphertext, &olen, key_data, key_data_len);
    decrypt(salt, ciphertext, &plaintext, &olen, key_data, key_data_len);
    len = strlen(input[i])+1;
    if (strncmp(plaintext, input[i], len)) 
      printf("FAIL: enc/dec failed for \"%s\"\n", input[i]);
    else 
      printf("OK: enc/dec ok for \"%s\"\n", plaintext);
  
    free(ciphertext);
    free(plaintext);
  }
  EVP_CIPHER_CTX *de = EVP_CIPHER_CTX_new();

  return 0;
}
