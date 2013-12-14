#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>

#define OUTLEN 128

int display(char *incode, int len);

int main(int argc, char* argv[])
{
        char *string = "西";
        iconv_t convert_fd;
        unsigned int inbuf_len = strlen(string);
        //int inbuf_len = strlen(argv[1]);
        char outbuf[OUTLEN];
        char *pin = string;
        //char *pin = argv[1];
        char *pout = &outbuf[0];  //用"pout=&outbuf" 会引发SIGSERV信号，导致段错误
        unsigned int outbuf_len = OUTLEN;

        memset(outbuf, 0, OUTLEN);
        printf("Originial Data:\n");
        printf("\tpin str: %s, outbuf str:%s\n", pin, outbuf);
        printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
        printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

        convert_fd = iconv_open("GB2312", "UTF-8");
        if(convert_fd == 0)
            return EXIT_FAILURE;

        int count = iconv(convert_fd, &pin, &inbuf_len, &pout, &outbuf_len);
        printf("iconv count : %d\n", count);
        iconv_close(convert_fd);

        printf("After Converted Data:\n");
        printf("\tpin str: %s, gb2312 str:%s\n", pin, outbuf );
        printf("\tinbuf_len=%d, outbuf_len=%d\n", inbuf_len, outbuf_len);
        printf("\tstrlen(outbuf)= %d\n", strlen(outbuf));

        int i,j;
        for(i = 0; i < strlen(outbuf); i += 2){
            display(outbuf+i, 2); //use HZK16 to display dot-matrix font in GB2312
        }

        return EXIT_SUCCESS;
}

int display(char *incode, int len)
{
        int i, j, k;
        char mat[16][2];
        FILE *HZK=NULL;
        unsigned char qh,wh;
        unsigned long offset;
        char font_map[2][16];

        memset(font_map, 0, sizeof(font_map));

        qh = incode[0] - 0xa0;//获得区码，中文编码从0xA1开始
        wh = incode[1] - 0xa0;   //获得位码，中文编码从0xA1开始
        offset = (94*(qh - 1) + (wh - 1)) * 32; //得到汉字在HZK16字库中的字节偏移位置
        printf("GBK Code: 0x%x, 0x%x\n", (unsigned char)incode[0], (unsigned char)incode[1]);
        printf("区码：%d,位码：%d\n",qh,wh);

        if((HZK = fopen("HZK16","rb")) == NULL){
           perror("Can't Open hzk16");
           return (EXIT_FAILURE);
        }

        fseek(HZK, offset, SEEK_SET);
        fread(mat,32,1,HZK);//读取汉字的16*16点阵字模
        fclose(HZK);

        printf("Dot-Matrix Size : 16x16\n");

        //display dot-matrix
        for(i = 0; i < 16; i++){
            for(j = 0; j < 2; j++){
                for(k = 0; k < 8; k++){
                    if(mat[i][j] & (0x80 >> k)){
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

        for(i = 1; i < 8; i++){
            for(j = 0; j < 8; j++){
                font_map[0][i] |= (mat[i][0] & (0x1 << j));
            }
        }

        for(i = 8; i < 16; i++){
            for(j = 0; j < 8; j++){
                font_map[1][i] |= (mat[i][1] & (0x1 << j));
            }
        }


        for(i = 0;i < 2; i++){
            for(j = 0; j < 16; j++){
                printf("0x%x ", font_map[i][j]);
            }
            printf("\n");
        }

        return EXIT_SUCCESS;
}
