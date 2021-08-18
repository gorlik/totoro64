#include <stdio.h>

#define BUF_SIZE (32*1024)

unsigned char buffer[BUF_SIZE];

int main(int argc, char *argv[])
{
  int i,j;
  int fsize;
  FILE *f;
  char *fname, *varname, *segname;

  if(argc<1 || argc>4) {
    fprintf(stderr,"usage: bin_to_c filename [variable name [ segment]]\n");
    return 0;
  }

  if(argc>3) segname=argv[3];
  else segname=NULL;

  if(argc>2) varname=argv[2];
  else varname="table";

  f=fopen(argv[1],"rb");
  if(f==NULL) {
    fprintf(stderr,"can't open file: %s\n",argv[1]);
    return 0;
  }
  fsize=fread(buffer, 1, BUF_SIZE, f);
  fclose(f);

  fprintf(stderr,"file size:%d\n",fsize);

  printf("/* automatically generated from: %s */\n\n",argv[1]);

  if(segname)
	  printf("#pragma rodata-name (\"%s\")\n\n",segname);


  printf("const unsigned char %s[%d] = {\n",varname,fsize);

  for(i=0;i<fsize;i++) {
      if(i%16==0) printf("  ");
      printf("0x%02X",buffer[i]);
      if(i%16==15) printf(",\n");
      else printf(", ");
  }

  printf("};\n");

  return 0;
}
