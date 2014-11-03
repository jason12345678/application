#if !defined(_MD5_CHECK_H_)
#define _MD5_CHECK_H_

char* MD5String( char*  output, char * string,unsigned int len);

int MD5Check( char* md5string, char* toCheckString,unsigned int len); 

char* MD5CheckFile(char* output,char* filepath );

#endif
