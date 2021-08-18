#include <stdio.h>

unsigned char buffer[2048];

int main(int argc, char *argv[])
{
  int i,j;
  FILE *f;

  if(argc!=2) {
    printf("usage: make_table filename\n");
    return 0;
  }

  f=fopen(argv[1],"rb");
  fread(buffer, 2048, 1, f);
  fclose(f);

#if 0

  printf(".export         _charset\n");
//  printf(".segment \"TABLES\"\n");
  printf("_charset:\n");


  for(j=0;j<8;j++) {
    for(i=0;i<256;i++) {
      if(i%16==0) printf(".byte ");
      printf("$%02X",buffer[i*8+j]);
      if(i%16==15) printf("\n");
      else printf(", ");
    }
  }

#else

  for(j=0;j<8;j++) {
    for(i=0;i<256;i++) {
//      if(i%16==0) printf(".byte ");
      printf("%c",buffer[i*8+j]);
//      if(i%16==15) printf("\n");
//      else printf(", ");
    }
  }

#endif
  return 0;
}
