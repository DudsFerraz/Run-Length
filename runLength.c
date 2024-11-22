#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    int value;
    int extension;
}fileNumber;

typedef struct{
    char fileType[3];
    int lines, columns, maxPixelValue;
}fileInfoType;

void cleanBuffer(FILE *fd,char *c){
    while(*c=='\n' || *c==' '){
        *c=fgetc(fd);
    }
}

fileNumber **createMatrix(int lines,int columns){
    fileNumber **newMatrix = malloc(sizeof(fileNumber*)*lines);
    fileNumber *data = malloc(sizeof(fileNumber)*lines*columns);

    for(int i=0;i<lines;i++){
        newMatrix[i] = &data[i*columns];
    }
    return newMatrix;
}

void getFileInfo(FILE *fd,fileInfoType *fileInfo){
    fscanf(fd,"%s",&fileInfo->fileType);
    fscanf(fd,"%d %d",&fileInfo->columns,&fileInfo->lines);
    fscanf(fd,"%d",&fileInfo->maxPixelValue);
}

void readP2File(FILE *fd,fileInfoType fileInfo,fileNumber **matrixP2){
    for(int i=0;i<fileInfo.lines;i++){
        for(int j=0;j<fileInfo.columns;j++){
            fscanf(fd,"%d",&matrixP2[i][j].value);
            matrixP2[i][j].extension=1;
        }
    }
}

void readP8File(FILE *fd,fileInfoType fileInfo,fileNumber **matrixP8){
    for(int i=0;i<fileInfo.lines;i++){
        int counter=0;
        for(int j=0;j<fileInfo.columns;){
            char nextChar=fgetc(fd);
            cleanBuffer(fd,&nextChar);
            if(nextChar=='@'){
                fscanf(fd,"%d",&matrixP8[i][counter].value);
                fscanf(fd,"%d",&matrixP8[i][counter].extension);
            }else{
                ungetc(nextChar,fd);
                fscanf(fd,"%d",&matrixP8[i][counter].value);
                matrixP8[i][counter].extension=1;
            }
            j+=matrixP8[i][counter].extension;
            counter++;
        }
        if(counter!=fileInfo.columns){
            matrixP8[i][counter].value=-1;
        }
    }
}

void P2ToP8(fileNumber **matrixP2,fileInfoType fileInfo,fileNumber **matrixP8){
    for(int i=0;i<fileInfo.lines;i++){
        int counter=0, extension=1;
        for(int j=0;j<fileInfo.columns;j++){
            if(matrixP2[i][j].value==matrixP2[i][j+1].value && j<(fileInfo.columns-1)){
                extension++;
            }else{
                matrixP8[i][counter].value=matrixP2[i][j].value;
                matrixP8[i][counter].extension=extension;
                extension=1;
                counter++;
            }
        }
        if(counter!=fileInfo.columns){
            matrixP8[i][counter].value=-1;
        }
    }
}

void P8ToP2(fileNumber **matrixP8,fileInfoType fileInfo,fileNumber **matrixP2){
    for(int i=0;i<fileInfo.lines;i++){
        int counterP8=0, counterP2=0;
        for(int j=0;j<fileInfo.columns;){
            for(int ii=0;ii<matrixP8[i][counterP8].extension;ii++){
                matrixP2[i][counterP2].value=matrixP8[i][counterP8].value;
                counterP2++;
            }
            j+=matrixP8[i][counterP8].extension;
            counterP8++;
        }
    }
}

void P8Output(fileNumber **matrixP8,fileInfoType fileInfo,FILE *fdOut){
    fprintf(fdOut,"P8\n");
    fprintf(fdOut,"%d %d\n",fileInfo.columns,fileInfo.lines);
    fprintf(fdOut,"%d\n",fileInfo.maxPixelValue);

    for(int i=0;i<fileInfo.lines;i++){
        for(int j=0;j<fileInfo.columns;j++){
            if(matrixP8[i][j].value==-1){
                break;
            }
            if(matrixP8[i][j].extension>3){
                fprintf(fdOut,"@ %d %d ",matrixP8[i][j].value,matrixP8[i][j].extension);
            }else{
                for(int ii=0;ii<matrixP8[i][j].extension;ii++){
                    fprintf(fdOut,"%d ",matrixP8[i][j].value);
                }
            }
        }
        fprintf(fdOut,"\n");
    }
}

void P2Output(fileNumber **matrixP2,fileInfoType fileInfo,FILE *fdOut){
    fprintf(fdOut,"P2\n");
    fprintf(fdOut,"%d %d\n",fileInfo.columns,fileInfo.lines);
    fprintf(fdOut,"%d\n",fileInfo.maxPixelValue);

    for(int i=0;i<fileInfo.lines;i++){
        for(int j=0;j<fileInfo.columns;j++){
            fprintf(fdOut,"%d ",matrixP2[i][j].value);
        }
        fprintf(fdOut,"\n");
    }
}

void printMatrix(fileNumber **matrix,fileInfoType fileInfo){
    for(int i=0;i<fileInfo.lines;i++){
        for(int j=0;j<fileInfo.columns;j++){
            if(matrix[i][j].value==-1){
                break;
            }
            printf("%d [%d] ",matrix[i][j].value,matrix[i][j].extension);
        }
        printf("\n");
    }
}

void freeMatrix(fileNumber **matrix){
    free(matrix[0]);
    free(matrix);
}

int main(int argc,char *argv[]){
    FILE *fd, *fdOut;
    fileInfoType fileInfo;
    fileNumber **matrixP2, **matrixP8;

    fd = fopen(argv[1],"r");
    fdOut = fopen(argv[2],"w");

    if(fd==NULL){
        printf("Erro ao abrir o arquivo\n");
        return -1;
    }

    getFileInfo(fd,&fileInfo);
    matrixP2 = createMatrix(fileInfo.lines,fileInfo.columns);
    matrixP8 = createMatrix(fileInfo.lines,fileInfo.columns);

    if(strcmp(fileInfo.fileType,"P2")==0){
        readP2File(fd,fileInfo,matrixP2);
        P2ToP8(matrixP2,fileInfo,matrixP8);
        P8Output(matrixP8,fileInfo,fdOut);
    }

    if(strcmp(fileInfo.fileType,"P8")==0){
        readP8File(fd,fileInfo,matrixP8);
        P8ToP2(matrixP8,fileInfo,matrixP2);
        P2Output(matrixP2,fileInfo,fdOut);
    }

    freeMatrix(matrixP2);
    freeMatrix(matrixP8);
    fclose(fd);
    fclose(fdOut);

    return 0;
}