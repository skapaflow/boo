/*
#getline {
	gcc -o boo boo.c -s
	-Wwrite-strings -Wextra -Werror -Wall
	-Wno-missing-braces -Wno-missing-field-initializers -Wformat=2
	-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith
	-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline
	-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code
	-Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls
	| del "C:\Users\Alber\Desktop\Anthropoid Shell\bin\boo.exe"
	| mv boo.exe "C:\Users\Alber\Desktop\Anthropoid Shell\bin\boo.exe"
	| cd repo_test
	| boo
	| cd..
}
*/

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32)
	#include <windows.h>
	#include <conio.h>

	#define WIN_OS

	#define gray 8
	#define blue 9
	#define green 10
	#define cyan 11
	#define red 12
	#define purple 13
	#define yellow 14
	#define white 15

	int output_state_shell;
#endif

#ifdef WIN_OS
	#define OS_BAR "\\"
	#define OSxBAR 0x5c
	#define ftell _ftelli64
#else
	#define OS_BAR "/"
	#define OSxBAR 0x2f
	#define ftell ftello
#endif

#define BOOMIN 128
#define BOOMID 512
#define BOOMAX 1024

#define UNMODIFIED 0
#define DELETED    1
#define INSERTED   2

#define flag printf("flag:%d\n", __LINE__), getch(); /* debug purpose */

#define xmalloc(c,v,n) xalloc(c,v,malloc(sizeof(c) * n), "malloc")
#define xcalloc(c,v,n) xalloc(c,v,calloc(n, sizeof(c)), "calloc")
#define xrealloc(c,v,n) xalloc(c,v,realloc(v, sizeof(c) * n), "realloc")
#define xfree(f) if (f != NULL) free(f), f = NULL
#define xfclose(f) if (f != NULL) fclose(f), f = NULL
#define xmemset(c,v,i,n) memset((c*)v, i, sizeof(c)*n)
#define xalloc(c,v,m,s) \
	if ((v = (c*) m) == NULL) {\
		fprintf(stderr, "\n [!] error_"s":\"%s\":%d\n", __FILE__, __LINE__);\
		getchar();\
		exit(EXIT_FAILURE);\
	}\

const char *hashtype = "0123456789abcdefghijklmnopqrstuvwxyz";
const char *newpath = "  o---o---o\n       \\\n        o---{new}";
const char *delpath = "  o---o---o\n       \\\n        o---{del}";

const char *boo[] = {
/*01*/"D",".boo",
/*03*/"F",".booignore.ini",
/*05*/"F",".boo"OS_BAR"log.txt",
/*07*/"D",".boo"OS_BAR"monitor",
/*09*/"D",".boo"OS_BAR"monitor"OS_BAR"master",
/*11*/"F",".boo"OS_BAR"monitor"OS_BAR".master",
/*13*/"D",".boo"OS_BAR"room",
/*15*/"F",".boo"OS_BAR"room"OS_BAR"register",
/*17*/"F",".boo"OS_BAR"room"OS_BAR"user",
/*19*/"D",".boo"OS_BAR"version",
/*20*/"F",".boo"OS_BAR"room"OS_BAR"note",
	NULL
};

const char boohelp[] =
	"\n"
	"usage: boo <command>\n\n"
	" install           Install boo repository\n"
	" monitor           Show monitored files\n"
	" remove            unstall boo repository\n"
	" submit            Save code version\n"
	" *<arm>            Change working arm\n"
	" merge             Merge from one arm to another\n"
	" name              Add repo name\n"
	" move              Move to monitoring\n"
	" undo              Undo code modification\n"
	" ver               Show version\n"
	" arm               Create new fork\n"
	" log               Show saved versions of the code\n"
	" del               Delete file with status \"DEL\"\n"
	" dif ('src/file')  Show all file difference or select one file \n"
	" up <tag>          Revert to a certain version\n";

const char boologo[] =
  "\n BBBBBBBBBBBBB          BBBBBBB            BBBBBBB\n"
	" BBBBBBBBBBBBBBB     BBBBBBBBBBBBB      BBBBBBBBBBBBB\n"
	" BBBB    BBBBBBBB   BBBBBBBBBBBBBBB    BBBBBBBBBBBBBBB\n"
	" BBBBB     BBBBBB  BBBBBBB   BBBBBBB  BBBBBBB   BBBBBBB\n"
	" BBBB    BBBBBBB   BBB    BBB    BBB  BBB    BBB    BBB\n"
	" BBBBBBBBBBBBB     B     BBBBB     B  B     BBBBB     B\n"
	" BBBB     BBBBBB   BBB    BBB    BBB  BBB    BBB    BBB\n"
	" BBBBB      BBBBB  BBBBBBB   BBBBBBB  BBBBBBB   BBBBBBB\n"
	" BBBB     BBBBBBB   BBBBBBBBBBBBBBB    BBBBBBBBBBBBBBB\n"
	" BBBBBBBBBBBBBBB     BBBBBBBBBBBBB      BBBBBBBBBBBBB\n"
	" BBBBBBBBBBBBB          BBBBBBB            BBBBBBB\n"
	"\n Version Control System [v1.0 (2020)]\n";

FILE *ftree = NULL;
FILE *reg_file = NULL;
FILE *logfile = NULL;

typedef struct {
	int lot;
	int edit;
	char *string;
} DIFF;

struct tm *TM;

int file_number;
char *null = NULL;
char *log_user = NULL;
char *arm_user = NULL;
char *path_tree = NULL;
char *data_move = NULL;
char *global_buf = NULL;

char *cat (const char *, ...);
char *strrmc (char *, int);
char *strend (const char *, int);
char *subchr (char *, int, int);
char *strtoupper (const char *);
int64_t size_file (FILE *);
char *hash (char *);
char *value_patter (int64_t);

int boo_print (FILE *, int, const char *, ...);
void boo_error (int, const char *, const char *, const char *, ...);
int boo_transport (char *, int, int);
int boo_tree (const char *, char **);

char *boo_crud (const char *, char *, const char *, const char *);
void boo_typewriter (int, char **);
int boo_ignore (const char *);
void boo_listdir (const char *, const char *, FILE *);
void boo_status (int);
void boo_show (bool, const char *);

DIFF *generate_partial_diff(int **, int, int, char **, char **, int);
int **compute_table(int *, int *, char **, char **, int, int, int);
void compare (DIFF *, char **, int, char **, int);
char **file_get_contents (const char *, int *);
bool boo_diff (const char *, const char *, bool);

void boo_name (char *);
void boo_move (const char *);
void boo_undo (void);
void boo_monitor (void);
void boo_submit (char *);
void boo_arm (char **);
void boo_tempmgr (const char *, const char *);
bool boo_swap_arm (char **);
void boo_up (char *);
void boo_log (void);
void boo_install (void);
void boo_remove (int, const char *);
void boo_del (void);

void boo_init (void);
void boo_close (void);

void command_line (int, char **);

char *cat (const char *fmt, ...) {

	static char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	return buf;
}

/* remove todo caracter especificado */
char *strrmc (char *s, int c) {

	char *b = s, *r = s;
	do {
		if (*b != c)
			*r++ = *b;
	} while (*b++);
	return s;
}

/* apontar para o seu último elemento depois de um caracter */
char *strend (const char *s, int c) {

	size_t i;
	static char b[1024] = {0};
	if (strchr(s,c) && c) {
		for (i = strlen(s); (s[i] != c) && (i > 0); i--);
		for (int j = 0; s[i]; j++, i++)
			b[j] = s[i+1];
	} else
		return strcpy(b,s);
	return b;
}

/* substituir caracteres de uma string */
char *subchr (char *s, int a, int b) {

	char *c = s;
	while (*c)
		if (*c++ == a)
			*--c = b;
	*c = '\0';
	return s;
}

char *strtoupper (const char *s) {

	static char b[BOOMAX];
	for (int i = 0; s[i]; i++)
		b[i] = toupper(s[i]);
	return b;
}

int64_t size_file (FILE *f) {

	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		int64_t pos = ftell(f);
		rewind(f);
		return pos;
	}
	return 0;
}
/* Jenkins's one_at_a_time hash */
char *hash (char *str) {

	uint32_t hash[2] = {0,0};

	for (int i = 0; i < 2; i++) {
		for (size_t j = 0; j != strlen(str); j++) {
			hash[i] += str[j];
			hash[i] += hash[i] << 10;
			hash[i] ^= hash[i] >> 6;
		}
		hash[i] += hash[i] << 3;
		hash[i] ^= hash[i] >> 11;
		hash[i] += hash[i] << 15;
		strrev(str);
	}

	sprintf(str, "%X%X", hash[0], hash[1]);

	return str;
}

char *value_patter (int64_t value) {

	char str[80];
	static char buf[80];
	snprintf(str, sizeof str, "%I64d", value);
	int len = strlen(str + 1);
	for (int i = ((len / 3) + len), j = 0, k = len; i >= 0; i--)
		if ((j < 3) && (k >= 0)) {
			buf[i] = str[k--];
			j++;
		} else {
			buf[i] = '.';
			j = 0;
		}

	return buf;
}

int boo_print (FILE *f, int color, const char *fmt, ...) {

#ifdef WIN_OS
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	if (color != false)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
	va_list ap;
	va_start(ap, fmt);
	int n = vprintf(fmt, ap);
	if (f != NULL)
		vfprintf(f, fmt, ap);
	va_end(ap);
#ifdef WIN_OS
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), info.wAttributes);
#endif
	return n;
}

/* boo_error(white, "\n\t", " ERROR: ", "miss file 404\n"); */
void boo_error (int color, const char *indent, const char *tag, const char *fmt, ...) {

	char *buf = calloc(1024, sizeof(char));
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);

	printf("%s", indent);
	boo_print(logfile, 207, " ! ");
	boo_print(logfile, red, "%s", tag);
	// if (strchr(buf, '\n'))
		// buf[strlen(buf)-1] = 0;
	boo_print(logfile, color, buf);

	va_end(ap);
	free(buf);
}

int boo_transport (char *file, int spin, int echo) {

	FILE *a = NULL;
	int lot = 0;
	char *buf = NULL;
	char *tmp = NULL;
	char *aux = NULL;

	if ((a = fopen(file,"r"))) {

		xmalloc(char, buf, BOOMAX);
		xmalloc(char, tmp, BOOMAX);
		xmalloc(char, aux, BOOMAX);

		while (fgets(buf, BOOMAX, a)) {
			if (!spin)
				sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", tmp, aux);
			else
				sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", aux, tmp);
			/* copy files */
			if (!CopyFile(tmp, aux, 0) && echo)
				boo_error(white, "\n\t", " FAIL ", "%s", tmp);
			lot++;
		}

		xfree(buf);
		xfree(tmp);
		xfree(aux);
		xfclose(a);
	}

	return lot;
}

/* listar arquivos e diretórios e retornar uma lista com seu tamanho */
int boo_tree (const char *fname, char **ls) {

	DIR *dir;
	static int i = 0;
	struct dirent *entry;
	
	char *path = NULL;
	xmalloc(char, path, BOOMAX);

	if ((dir = opendir(fname))) {
		while ((entry = readdir(dir)) != NULL) {
			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				continue;
			sprintf(path, "%s"OS_BAR"%s", fname, entry->d_name);
			strcpy(ls[i++], path);
			boo_tree(path, ls);
		}
		closedir(dir);
	}

	xfree(path);

	return i;
}

/* (Create Read Update Delete) */
char *boo_crud (const char *file, char *out, const char *cmd, const char *tag_line) {
	/*
	 * boo_crud("tag.ini", NULL, "create", "[ARM]master");
	 * boo_crud("tag.ini", buf, "read", "[ARM]");
	 * boo_crud("tag.ini", NULL, "update", "[ARM]master|[ARM]master*");
	 * boo_crud("tag.ini", NULL, "delete", "[USER]joao");
	 */
	FILE *f = NULL;
	int lot = 0, j = 0;
	char **buf = NULL, *tmp = NULL, *p = NULL;
	if (out != NULL)
		*out = 0;

	/* get all lines */
	if ((f = fopen(file, "r"))) {

		xmalloc(char *, buf, BOOMID);
		for (int i = 0; i < BOOMIN; i++)
			xmalloc(char, buf[i], BOOMIN);
		xcalloc(char, tmp, BOOMIN);

		/* list file */
		while (fgets(tmp, BOOMIN, f))
			strcpy(buf[lot++], tmp);

		/* set return */
		if (*cmd != 'u')
			while (!(p = (char *) strstr(buf[j++], tag_line)) && j < lot); /* verify existent tag */

		switch (*cmd) {
			case 'c': /* create */
				if ((f = fopen(file, "a"))) {
					if (p == NULL)
						fprintf(f, "%s\n", tag_line);
					else if (out != NULL)
						strcpy(out, (out != NULL ? buf[j-1] : "\0"));
				} else
					printf("\n [!] error_crud:%d: file \"%s\"\n", __LINE__, file);
				break;
			case 'r': /* read */
				if (out != NULL) {
					for (int i = 0; i < lot; i++) {
						if (strstr(buf[i], tag_line)) {
							strcpy(tmp, strend(buf[i], ']'));
							strcat(out, tmp);
						}
					}
				} else
					printf("\n [!] error_crud:%d: out == NULL\n", __LINE__);
				break;
			case 'u': {/* update */
				char *aux = strtok(strcpy(tmp, tag_line), "|");
				while (!(p = (char *) strstr(buf[j++], aux)) && j < lot);
				if (p != NULL) {
					if ((f = fopen(file, "w"))) {
						aux = strtok(NULL, "|");
						for (int i = 0; i < lot; i++) { /* rewrite */
							if (i != (j-1))
								fprintf(f, "%s", buf[i]);
							else
								fprintf(f, "%s\n", aux);
						}
					} else
						printf("\n [!] error_crud:%d: file \"%s\"\n", __LINE__, file);
					if (out != NULL)
						strcpy(out, (out != NULL ? buf[j-1] : "\0"));
				}
				break;
			}
			case 'd': /* delete */
				if (tag_line && (f = fopen(file, "w"))) {
					for (int i = 0; i < lot; i++)
						if (!strstr(buf[i], tag_line))
							fprintf(f, "%s", buf[i]);
				} else
					printf("\n [!] error_crud:%d: file \"%s\"\n", __LINE__, file);
				if (out != NULL)
					strcpy(out, (p != NULL ? buf[j-1] : "\0"));
				break;
			default: printf("\n [!] error_crud:%d: no tag correspondent\n", __LINE__); break;
		}

		for (int i = 0; i < BOOMIN; i++)
			xfree(buf[i]);
		xfree(buf);
		xfree(tmp);
		xfclose(f);
	} // else
		// printf("\n [!] error_crud:%d: file \"%s\"\n", __LINE__, file);

	/* 
		OUT retorna tudo que já existe
		no arquivo de tags, próprio para
		leitura, e evitando duplicatas,
		pois as chaves podem ser iguai,
		porem o registro da chave deve
		ser diferente.
	*/
	return out;
}

/* gerar log */
void boo_typewriter (int argc, char **argv) {

	char *path = NULL;
	char *timer = NULL;

	xcalloc(char, path, BOOMAX);
	xcalloc(char, timer, BOOMAX);

	struct tm *t;
	time_t seconds;
	time(&seconds);
	
	if ((logfile = fopen(boo[5],"a"))) {

		t = localtime(&seconds);
		getcwd(path, MAX_PATH);
		snprintf(timer, BOOMAX, "%02d:%02d:%02d %02d/%02d/%04d \"%s\"    boo ", t->tm_hour, t->tm_min, t->tm_sec, t->tm_mday, t->tm_mon+1, t->tm_year + 1900, path);

		for (int i = 1; i < argc; i++)
			strcat(timer, cat("%s ", argv[i]));
		
		fprintf(logfile, "%s\n", timer);
		xfclose(logfile);
	}

	xfree(path);
	xfree(timer);
}
/* ignore files by .booignore.ini */
int boo_ignore (const char *fname) {

	int n = 0;
	FILE *a = NULL;
	char *buf = NULL, *tmp = NULL;

	xcalloc(char, buf, BOOMAX);
	xcalloc(char, tmp, BOOMAX);

	if ((a = fopen(boo[3], "r"))) /* boo[3] .booignore.ini */
		while (fgets(buf, BOOMAX, a)) {

			/* remove '\n' */
			if (strchr(buf, '\n'))
				buf[strlen(buf)-1] = 0;

			/* ignore type file */
			if (strchr(buf, '*')) {
				strcpy(buf, strrmc(buf, '*'));
				if (strstr(fname, buf)) {
					n++;
					break;
				}
			}

			/* ignore file */
			sprintf(tmp, "."OS_BAR"%s", buf);
			if (!strcmp(tmp, fname)) {
				n++;
				break;
			}
		}

	xfree(buf);
	xfree(tmp);
	xfclose(a);

	return n;
}

void boo_listdir (const char *name, const char *destiny, FILE *file) {

	DIR *dir = NULL;
	struct dirent *entry = NULL;
	char *buf = NULL;
	char *path = NULL;

	xcalloc(char, buf, BOOMAX);
	xmalloc(char, path, BOOMAX);

	if ((dir = opendir(name))) {
		while ((entry = readdir(dir)) != NULL) {

			FILE *a = NULL;
			*buf = 0;
			sprintf(buf, "%s"OS_BAR"%s", name, entry->d_name);

			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || boo_ignore(buf))
				continue;

			snprintf(path, BOOMID, "%s"OS_BAR"%s", name, entry->d_name);
			/* diretório */
			// if (!(a = fopen(path,"r")))
				// fprintf(file, "\"%s\",\"%s\"\n", path, entry->d_name);
			// else
			/* arquivo */
			if ((a = fopen(path,"r"))) {
				/* hash_table */
				fprintf(file, "\"%s\",\".boo"OS_BAR"%s"OS_BAR"%s\"\n", path, destiny, hash(strcpy(buf, path)));
				xfclose(a);
			}
			boo_listdir(path, destiny, file);
		}
		closedir(dir);
	}
	xfree(buf);
	xfree(path);
}

void boo_status (int argc) {

	DIR *dir = NULL;
	char *buf = NULL;
	xmalloc(char, buf, BOOMAX);
	/* create log file */
	logfile = fopen(boo[5], "a");

	/* open main directory (.boo) */
	if ((dir = opendir(boo[1]))) {

		/* load number files */
		sscanf(boo_crud(boo[17], buf, "read", "[NUM_FILE]"), "%d", &file_number);
		/* carregar braço de trabalho do usuário */
		sscanf(boo_crud(boo[17], buf, "read", "*"), "%s", arm_user);
		arm_user[strlen(arm_user)-1] = 0; /* (remove *) */
		sprintf(path_tree, "%s"OS_BAR".%s", boo[7], arm_user);
		/* pegar data de movimentação */
		sscanf(boo_crud(boo[17], buf, "read", cat("[MOVE_DATA]\"%s\"", arm_user)), "\"%[^\"]\",\"%[^\"]\"", null, data_move);
		/* load user name */
		sscanf(boo_crud(boo[17], buf, "read", "[USER]"), "\"%[^\"]\"\n", log_user);

		/* show main status */
		if (argc == 1)
			boo_show(false, NULL);

		closedir(dir);

	} else if (argc == 1)
		boo_print(logfile, purple, "%s", boologo);

	xfree(buf);
	xfclose(logfile);
}

void boo_show (bool key, const char *fname) {

	DIR *dir;
	FILE *D, *S, *re_tree, *in_tree, *new_file;

	int n = 0, j = 0, iter = 0;
	char **del_files = NULL;
	char *buf = NULL, *tmp1 = NULL, *tmp2 = NULL;

	xmalloc(char, buf, BOOMAX);
	xmalloc(char, tmp1, BOOMAX);
	xmalloc(char, tmp2, BOOMAX);

	xmalloc(char*, del_files, BOOMAX);
	for (int i = 0; i < BOOMAX; i++)
		xcalloc(char, del_files[i], BOOMAX);

	/* main status */
	boo_print(logfile, false, "\n\t(");
	boo_print(logfile, 96, "%s", arm_user);
	boo_print(logfile, false, "|");
	boo_print(logfile, 160, "%s", log_user);
	boo_print(logfile, false, "|");
	boo_print(logfile, 176, "%d", file_number);
	boo_print(logfile, false, ") Status / Last move (%s)\n\n", (*data_move ? data_move : "--:--"));

	if ((ftree = fopen(path_tree,"r"))) {
		/* seek files: "DEL", "OLD" and "MOD" */
		while (fgets(buf, BOOMAX, ftree)) {

			sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);
			
			/* set deleted files */
			if (!(D = fopen(tmp1, "r")) && (D = fopen(tmp2, "r"))) {
				strcpy(del_files[j++], buf);
				n = boo_print(logfile, red, "\t DEL %s\n", tmp1);
			}

			if (fname == NULL || strstr(tmp1, fname))
				if ((S = fopen(tmp1, "r")) && (S = fopen(tmp2, "r")))
					if (boo_diff(tmp1, tmp2, key))
						n++;

			xfclose(D);
			xfclose(S);
		}
		rewind(ftree);

		/* re-list files to the tree list */
		if ((re_tree = fopen(path_tree,"w"))) {
			sprintf(buf, "monitor"OS_BAR"%s", arm_user);
			boo_listdir(".", buf, re_tree);
			xfclose(re_tree);
		}

		/* seek new files */
		while (fgets(buf, BOOMAX, ftree)) {

			sscanf(buf, "\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);

			/* open file and directory */
			int i_ferr = !!(new_file = fopen(tmp2,"r"));
			int i_derr = !!(dir = opendir(tmp1));

			if (!i_derr)
				iter++;

			/* set new files */
			if (!i_ferr && !i_derr) {

				boo_print(logfile, 0, "\t");
				boo_print(logfile, 159, " NEW ");
				n = boo_print(logfile, blue, "%s\n", tmp1);
			}

			if (i_derr)
				closedir(dir);
			xfclose(new_file);
		}

		/* update the number of files */
		boo_crud(boo[17], NULL, "update", cat("[NUM_FILE]%d|[NUM_FILE]%d", file_number, iter));

		/* add deleted files to tree */
		if ((in_tree = fopen(path_tree,"a")))
			for (int i = 0; i < j; i++)
				fprintf(in_tree, "%s", del_files[i]);
		xfclose(in_tree);

		if (!n)
			boo_print(logfile, false, "\t[!] No modifications\n");

		xfclose(ftree);
	} else
		boo_print(logfile, yellow, "\n [?] Where are %s\n\n", path_tree);

	for (int i = 0; i < BOOMAX; i++)
		xfree(del_files[i]);
	xfree(del_files);

	xfree(buf);
	xfree(tmp1);
	xfree(tmp2);
}
/***************************************************************************
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
***************************************************************************/
DIFF *generate_partial_diff(int **table, int a, int b, char **sequence_1, char **sequence_2, int start) {

	/* initialise the indices */
	int i = (a - 1);
	int j = (b - 1);

	DIFF *vdiff = calloc((j + i), sizeof(DIFF));
	vdiff->lot = 0;

	/* loop until there are no items remaining in either sequence */
	while (i > 0 || j > 0) {
		/* check what has happened to the items at these indices */
		if (i > 0 && j > 0 && !strcmp(sequence_1[i + start - 1], sequence_2[j + start - 1])) {
			/* update the diff and the indices */
			vdiff[vdiff->lot].string = strdup(sequence_1[i + start - 1]);
			vdiff[vdiff->lot].edit = UNMODIFIED;
			vdiff->lot++;
			i--;
			j--;
		} else if (j > 0 && table[i][j] == table[i][j - 1]) {
			/* update the diff and the indices */
			vdiff[vdiff->lot].edit = INSERTED;
			vdiff[vdiff->lot].string = strdup(sequence_2[j + start - 1]);
			vdiff->lot++;
			j--;
		} else if (i > 0) {
			/* update the diff and the indices */
			vdiff[vdiff->lot].edit = DELETED;
			vdiff[vdiff->lot].string = strdup(sequence_1[i + start - 1]);
			vdiff->lot++;
			i--;
		}
	}

	return vdiff;
}

int **compute_table(int *a, int *b, char **sequence_1, char **sequence_2, int start, int end1, int end2) {

	/* determine the lengths to be compared */
	int len_1 = (end1 - start + 1);
	int len_2 = (end2 - start + 1);
	*b = (len_2 + 1);
	*a = 1;

	/* initialise the table */
	int **table = malloc(sizeof(int *) * (len_1 + 1));
	for (unsigned int i = 0; i < (unsigned) (len_1 + 1); i++)
		table[i] = calloc((len_2 + 1), sizeof(int));

	/* loop over the rows */
	for (int i = 1; i <= len_1; i++) {
		/* create the new row */
		table[i][0] = 0;
		*a = i + 1;
		/* loop over the columns */
		for (int j = 1; j <= len_2; j++) {
			/* store the longest common subsequence length */
			if (!strcmp(sequence_1[(i + start - 1)], sequence_2[(j + start - 1)]))
				table[i][j] = table[i - 1][j - 1] + 1;
			else
				table[i][j] = max(table[i - 1][j], table[i][j - 1]);
		}
	}

	/* return the table */
	return table;
}

void compare (DIFF *diff, char **sequence_1, int end1, char **sequence_2, int end2) {

	int start = 0;
	int sequence_lot = end1;
	end1--;
	end2--;

	/* skip any common prefix */
	while (start <= end1 && start <= end2 && !strcmp(sequence_1[start], sequence_2[start]))
		start++;
	/* skip any common suffix */
	while (end1 >= start && end2 >= start && !strcmp(sequence_1[end1], sequence_2[end2])) {
		end1--;
		end2--;
	}

	/* compute the table of longest common subsequence lengths */
	int l1 = 0, l2 = 0;
	int **table = compute_table(&l1, &l2, sequence_1, sequence_2, start, end1, end2);

	/* generate the partial diff */
	DIFF *vdiff = generate_partial_diff(table, l1, l2, sequence_1, sequence_2, start);
	int vlot = vdiff->lot;

	for (int i = 0; i < l1; i++)
		free(table[i]);
	free(table);

	/* generate the full diff */
	for (int i = 0; i < start; i++, diff->lot++) {
		diff[i].edit = UNMODIFIED;
		diff[i].string = strdup(sequence_1[i]);
	}

	while (vdiff->lot > 0) {
		diff[diff->lot].edit = vdiff[--vdiff->lot].edit;
		diff[diff->lot++].string = strdup(vdiff[vdiff->lot].string);
	}

	for (int i = 0; i < vlot; i++)
		free(vdiff[i].string);
	free(vdiff);

	for (int i = end1 + 1; i < sequence_lot; i++, diff->lot++) {
		diff[diff->lot].edit = UNMODIFIED;
		diff[diff->lot].string = strdup(sequence_1[i]);
	}
}

char **file_get_contents (const char *fname, int *len) {

	FILE *f = NULL;
	int i = 0;
	char *buf = malloc(sizeof(char) * BOOMAX);
	char **array = malloc(sizeof(char *));

	if ((f = fopen(fname, "r")))
		while (fgets(buf, BOOMAX, f)) {
			array[i++] = strdup(buf);
			array = realloc(array, sizeof(char *) * (i + 1));
		}
	*len = i;

	free(buf);
	fclose(f);
	return array;
}
/***************************************************************************
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
*                                                                          *
***************************************************************************/
bool boo_diff (const char *file1, const char *file2, bool key) {

	FILE *a = NULL;
	FILE *b = NULL;
	bool rtn = false; /* untouch file */

	/* new file */
	if (!(a = fopen(file1, "r")) || !(b = fopen(file2, "r"))) {
		return true;
		boo_error(white, "\n", " ERROR: ", "cannot open file \"%s\"\n", (a==NULL?file1:(b==NULL?file2:file1)));
	}

	/* mod file */
	if (size_file(a) != size_file(b))
		rtn = true;

	/* first verification */
	if (!rtn) {
		char *buf1 = NULL, *buf2 = NULL;
		xcalloc(char, buf1, BOOMAX);
		xcalloc(char, buf2, BOOMAX);
		while (fgets(buf1, BOOMAX, a) && fgets(buf2, BOOMAX, b))
			if (strcmp(buf1, buf2)) {
				rtn = true;
				break;
			}
		xfree(buf1);
		xfree(buf2);
		rewind(a);
		rewind(b);
	}

	/* show change line */
	if (rtn && a != NULL && b != NULL) {

		int lot1, lot2;
		char **f1 = file_get_contents(file2, &lot1);
		char **f2 = file_get_contents(file1, &lot2);

		int diff_len = (lot1 + lot2);
		DIFF *diff = malloc(sizeof(DIFF) * (diff_len + 1));
		diff->lot = 0;

		compare(diff, f1, lot1, f2, lot2);

		boo_print(logfile, 0, "\t");
		boo_print(logfile, 207, " MOD ");
		boo_print(logfile, 95, "%s\n", file1);

		for (int i = 0, gap = 0, index = 0; i < diff->lot; i++) {

			if (diff[i].edit == UNMODIFIED) {
				index -= gap;
				gap = 0;
			}

			switch (diff[i].edit) {
				case UNMODIFIED:
					if (key)
						boo_print(logfile, yellow, "\t%.3d   %s", (index+1), diff[i].string);
					break;
				case DELETED:
					boo_print(logfile, red, "\t%.3d - %s", (index+1), diff[i].string);
					gap++;
					break;
				case INSERTED:
					boo_print(logfile, green, "\t%.3d + %s", (index+1-gap), diff[i].string);
					break;
				default: break;
			}

			index++;

			if (!strchr(diff[i].string, '\n'))
				printf("\n");

			if (diff[i].edit)
				rtn = true;
		}

		for (int i = 0; i < diff->lot; i++)
			free(diff[i].string);
		free(diff);

		for (int i = 0; i < lot1; i++)
			free(f1[i]);
		free(f1);

		for (int i = 0; i < lot2; i++)
			free(f2[i]);
		free(f2);
	}

	return rtn;
}

/* show and create repository or user name */
void boo_name (char *name) {

	FILE *a = NULL;
	const char *path = boo[17];
	char *buf = NULL;
	xmalloc(char, buf, BOOMAX);

	if ((a = fopen(path, "r"))) {
		/* create name */
		if (name) {
			boo_crud(path, NULL, "delete", "[USER]");
			boo_crud(path, NULL, "create", cat("[USER]\"%s\"", name));
			boo_print(logfile, green, "\n [!] Repo name save\n");
		/* show current name */
		} else {
			boo_print(logfile, green, "\n Repo enable: ");
			sscanf(boo_crud(path, buf, "read", "[USER]"), "[USER] \"%[^\"]\"", buf);
			boo_print(logfile, yellow, "%s\n", buf);
		}
	} else
		boo_error(white, "\n", " ERROR: ", "Where are the file \"%s\"\n", path);

	xfree(buf);
	xfclose(a);
}

void boo_move (const char *name) {

	char *data = NULL;

	xmalloc(char, data, BOOMAX);

	sprintf(data, "%02d:%02d:%02d - %02d/%02d/%04d", TM->tm_hour, TM->tm_min, TM->tm_sec, TM->tm_mday, TM->tm_mon+1, TM->tm_year + 1900);

	/* mover apenas um arquivo */
	if (name) {
		boo_print(logfile, false, "\n\t(");
		boo_print(logfile, 96, "%s", strtoupper(arm_user));
		boo_print(logfile, false, ") One file move to monitoring. (%s)\n", data);
		boo_print(logfile, false, "\n\tMOVE: boo move <file> \"move just one file\"\n");

		if (!CopyFile(name, cat("%s"OS_BAR"%s"OS_BAR"%s", boo[7], arm_user, strend(name, OSxBAR)), 0)) {
			boo_error(white, "\n\t", " FAIL: ", "%s\n", name);
			return;
		} else
			boo_print(logfile, green,"\n\tSUCCESS %s\n", name);
	} else {/* mover todos os arquivos */
		/* relistar arquivos */
		if ((ftree = fopen(path_tree,"w"))) {
			char buf[BOOMID];
			sprintf(buf, "monitor"OS_BAR"%s", arm_user);
			boo_listdir(".", buf, ftree);
			xfclose(ftree);
		}
		/* mover arquivos para "./.boo/monitor/.%s" */
		if ((ftree = fopen(path_tree,"r"))) {
			boo_print(logfile, false, "\n\t(");
			boo_print(logfile, 96, "%s", strtoupper(arm_user));
			boo_print(logfile, false, ") Files move to monitoring (%s)\n", data);
			boo_print(logfile, false, "\n\tMOVE: boo move <file> \"move just one file\"\n");
			boo_transport(path_tree, false, false);
		} else
			boo_error(white, "\n", " ERROR: ", "Where are the file \"%s\"", path_tree);
		xfclose(ftree);
	}

	/* atualizar tag com a data atual */
	boo_crud(boo[17], NULL, "delete", cat("[MOVE_DATA]\"%s\"", arm_user));
	boo_crud(boo[17], NULL, "create", cat("[MOVE_DATA]\"%s\",\"%s\"", arm_user, data));

	xfree(data);
}

void boo_undo (void) {

	char *buf = NULL;
	char *tmp = NULL;
	char *aux = NULL;

	xcalloc(char, buf, BOOMAX);
	xcalloc(char, tmp, BOOMAX);
	xcalloc(char, aux, BOOMAX);

	sscanf(boo_crud(boo[17], tmp, "read", cat("[MOVE_DATA]\"%s\"", arm_user)), "[MOVE_DATA] \"%[^\"]\",\"%[^\"]\"", aux, buf);
	sprintf(aux, "%02d:%02d:%02d - %02d/%02d/%04d", TM->tm_hour, TM->tm_min, TM->tm_sec, TM->tm_mday, TM->tm_mon+1, TM->tm_year + 1900);
	printf("\n\t%*s", (22 + (int) strlen(arm_user)), "");
	boo_print(logfile, 72, "%s", aux);
	boo_print(logfile, false, "\n\treturn to last move ");
	boo_print(logfile, green, "(");
	boo_print(logfile, 32, "%s", arm_user);
	boo_print(logfile, green, " %s)\n", buf);

	boo_print(logfile, false, "\tFrom ");
	boo_print(logfile, 32, "(%s"OS_BAR"%s)", boo[7], arm_user);
	boo_print(logfile, false, " return ");
	boo_print(logfile, 32, "[%d]", file_number);
	boo_print(logfile, false, " files\n");

	if (!boo_transport(path_tree, true, false))
		boo_error(white, "\n", " WARNING: ", "no files to undo action\n");

	xfree(buf);
	xfree(tmp);
	xfree(aux);
}

void boo_monitor (void) {

	int n = 0;
	int64_t lines = 0;
	FILE *r = NULL;
	FILE *a = NULL;
	char *buf = NULL;
	char *tmp1 = NULL;
	char *tmp2 = NULL;

	xmalloc(char, buf, BOOMAX);
	xmalloc(char, tmp1, BOOMAX);
	xmalloc(char, tmp2, BOOMAX);

	sprintf(buf, "./.boo/monitor/.%s", arm_user);

	if ((r = fopen(buf, "r"))) {

		boo_print(logfile, false, "\n\tMonitoring files / Last move (%s)\n", (*data_move ? data_move : "--:--"));
		boo_print(logfile, false, "\tIn the directory: %s"OS_BAR"%s\n\n", boo[7], arm_user);

		while (fgets(buf, BOOMAX, r)) {
			sscanf(buf, "\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);
			if ((a = fopen(tmp2,"r"))) {
				n++;
				boo_print(logfile, green,"\t%s\n", tmp1);
				/* counting lines */
				while (fgets(buf, BOOMAX, a))
					if (strchr(buf, '\n'))
						lines++;
			}
			xfclose(a);
		}
		if (!n)
			boo_error(white, "\t", " WARNING: ", "EMPTY\n");
		else {
			printf("\n\t");
			boo_print(logfile, 95, " FILES ");
			boo_print(logfile, 245, " %d ", n);
			boo_print(logfile, white, " / ");
			boo_print(logfile, 95, " TOTAL LINES ");
			boo_print(logfile, 245, " %s \n", value_patter(lines));
		}
		xfclose(r);
	} else
		boo_error(white, "\n", " WARNING: ", "no monitoring files\n");

	xfree(buf);
	xfree(tmp1);
	xfree(tmp2);
}

void boo_submit (char *arg) {

	int i = 0;
	char tag[100];
	char *buf = NULL, *tmp1 = NULL, *tmp2 = NULL;

	xmalloc(char, buf, BOOMAX);
	xmalloc(char, tmp1, BOOMAX);
	xmalloc(char, tmp2, BOOMAX);

	srand(time(NULL));

	/* generate random hash */
	while (i < 20)
		(tag[i++] = hashtype[(rand() % 20)]);
	tag[i] = 0;

	/* agglutinate record */
	if (arg) {
		boo_print(logfile, yellow, "\n [!] Modifications save\n");
		boo_print(logfile, yellow, "\n      #: %s", arg);
		boo_print(logfile, yellow, "\n    arm: %s", strtoupper(arm_user));
		boo_print(logfile, yellow, "\n author: %s", log_user);
		boo_print(logfile, yellow, "\n     up: %s", tag);
		boo_print(logfile, yellow, "\n   date: %02d:%02d:%02d %02d/%02d/%04d\n", TM->tm_hour, TM->tm_min, TM->tm_sec, TM->tm_mday, TM->tm_mon+1, TM->tm_year + 1900);

		/* create directory */
		if ((reg_file = fopen(".boo/room/register","a"))) {
			fprintf(reg_file, "\n      #: %s", arg);
			fprintf(reg_file, "\n    arm: %s", strtoupper(arm_user));
			fprintf(reg_file, "\n author: %s", log_user);
			fprintf(reg_file, "\n     up: %s", tag);
			fprintf(reg_file, "\n   date: %02d:%02d:%02d %02d/%02d/%04d\n", TM->tm_hour, TM->tm_min, TM->tm_sec, TM->tm_mday, TM->tm_mon+1, TM->tm_year + 1900);
		}
		xfclose(reg_file);

		/* create directory */
		CreateDirectory(cat("%s"OS_BAR"%s", boo[19], tag), NULL);

		/* generate rollback file */
		FILE *w = fopen(cat("%s"OS_BAR"%s"OS_BAR"BOOWAKEUP", boo[19], tag),"w");

		/* mover arquivos */
		if ((ftree = fopen(path_tree,"r"))) {
			while (fgets(buf, BOOMAX, ftree)) {
				sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);
				/* BOOWAKEUP */
				fprintf(w, "\"%s"OS_BAR"%s"OS_BAR"%s\",\"%s\"\n", boo[19], tag, hash(strcpy(buf, tmp1)), tmp1);
				/* move files to the version directory */
				sprintf(tmp2, "%s"OS_BAR"%s"OS_BAR"%s", boo[19], tag, hash(strcpy(buf, tmp1)));
				if (!CopyFile(tmp1, tmp2, 0))
					boo_error(white, "\t", " ERROR: ", "FAIL %s\n", tmp2);
			}
		} else
			boo_print(logfile, false, "\n\t[!] No files to save\n");
		xfclose(ftree);
		xfclose(w);
	} else
		boo_print(logfile, green, "\n [!] Your save need a name\n");

	xfree(buf);
	xfree(tmp1);
	xfree(tmp2);
}

void boo_arm (char **argv) {

	char *buf = NULL;
	char **tree = NULL;

	xcalloc(char, buf, BOOMAX);

	xmalloc(char *, tree, BOOMID);
	for (int i = 0; i < BOOMID; i++)
		xcalloc(char, tree[i], BOOMAX);

	/* deletar braço de trabalho */
	if (argv[2] && !strcmp(argv[2], "-d") && argv[3]) {

		if (strcmp(argv[3], arm_user)) {

			/* remove tag */
			if (strstr(boo_crud(boo[17], buf, "read", cat("[ARM]%s", argv[3])), argv[3])) {

				boo_crud(boo[17], NULL, "delete", cat("[ARM]%s", argv[3]));
				boo_crud(boo[17], NULL, "delete", cat("[MOVE_DATA]\"%s\"", argv[3]));

				/* removar diretório */
				int c = boo_tree(boo[7], tree); 

				/* listar arquivos monitorados */
				for (int i = c; i > 0; i--) {
					sprintf(buf, "%s"OS_BAR"%s", boo[7], argv[3]);
					/* remover arquivos */
					if (strstr(tree[i], buf)) {
						remove(tree[i]);
						RemoveDirectory(tree[i]);
					}
				}
				/* remover diretório */
				sprintf(buf, "%s"OS_BAR".%s", boo[7], argv[3]);
				remove(buf);
				boo_print(logfile, red, "\n [!] Arm {%s} deleted\n\n%s\n", strtoupper(argv[3]), delpath);
			} else
				boo_error(white, "\n ", " WARNING: ", "Unexistent arm\n");
		} else
			boo_error(white, "\n ", " ERROR: ", "You cannot delete the current arm\n");
	/* adicionar braço de trabalho */
	} else if (argv[2] && strcmp(argv[2], "-d")) {
		if (!strchr(argv[2], '*')) {
			if (!strstr(boo_crud(boo[17], buf, "create", cat("[ARM]%s", argv[2])), argv[2])) {
				boo_print(logfile, green, "\n [!] New working arm {%s}\n\n%s\n", strtoupper(argv[2]), newpath);
				sprintf(buf, "%s"OS_BAR"%s", boo[7], argv[2]);
				CreateDirectory(buf, NULL);
				sprintf(buf, "%s"OS_BAR".%s", boo[7], argv[2]);
				fopen(buf,"w");
			} else
				boo_error(white, "\n ", " WARNING: ", "This arm already exists\n");
		} else
			boo_error(white, "\n ", " ERROR: ", "Invalid arm\n");
	/* listar braço de trabalho */
	} else {

		printf("\n");
		boo_crud(boo[17], buf, "read", "[ARM]");

		char *aux = strtok(buf, "\n ");
		do {
			if (strstr(aux, "*")) {
				boo_print(logfile, yellow," (");
				boo_print(logfile, 224, "%s", strtoupper(aux));
				boo_print(logfile, yellow, ") ");
			} else
				boo_print(logfile, green, " %s ", aux);
		} while ((aux = strtok(NULL, "\n ")) != NULL);

		boo_print(logfile, false, "\n\n CREATE: boo arm <name>   DELETE: boo arm -d <name>   CHANGE: boo *<name>");
	}

	for (int i = 0; i < BOOMID; i++)
		xfree(tree[i]);

	xfree(tree);
	xfree(buf);
}

/* gestor de arquivos temporareos */
void boo_tempmgr (const char *option, const char *branch) {

	DIR *dir = NULL;
	FILE *new_file = NULL;
	char *buf = NULL;
	char *tmp1 = NULL;
	char *tmp2 = NULL;
	char **tree = NULL;
	const char *c = option;

	xmalloc(char, buf, BOOMAX);
	xmalloc(char, tmp1, BOOMAX);
	xmalloc(char, tmp2, BOOMAX);

	xmalloc(char *, tree, BOOMAX);
	for (int i = 0; i < BOOMID; i++)
		xmalloc(char, tree[i], BOOMAX);

	/* save .temp files */
	if (*c == 's') {

		/* caso o diretório .temp não exista, cria-lo! */
		sprintf(buf, "%s"OS_BAR"%s"OS_BAR".temp", boo[7], branch);
		CreateDirectory(buf, NULL);

		/* limpar .temp antes de mover novos arquivos */
		int n = boo_tree(buf, tree);
		for (int i = n; i >= 0; i--)
			remove(tree[i]);

		/* listar */
		sprintf(path_tree, "%s"OS_BAR"%s"OS_BAR".temp"OS_BAR"BOOTEMPWAKEUP", boo[7], branch);
		ftree = fopen(path_tree, "w");

		sprintf(buf, "monitor"OS_BAR"%s"OS_BAR".temp", branch);
		boo_listdir(".", buf, ftree);

		xfclose(ftree);

		/* mover arquivos de SRC para .temp, antes de ir para o novo ARM */
		boo_transport(path_tree, false, false);

	/* load .temp files */
	} else if (*c == 'l') {

		/* reverter arquivos de .temp para o braço atual */
		sprintf(path_tree, "%s"OS_BAR"%s"OS_BAR".temp"OS_BAR"BOOTEMPWAKEUP", boo[7], branch);
		boo_transport(path_tree, true, false);

		/* listar novos arquivos */
		sprintf(path_tree, "%s"OS_BAR"%s"OS_BAR".temp"OS_BAR"BOOTEMPLIST", boo[7], branch);
		ftree = fopen(path_tree,"w");

		sprintf(buf, "monitor"OS_BAR"%s"OS_BAR".temp", branch);
		boo_listdir(".", buf, ftree);

		xfclose(ftree);

		/* deletar arquivos que vieram do ARM anterior */
		if ((ftree = fopen(path_tree,"r"))) {
			while (fgets(buf, BOOMAX, ftree)) {
				sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);
				if (!(new_file = fopen(tmp2,"r")) && !(dir = opendir(tmp1))) {
					remove(tmp1);
					remove(tmp2);
					if (!!dir)
						closedir(dir);
					xfclose(new_file);
				}
			}
			xfclose(ftree);
		}
	} else
		boo_print(logfile, red, "\n\tunknow option '%s'\n", option);

	xfree(buf);
	xfree(tmp1);
	xfree(tmp2);

	for (int i = 0; i < BOOMID; i++)
		xfree(tree[i]);
	xfree(tree);
}

bool boo_swap_arm (char **argv) {

	char *buf = NULL;

	/* os arquivos do diretório fonte serão modificados para o braço atual */
	if (argv[1] && argv[1][0] == '*' && argv[1][1] > 32) {

		/* verify new arm */
		char *s = argv[1];
		s++;
		xmalloc(char, buf, BOOMIN);
		boo_crud(boo[17], buf, "read", cat("[ARM]%s", s));

		if (*buf) {

			/* update new arm */
			boo_crud(boo[17], NULL, "update", cat("[ARM]%s*|[ARM]%s", arm_user, arm_user));
			boo_crud(boo[17], NULL, "update", cat("[ARM]%s|[ARM]%s*", buf, strrmc(buf, '\n')));

			/* salvar arquivos do ARM atual em .temp */
			boo_tempmgr("save", arm_user);

			/* show status */
			buf = strrmc(buf, 'n');
			boo_print(logfile, false, "\n enable arm (");
			boo_print(logfile, 96, "%s", strtoupper(buf));
			boo_print(logfile, false, ")\n");

			/* recarregar arquivos do novo ARM */
			boo_tempmgr("load", buf);

		} else
			boo_error(white, "\n ", " ERROR: ", "unknown arm\n");

		xfree(buf);
		return true;
	}
	return false;
}

void boo_up (char *code) {

	if (code) {
		int lot = 0;
		char *buf = NULL;
		xmalloc(char, buf, BOOMAX);
		sprintf(buf, "%s"OS_BAR"%s"OS_BAR"BOOWAKEUP", boo[19], code);
		if ((lot = boo_transport(buf, false, false))) {
			boo_print(logfile, green, "\n [!] ");
			boo_print(logfile, 32, " %d ", lot);
			boo_print(logfile, green, " File%c reborn\n", (lot>1?'s':0));
		} else
			boo_error(white, "\n ", " ERROR: ", "Wrong (UP) code\n");
		xfree(buf);
	} else
		boo_error(white, "\n ", " WARNING: ", "Insert the (UP) code of the register version\n");
}

void boo_log (void) {

	char *buf = NULL;
	xmalloc(char, buf, BOOMAX);
	if ((reg_file = fopen(boo[15],"r"))) {
		if (!size_file(reg_file))
			boo_error(white, "\n ", " WARNING: ", "no register\n");
		while (fgets(buf, BOOMAX, reg_file))
			boo_print(logfile, yellow, "%s", buf);
		xfclose(reg_file);
	} else
		boo_error(white, "\n ", " ERROR: ", "where are the file \"%s\"\n", boo[15]);

	xfree(buf);
}

void boo_install (void) {

	FILE *f = NULL;
	DIR *dir = NULL;

	if (!(dir = opendir(boo[1]))) {
		/* mkdir */
		for (int i = 0; boo[i]; i++) {
			if (*boo[i] == 'D') { /* create directory */
			#ifdef WIN_OS
				CreateDirectory(boo[++i], NULL);
			#else
				if (mkdir(boo[++i], 0777) && errno != EEXIST)
					boo_error(white, "\n ", " ERROR: ", "while trying to create \"%s\"\n", boo[i-1]);
			#endif
			} else
				fopen(boo[++i], "a"); /* create files */
		}

		/* insert tags */
		if ((f = fopen(boo[17],"w")))
			fprintf(f, "[USER] \"unknown\"\n[ARM] master*\n");
		xfclose(f);

		/* create .booignore.ini */
		if (!(f = fopen(boo[3],"r"))) {
			if ((f = fopen(boo[3],"a"))) {
				fprintf(f,".booignore.ini\n.boo\n");
				xfclose(f);
			} else
				boo_error(white, "\n ", " WARNING: ", "create \"%s\"\n", boo[3]);
		} else
			xfclose(f);

		boo_print(logfile, green, "\n [!] Boo repository INSTALLED\n");
	
	#ifdef WIN_OS
		/* hide root directory on windows */
		SetFileAttributes(boo[1], FILE_ATTRIBUTE_HIDDEN);
	#else
		/* linux hide */
	#endif

		closedir(dir);
	} else
		boo_error(white, "\n ", " WARNING: ", "Boo are installed on current directory\n");
}

void boo_remove (int w, const char *file) {

	char *buf = NULL;
	char **tree = NULL;

	xmalloc(char, buf, BOOMAX);

	xmalloc(char *, tree, BOOMAX);
	for (int i = 0; i < BOOMID; i++)
		xcalloc(char, tree[i], BOOMAX);

	int c = boo_tree(file, tree);

	for (int i = c; i >= 0; i--) {

	#ifdef WIN_OS
		remove(tree[i]);
		RemoveDirectory(tree[i]);
	#else
		sprintf(buf, "rm %s; rmdir %s", tree[i], tree[i]);
		system(buf);
	#endif

		if (w)
			boo_print(logfile, green,"\n REMOVE %s", tree[i]);
	}

#ifdef WIN_OS
	RemoveDirectory(file);
#else
	sprintf(buf, "rmdir %s", file);
	system(buf);
#endif

	if (w)
		boo_print(logfile, green, "\n\n [!] Boo repository REMOVED\n");

	for (int i = 0; i < BOOMID; i++)
		xfree(tree[i]);
	xfree(tree);

	xfree(buf);
}

void boo_del (void) {

	FILE *fdel = NULL;
	char *buf = NULL;
	char *tmp1 = NULL;
	char *tmp2 = NULL;
	char *tmp3 = NULL;

	xmalloc(char, buf, BOOMAX);
	xmalloc(char, tmp1, BOOMAX);
	xmalloc(char, tmp2, BOOMAX);
	xmalloc(char, tmp3, BOOMAX);

	if ((ftree = fopen(path_tree, "r"))) {

		int n = 0;

		while (fgets(buf, BOOMAX, ftree)) { /* localizar arquivo "DEL" */

			sscanf(buf,"\"%[^\"]\",\"%[^\"]\"", tmp1, tmp2);

			/* deletar arquivos de .temp */
			sprintf(tmp3, "%s"OS_BAR"%s"OS_BAR".temp"OS_BAR"%s", boo[7], arm_user, strend(tmp2, OSxBAR));

			if (!(fdel = fopen(tmp1, "r")) && (fdel = fopen(tmp2, "r"))) {
				xfclose(fdel);
				remove(tmp3); /* deletar arquivos de .temp */
				if (!remove(tmp2))
					n = boo_print(logfile, red, "\n\tDEL %s", tmp1);
				else
					boo_print(logfile, red, "\n\tFAIL %s", tmp1);
			} else
				xfclose(fdel);
		}

		if (n)
			boo_print(logfile, green, "\n\n\t[!] file drop\n");
		else
			boo_error(white, "\n\t", " WARNING: ", "no file for delete\n");

		xfclose(ftree);
	} else
		boo_error(white, "\n\t", " WARNING: ", "where are the file \"%s\"", path_tree);

	xfree(buf);
	xfree(tmp1);
	xfree(tmp2);
	xfree(tmp3);
}

void boo_init (void) {

	time_t seconds;
	time(&seconds);
	TM = localtime(&seconds);
	srand(time(NULL));

	xcalloc(char, null, BOOMID);
	xcalloc(char, log_user, BOOMID);
	xcalloc(char, arm_user, BOOMID);
	xcalloc(char, path_tree, BOOMAX);
	xcalloc(char, data_move, BOOMID);
	xcalloc(char, global_buf, BOOMID);

#ifdef WIN_OS
	output_state_shell = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
#endif

	char *buf = NULL;
	xmalloc(char, buf, BOOMIN);
	/* verify user file */
	boo_crud(boo[17], buf, "read", "[ARM]");
	boo_crud(boo[17], buf, "read", "[USER]");
	xfree(buf);
}

void boo_close (void) {

	xfree(null);
	xfree(log_user);
	xfree(arm_user);
	xfree(path_tree);
	xfree(data_move);
	xfree(global_buf);

#ifdef WIN_OS
	SetConsoleOutputCP(output_state_shell);
#endif
}

void command_line (int argc, char **argv) {

	DIR *dir = NULL;

	if (argc == 1)
		return;

	if (boo_swap_arm(argv))
		return;

	if (!strcmp(argv[1], "install")) {
		boo_install();
		return;
	}

	if (!strcmp(argv[1], "help")) {
		printf("%s", boohelp);
		return;
	} else if (!strcmp(argv[1], "ver")) {
		boo_print(logfile, purple, "%s", boologo);
		return;
	}

	if ((dir = opendir(".boo"))) {

		if (!strcmp(argv[1], "monitor"))
			boo_monitor();
		else if (!strcmp(argv[1], "remove")) {
			closedir(dir);
			fclose(logfile);
			boo_remove(true, ".boo");
			return;
		} else if (!strcmp(argv[1], "submit"))
			boo_submit(argv[2]);
		// else if (!strcmp(argv[1], "merge"))
		// 	boo_merge(argv[2]);
		else if (!strcmp(argv[1], "name"))
			boo_name(argv[2]);
		else if (!strcmp(argv[1], "move"))
			boo_move(argv[2]);
		else if (!strcmp(argv[1], "undo"))
			boo_undo();
		else if (!strcmp(argv[1], "log"))
			boo_log();
		else if (!strcmp(argv[1], "arm"))
			boo_arm(argv);
		else if (!strcmp(argv[1], "del"))
			boo_del();
		else if (!strcmp(argv[1], "dif"))
			boo_show(true, argv[2]);
		else if (!strcmp(argv[1], "up"))
			boo_up(argv[2]);
		else {
			boo_error(white, "\n ", " WARNING: ", "invalid command: ");
			boo_print(logfile, green, "%s\n", argv[1]);
		}
		closedir(dir);
	} else
		boo_error(white, "\n\t", " WARNING 2319: ", "where is BOO?\n");
}

int main (int argc, char **argv) {

	boo_typewriter(argc, argv);
	boo_init();
	boo_status(argc);
	command_line(argc, argv);
	boo_close();

	return 0;
}