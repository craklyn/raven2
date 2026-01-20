
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"

void convert(char *filename)
{
  int count = 0, worn = 0, moved = 0;
  RentInfo rentinfo;
  ObjFileElem object;
  FILE *f;

  f = fopen(filename, "r+b");
  if (!f) {
    fprintf(stderr, "Cannot open %s for readwrite: ", filename);
    perror("");
    return;
  }
  if (fread(&rentinfo, sizeof(rentinfo), 1, f) != 1) {
    fprintf(stderr, "File %s has an incomplete rent header\n", filename);
    fclose(f);
    return;
  };
  while (!feof(f)) {
    int success = fread(&object, sizeof(object), 1, f);
    if (feof(f)) break;
    if (success != 1 && ferror(f)) {
      fprintf(stderr, "File %s has a truncated object (%d read OK)\n",
          filename, count);
      fclose(f);
      return;
    }
    if (object.position > 19) {
      fprintf(stderr, "WARNING: For %s, item #%d is in high slot number %d!\n",
          filename, count, object.position);
    } else {
      long mark = ftell(f);
      if (object.position > 6) object.position += 2, moved++;
      if (object.position > 9) object.position += 1;
      fseek(f, -sizeof(object), SEEK_CUR);
      if (fwrite(&object, sizeof(object), 1, f) != 1) {
        fprintf(stderr, "%s: could not write #%d back: ", filename, count);
        perror("");
      };
      fseek(f, mark, SEEK_SET);
    }
    count++;
  }
  fclose(f);
  printf("File %s successfully converted, %d objects, %d moved\n",
      filename, count, moved);
}

int main(int argc, char **argv)
{
  int file;

  for (file = 1; file < argc; file++) {
    convert(argv[file]);
  }
  return 0;
}
