#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>

#define OUTLEN 128
#define FONT_BYTES 32

typedef enum
{
    PER_COLUMN_TYPE = 0x1,
    PER_ROW_TYPE,
    COLUMN_ROW_TYPE,
    ROW_COLUMN_TYPE,
}arrange_type;

int display(char *incode, int len);

char buffer[16][16];

#define UTF8_LEN 3
//#define DEBUG

int main(int argc, char* argv[])
{
#ifdef DEBUG
        char *in_word = "西";
        unsigned int inbuf_len = strlen(in_word);
        char *pin = in_word;
#else
        char *pin = argv[1];
        unsigned int inbuf_len = strlen(argv[1]);
#endif
        iconv_t convert_fd;
        char outbuf[OUTLEN];
        char *pout = &outbuf[0]; 
        unsigned int outbuf_len = OUTLEN;
        int i;

        memset(outbuf, 0, OUTLEN);
        printf("Originial Data:\n");
        printf("\tpin str: %s, outbuf str:%s\n", pin, outbuf);
        printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
        printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

        // that word may be encoded by UTF-8, so convert it to GB2312
        if(inbuf_len == UTF8_LEN){
            convert_fd = iconv_open("GB2312", "UTF-8");
            if(convert_fd == 0)
                return EXIT_FAILURE;

            int count = iconv(convert_fd, &pin, &inbuf_len, &pout, &outbuf_len);
            printf("iconv count : %d\n", count);
            iconv_close(convert_fd);
        }

        printf("After Converted Data:\n");
        printf("\tpin str: %s, gb2312 str:%s\n", pin, outbuf);
        printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
        printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

        for(i = 0; i < strlen(outbuf); i += 2){
            display(outbuf+i, 2); //use HZK16 to display dot-matrix font in GB2312
        }

        return 0;
}

int get_word_array(char *incode, int len, char *word_array)
{

    FILE *HZK=NULL;
    unsigned char qh,wh;
    unsigned long offset;

    //get area code，chinese encoded start from 0xA1
    //get bit code，chinese encoded start from 0xA1
    qh = incode[0] - 0xa0;
    wh = incode[1] - 0xa0;
    
    //get offset position from HZK16 library when use chinese
    offset = (94*(qh - 1) + (wh - 1)) * 32;     
    printf("GBK Code: 0x%x, 0x%x\n", (unsigned char)incode[0], (unsigned char)incode[1]);
    printf("Area Code:%d,Bit Code:%d\n", qh, wh);

    if((HZK = fopen("HZK16","rb")) == NULL){
        perror("Can't Open hzk16");
        return (EXIT_FAILURE);
    }

    fseek(HZK, offset, SEEK_SET);

    //read font data 16*16 dot-matrix 
    fread(word_array, FONT_BYTES, 1, HZK);    
    
    fclose(HZK);

    return 1;
}

int dot_arrange_type(arrange_type at, 
        unsigned char*matrix_before, unsigned char*matrix_after)
{
    int i, j, k;
    unsigned char tmp[2][16];
    unsigned char matrix_ori[16][2];
    unsigned char temp[16][16];

    memcpy(matrix_ori, matrix_before, 32);
    memset(tmp, 0, 32);

#if 0
    for(i = 0; i < 16; i++){
            for(j = 0; j < 2; j++){
                for(k = 0; k < 8; k++){
                    if(matrix_before[i][j] & (0x80 >> k)){
                        printf("%c", 0x23);
                        font_map[i][j] |= (0x80 >> k);
                    }else{
                        printf("-");
                        font_map[i][j] |= 0x0 << k;
                    }
                }
            }
            printf("\n");
        }
#endif
#if 0
    if(at == COLUMN_ROW_TYPE){
        // two bytes and 8 rows
        for(i = 1; i < 8; i++){
            for(j = 0; j < 8; j++){
                tmp[0][i] |= (matrix_ori[i][0] & (0x1 << j));
            }
        }

        // two bytes and anothre 8 rows
        for(i = 8; i < 16; i++){
            for(j = 0; j < 8; j++){
                tmp[1][i] |= (matrix_ori[i][1] & (0x1 << j));
            }
        }
    }else if(at == ROW_COLUMN_TYPE){

        // 16 rows and the first byte
        for(i = 0; i < 16; i++)
            for(j = 0; j < 8;j++)
                tmp[0][i] |= (matrix_ori[i][0] & (0x1 << j));

        // 16 rows and the second byte
        for(i = 0; i < 16; i++)
            for(j = 0; j < 8; j++)
                tmp[1][i] |= (matrix_ori[i][1] & (0x1 << j));
    }else if(at == PER_ROW_TYPE){
        // Do nothing, because dot-matrix of the HZK16 file
        // also use this type to store.
    }else if(at == PER_COLUMN_TYPE){
    
    }
#endif

    int byt;
    for(k = 0; k < 16; k++){
        byt = 0x0;
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++)
                if(k < 8)
                    byt |= (matrix_ori[j][0] & (1 << j));
                else
                    byt |= (matrix_ori[j][1] & (1 << j));
        }
        tmp[0][k] = byt;
    }
    
    for(k = 0; k < 16; k++){
        byt = 0x0;
        for(i = 0; i < 8; i++){
            for(j = 8; j < 16; j++)
                if(k < 8)
                    byt |= (matrix_ori[j][0] & (1 << j));
                else
                    byt |= (matrix_ori[j][1] & (1 << j));
        }
        tmp[1][k] = byt;
    }

    memcpy(matrix_after, tmp, FONT_BYTES);

    return 1;
}

int display(char *incode, int len)
{
        int i, j, bit;
        char matrix_real[16][2];
        unsigned char font_map[2][16];

        memset(font_map, 0, FONT_BYTES);

        get_word_array(incode, len, matrix_real);

        //display dot-matrix, it has 16 rows.
        //Every row has two parts,each part has 8 bits
        //It displayed from left to right
        printf("Dot-Matrix Size : 16x16\n");
        for(i = 0; i < 16; i++){
            for(j = 0; j < 2; j++){
                for(bit = 0; bit < 8; bit++){
                    if(matrix_real[i][j] & (0x80 >> bit)){
                        printf("%c", 0x23);
                    }else{
                        printf("-");
                    }
                }
            }
            printf("\n");
        }

        dot_arrange_type(ROW_COLUMN_TYPE, matrix_real, font_map);

        for(i = 0;i < 2; i++){
            for(j = 0; j < 16; j++){
                printf("0x%02x ", font_map[i][j]);
            }
            printf("\n");
        }

        return EXIT_SUCCESS;
}
