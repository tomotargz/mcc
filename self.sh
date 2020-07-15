#!/bin/bash -eu

TMP=tmp-self
mkdir -p $TMP

compileWithMcc2(){
    file=$1
    cat <<EOF > $TMP/$1
typedef struct FILE FILE;
extern FILE *stdout;
extern FILE *stderr;
void *malloc(long size);
void *calloc(long nmemb, long size);
int *__errno_location();
char *strerror(int errnum);
FILE *fopen(char *pathname, char *mode);
long fread(void *ptr, long size, long nmemb, FILE *stream);
int feof(FILE *stream);
static void assert() {}
int strcmp(char *s1, char *s2);
int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
long strlen(char *p);
int strncmp(char *p, char *q);
void *memcpy(char *dst, char *src, long n);
char *strndup(char *p, long n);
int isspace(int c);
char *strstr(char *haystack, char *needle);
long strtol(char *nptr, char **endptr, int base);
typedef struct {
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} __va_elem;
typedef __va_elem va_list[1];
static void va_start(__va_elem *ap) {
  __builtin_va_start(ap);
}
static void va_end(__va_elem *ap) {}
EOF

    grep -v '^#' mcc.h >> $TMP/$1
    grep -v '^#' $1 >> $TMP/$1
    sed -i 's/\bbool\b/int/g' $TMP/$1
    sed -i 's/\berrno\b/*__errno_location()/g' $TMP/$1
    sed -i 's/\btrue\b/1/g; s/\bfalse\b/0/g;' $TMP/$1
    sed -i 's/\bNULL\b/0/g' $TMP/$1
    sed -i 's/\bSEEK_END\b/2/g' $TMP/$1
    sed -i 's/\bSEEK_SET\b/0/g' $TMP/$1
    sed -i 's/\bsize_t\b/int/g' $TMP/$1
    sed -i 's/INT_MAX/2147483647/g' $TMP/$1

    ./mcc $TMP/$1 > $TMP/${1%.c}.s
    gcc -c -o $TMP/${1%.c}.o $TMP/${1%.c}.s
    echo $1 compiled
}

for i in ./*.c; do
    gcc -I. -c -o $TMP/${i%.c}.o $i
done

# compileWithMcc2 codegen.c
# compileWithMcc2 error.c
compileWithMcc2 main.c
# compileWithMcc2 node.c
# compileWithMcc2 parse.c
# compileWithMcc2 tokenize.c
# compileWithMcc2 type.c

gcc -static -o mcc2 $TMP/*.o
echo mcc2 generated
