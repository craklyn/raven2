
#include "general/conf.h"
#include "general/sysdep.h"

#define OLD_SIZE 100
#define NEW_SIZE 128

struct header
{
    long link1;
    long link2;
};

int main(void)
{
    FILE *in, *out;
    char data[NEW_SIZE];
    struct header *header = (struct header *)data;

    memset(data, 0, sizeof(data));

    in = fopen("plrmail", "rb");
    out = fopen("plrmail2", "wb");

    while (fread(data, OLD_SIZE, 1, in) == 1) {
        switch (header->link1) {
            case -1:            /* header block */
                if (header->link2 > 0)
                    header->link2 = header->link2 / OLD_SIZE * NEW_SIZE;
                break;
            case -2:            /* last block */
            case -3:            /* deleted block */
            case -4:            /* parcel block */
                break;
            default:            /* link to next */
                if (header->link1 > 0)
                    header->link1 = header->link1 / OLD_SIZE * NEW_SIZE;
                break;
        }
        fwrite(data, NEW_SIZE, 1, out);
    }

    fclose(out);
    fclose(in);
}
