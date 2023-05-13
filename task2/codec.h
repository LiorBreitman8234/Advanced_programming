#ifndef __CODEC__
#define __CODEC__

extern "C"{
    void encrypt(char *s,int key);
    void decrypt(char *s,int key);

}

#endif
