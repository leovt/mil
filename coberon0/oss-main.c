#include <stdio.h>
#include <stdlib.h>

#include "oss.h"

int main(int argc, char** argv){
  if (argc<2) {
    fprintf(stderr, "Need a filename!\n");
    return EXIT_FAILURE;
  }
  Init(fopen(argv[1], "r"));
  symbol sym;
  do {
    sym = Get();
    if (sym==ident){
      printf("sym=%2d %-12s %s\n", sym, symbol_name[sym], id);
    }
    else if (sym==number){
      printf("sym=%2d %-12s %d\n", sym, symbol_name[sym], val);
    }
    else {
      printf("sym=%2d %s\n", sym, symbol_name[sym]);
    }
  } while(sym != eof);
  return EXIT_SUCCESS;
}
