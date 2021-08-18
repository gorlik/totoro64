#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define BUF_SIZE (32*1024)

unsigned char buffer[BUF_SIZE];
unsigned char linebuffer[1024];


int main(int argc, char *argv[])
{
  int i,j;
  int rsize;
  int start, len, stop;
  FILE *f;
  char *fname, *varname, *segname;

  if(argc<1 || argc>4) {
    fprintf(stderr,"usage: asm_tobin filename [start [ size]]\n");
    return 0;
  }

  if(argc>3) len=atoi(argv[3]);
  else len=0;

  if(argc>2) start=atoi(argv[2]);
  else start=0;

  f=fopen(argv[1],"rb");
  if(f==NULL) {
    fprintf(stderr,"can't open file: %s\n",argv[1]);
    return 0;
  }

  rsize=0;
  while(fgets(linebuffer,1024,f)) {
    if(linebuffer[0]==';') continue;
    for(j=0;j<strlen(linebuffer);j++)
      if(linebuffer[j]!=',' && !isdigit(linebuffer[j]))
	linebuffer[j]=' ';
    
    unsigned char *p;
    //printf("i: %d read: %s\n",rsize,linebuffer);
    p=strtok(linebuffer," ,");
    while(p) {
       buffer[rsize++]=atoi(p);
       p=strtok(NULL," ,");
    }
  }

//  fsize=fread(buffer, 1, BUF_SIZE, f);
  fclose(f);

  fprintf(stderr,"read %d bytes\n",rsize);

  //  printf("/* automatically generated from: %s */\n\n",argv[1]);


  //  printf("const unsigned char %s[%d] = {\n",varname,fsize);

  if(len) stop=start+len;
  else stop=rsize;

  if(stop>rsize) {
    fprintf(stderr,"error: start+len can't be larger than rsize\n");
    return 0;
  }
  
  for(i=start;i<stop;i++) {
    printf("%c",buffer[i]);
    //      if(i%16==0) printf("  ");
    //      printf("0x%02X",buffer[i]);
      //      if(i%16==15) printf(",\n");
      //      else printf(", ");
  }

  // printf("};\n");

  return 0;
}
