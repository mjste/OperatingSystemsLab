#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

void createFilename(int index, char* buffer) {
    char number[10];
    number[0] = 0;
    strcat(buffer, "w");
    sprintf(number, "%d.txt", index);
    strcat(buffer, number);
}


double f(double x) {
    return 4/(x*x+1);
}


void integrate(double width, int index, int n) {
    double from = (double)index/(double)n;
    double to = ((double)index+1)/(double)n;
    int count = (int)((to-from)/width);

    double sum = 0;
    for (int i = 0; i < count; i++) {
        double x1 = from+i*width;
        double x2 = from+(i+1)*width;
        sum += (f(x1)+f(x2))/2*width;
    }
    sum += (f(from+(count-1)*width)+f(to))/2*width;

    char filename[200];
    createFilename(index+1, filename);
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "%lf", sum);
    fclose(fp);
}


int main(int argc, char** argv) {
    if (argc != 3) {
        exit(1);
    }

    if (!isdigit(argv[1][0]) || !isdigit(argv[2][0])) {
        fprintf(stderr, "main: entered arguments are not numbers");
        exit(2);
    }

    double width = atof(argv[1]);
    int n = atoi(argv[2]);

    pid_t pid;
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) {
            integrate(width, i, n);
            return 0;
        }
    }
    
    for (int i = 0; i < n; i++)
        wait(0);

    double sum = 0;
    for (int i = 1; i <= n; i++) {
        char filename[200];
        filename[0] = 0;
        createFilename(i, filename);
        FILE* fp = fopen(filename, "r");
        double value;
        fscanf(fp, "%lf", &value);
        fclose(fp);
        remove(filename);
        sum += value;
    }

    printf("Value: %lf\n", sum);
    return 0;
}