#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "main.h"

unsigned char cupbuf[0x800];

int extract_cias(char *contents_path, unsigned long long *tidlist, unsigned int tidindex) {
    FILE *contents, *cia;
    char *contents_buf;
    size_t cia_size;
    Contents_header *cont_hdr;
    char *outdir = "updates";

    // TODO: Replace this with something better
    char out_str[PATH_MAX - 1] = {0};

    mkdir(outdir);
    contents = fopen(contents_path, "rb");

    if (!contents) return 1;

    cont_hdr = (Contents_header*)malloc(sizeof(Contents_header));
    fseek(contents, 0, SEEK_SET);
    fread(cont_hdr, sizeof(Contents_header), 1, contents);

    if(memcmp(cont_hdr->magic, (uint8_t[]){'C','O','N','T'}, 4)) {
        printf("Invalid Contents\n");
        free(cont_hdr);
        return 1;
    }

    for (int i = 0; i < tidindex; i++) {
        snprintf(out_str, PATH_MAX, "%s/%016"PRIx64".cia", outdir, tidlist[i]);
        cia = fopen(out_str, "wb");
		cia_size = cont_hdr->entries[i].offset_end - cont_hdr->entries[i].offset;
        contents_buf = malloc(cia_size);

        // TODO: make this better
        fseek(contents, cont_hdr->entries[i].offset + sizeof(Contents_header) - 2048, SEEK_SET);
        fread(contents_buf, cia_size, 1, contents);

        printf("Writing %s (%"PRIi64" KB)\n", out_str, cia_size / 1024);
        fwrite(contents_buf, cia_size, 1, cia);
        fclose(cia);
        free(contents_buf);
    }
    
    free(cont_hdr);
}

int main(int argc, char *argv[]) {
    unsigned int tidindex;
    unsigned int total_titles = 0;
    unsigned long long *tidlist = (unsigned long long*)cupbuf;
    FILE *cuplist;

    if(argc!=3) {
        printf("Usage: ctc <cuplist> <contents.cnt>\n");
        return 0;
    }
    
    memset(cupbuf, 0, 0x800);

    cuplist = fopen(argv[1], "rb");
    fread(cupbuf, 1, 0x800, cuplist);
    fclose(cuplist);

    for(tidindex = 0; tidindex < 0x100; tidindex++) {
        if (tidlist[tidindex] == 0)
            break;
        total_titles++;
    }

    printf("Indexed titles: %u\n", total_titles);
    extract_cias(argv[2], tidlist, tidindex);

    return 0;
}