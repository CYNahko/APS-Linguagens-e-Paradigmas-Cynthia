// Compile: gcc -std=c11 -O0 -g vm/cocoonvm.c -o bin/cocoonvm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_LINES 20000
#define LINE_LEN 512
#define MAX_LABELS 2000
#define STACK_SIZE 4096
#define MAX_HISTORY 1024
#define MAX_STR 256

// Registers: R1..R4, IP (not exposed), SP (stack pointer)
int64_t regs[6]; // 0..3 = R1..R4, 4 = IP, 5 = SP
int64_t stack_arr[STACK_SIZE];
int sp_top = STACK_SIZE;
int flag_zero = 0, flag_gt = 0;

typedef struct { const char *name; int64_t val; } sensor_t;
sensor_t sensors[] = {
    {"sensor_energia", 120},
    {"sensor_calor", 25},
    {"sensor_fluxo", 50},
    {"sensor_luz", 10},
    {"sensor_impacto", 0},
    {NULL, 0}
};

char *program[MAX_LINES];
int prog_lines = 0;

typedef struct { char name[MAX_STR]; int index; } label_t;
label_t labels[MAX_LABELS];
int label_count = 0;

char *history[MAX_HISTORY];
int history_count = 0;

static char *trim(char *s) {
    if(!s) return s;
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) *end-- = 0;
    return s;
}

static void add_label(const char *name, int idx) {
    if(label_count >= MAX_LABELS) return;
    strncpy(labels[label_count].name, name, MAX_STR-1);
    labels[label_count].name[MAX_STR-1]=0;
    labels[label_count].index = idx;
    label_count++;
}

static int find_label(const char *name) {
    for(int i=0;i<label_count;i++){
        if(strcmp(labels[i].name, name)==0) return labels[i].index;
    }
    return -1;
}

static int64_t *get_reg_ptr(const char *tok) {
    // accepts R1..R4 or names massa/energia mapping
    if(tolower(tok[0])=='r' && isdigit((unsigned char)tok[1])) {
        int rn = atoi(tok+1);
        if(rn>=1 && rn<=4) return &regs[rn-1];
    }
    // aliases
    if(strcmp(tok,"massa")==0) return &regs[0];
    if(strcmp(tok,"energia")==0) return &regs[1];
    if(strcmp(tok,"mutacao")==0) return &regs[2];
    if(strcmp(tok,"fluxo")==0) return &regs[3];
    return NULL;
}

static int64_t sensor_lookup(const char *name) {
    for(int i=0; sensors[i].name; ++i) {
        if(strcmp(sensors[i].name, name)==0) return sensors[i].val;
    }
    // try strip possible whitespace
    return 0;
}

static void push_stack(int64_t v) {
    if(regs[5] <= 0) { fprintf(stderr,"[VM] stack overflow\n"); exit(1); }
    regs[5]--;
    stack_arr[regs[5]] = v;
}
static int64_t pop_stack() {
    if(regs[5] >= STACK_SIZE) { fprintf(stderr,"[VM] stack underflow\n"); exit(1); }
    int64_t v = stack_arr[regs[5]];
    regs[5]++;
    return v;
}

static char *dupstr(const char *s) {
    if(!s) return NULL;
    char *r = malloc(strlen(s)+1);
    strcpy(r,s);
    return r;
}

void load_prog(const char *fname) {
    FILE *f = fopen(fname,"r");
    if(!f){ perror("open"); exit(1); }
    char buf[LINE_LEN];
    prog_lines = 0;
    while(fgets(buf, LINE_LEN, f) && prog_lines < MAX_LINES) {
        // strip newline
        char *p = buf;
        while(*p && (*p=='\r' || *p=='\n')) *p++ = 0; // shouldn't matter
        // copy trimmed
        char *line = strdup(buf);
        char *t = trim(line);
        if(*t==0 || *t==';') { free(line); continue; } // empty or comment
        char *stored = strdup(t);
        free(line);
        program[prog_lines++] = stored;
    }
    fclose(f);
}

void index_labels() {
    for(int i=0;i<prog_lines;i++){
        char *s = program[i];
        char tmp[LINE_LEN];
        strncpy(tmp, s, LINE_LEN-1); tmp[LINE_LEN-1]=0;
        char *t = trim(tmp);
        int L = strlen(t);
        if(L>1 && t[L-1]==':') {
            char name[MAX_STR];
            strncpy(name, t, L-1);
            name[L-1]=0;
            add_label(name, i+1); // label points to next line
        } else {
            // check "LABEL name"
            if(strncmp(t,"LABEL ",6)==0) {
                char *name = trim(t+6);
                add_label(name, i+1);
            }
        }
    }
}

// parsing helpers
static int tokenize(char *s, char **out, int max) {
    int n = 0;
    char *p = s;
    while(*p && n < max) {
        while(isspace((unsigned char)*p)) p++;
        if(*p==0) break;
        if(*p=='"') {
            // quoted string
            char *q = ++p;
            while(*p && *p!='"') p++;
            int len = p - q;
            out[n] = malloc(len+1);
            memcpy(out[n], q, len);
            out[n][len]=0;
            if(*p=='"') p++;
            n++;
        } else {
            char *q = p;
            while(*p && !isspace((unsigned char)*p)) {
                if(*p==',' || *p==':' ) break;
                p++;
            }
            int len = p - q;
            if(len>0) {
                out[n] = malloc(len+1);
                memcpy(out[n], q, len);
                out[n][len]=0;
                n++;
            } else {
                if(*p==',' || *p==':' ) {
                    out[n] = malloc(2); out[n][0]=*p; out[n][1]=0; n++; p++;
                } else {
                    p++;
                }
            }
        }
    }
    return n;
}

void free_tokens(char **toks, int n) {
    for(int i=0;i<n;i++) free(toks[i]);
}

void exec_line(const char *line) {
    // ignore comments at end
    char tmp[LINE_LEN]; strncpy(tmp, line, LINE_LEN-1); tmp[LINE_LEN-1]=0;
    char *p_comment = strchr(tmp, ';');
    if(p_comment) *p_comment = 0;
    char *tline = trim(tmp);
    if(*tline==0) return;
    
    int L = strlen(tline);
    if(L>1 && tline[L-1]==':') return;
    if(strncmp(tline,"LABEL ",6)==0) return;

    // tokenize into tokens
    char *toks[20];
    for(int i=0;i<20;i++) toks[i]=NULL;
    int tn = tokenize((char*)tline, toks, 20);
    if(tn==0) return;

    // handle opcodes
    if(strcmp(toks[0],"LOAD")==0 && tn>=3) {
        
        char *r = toks[1];
        char *v = toks[2];
        int64_t val = atoll(v);
        int64_t *rp = get_reg_ptr(r);
        if(rp) *rp = val;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"MOV")==0 && tn>=3) {
        int64_t *ra = get_reg_ptr(toks[1]);
        int64_t *rb = get_reg_ptr(toks[2]);
        if(ra && rb) *ra = *rb;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"ADD")==0 && tn>=3) {
        int64_t *r = get_reg_ptr(toks[1]);
        int64_t val = atoll(toks[2]);
        if(r) *r += val;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"SUB")==0 && tn>=3) {
        int64_t *r = get_reg_ptr(toks[1]);
        int64_t val = atoll(toks[2]);
        if(r) *r -= val;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"MUL")==0 && tn>=3) {
        int64_t *r = get_reg_ptr(toks[1]); int64_t val = atoll(toks[2]);
        if(r) *r *= val; free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"DIV")==0 && tn>=3) {
        int64_t *r = get_reg_ptr(toks[1]); int64_t val = atoll(toks[2]);
        if(r && val!=0) *r /= val; free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"CMP")==0 && tn>=3) {
        
        char *left = toks[1];
        char *right = toks[2];
        int64_t rval = atoll(right);
        if(strncmp(left,"sensor_",7)==0) {
            int64_t s = sensor_lookup(left);
            flag_zero = (s == rval);
            flag_gt = (s > rval);
        } else {
            int64_t *rp = get_reg_ptr(left);
            if(rp) {
                flag_zero = (*rp == rval);
                flag_gt = (*rp > rval);
            } else {
                flag_zero = 0; flag_gt = 0;
            }
        }
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"IFZERO")==0 && tn>=2) {
        char *lbl = toks[1];
        if(flag_zero) {
            int pos = find_label(lbl);
            if(pos>=0) regs[4] = pos;
            else { /* missing label */ }
        }
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"IFGT")==0 && tn>=2) {
        char *lbl = toks[1];
        if(flag_gt) {
            int pos = find_label(lbl);
            if(pos>=0) regs[4] = pos;
        }
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"JMP")==0 && tn>=2) {
        char *lbl = toks[1];
        int pos = find_label(lbl);
        if(pos>=0) regs[4] = pos;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"PUSH")==0 && tn>=2) {
        int64_t *rp = get_reg_ptr(toks[1]);
        if(rp) push_stack(*rp);
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"POP")==0 && tn>=2) {
        int64_t *rp = get_reg_ptr(toks[1]);
        if(rp) *rp = pop_stack();
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"ABSORVER")==0) {
        
        regs[1] += 30;
        history[history_count++] = dupstr("ABSORVER");
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"DISSOLVER")==0) {
       
        regs[0] -= 5; regs[1] += 10;
        history[history_count++] = dupstr("DISSOLVER");
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"CONDENSAR")==0) {
        
        regs[2] += 1;
        history[history_count++] = dupstr("CONDENSAR");
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"MUTAR")==0) {
        
        if(tn>=2) {
            char buf[MAX_STR];
            snprintf(buf, MAX_STR, "MUTAR %s", toks[1]);
            history[history_count++] = dupstr(buf);
            regs[2] += 1; 
        }
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"PARTILHAR")==0) {
        
        regs[3] += 5;
        history[history_count++] = dupstr("PARTILHAR");
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"ANULAR")==0) {
        
        regs[1] = regs[1] / 2;
        history[history_count++] = dupstr("ANULAR");
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"PRINT_REG")==0 && tn>=2) {
        int64_t *rp = get_reg_ptr(toks[1]);
        if(rp) printf("[VM] %s = %ld\n", toks[1], *rp);
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"PRINT_STR")==0 && tn>=2) {
        printf("%s\n", toks[1]);
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"DUMP")==0) {
        printf("[VM] DUMP R1=%ld R2=%ld R3=%ld R4=%ld\n",
               regs[0], regs[1], regs[2], regs[3]);
        printf("[VM] history_count=%d\n", history_count);
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"RETURN")==0) {
        
        regs[4] = -1;
        free_tokens(toks, tn); return;
    }
    if(strcmp(toks[0],"HALT")==0) {
        regs[4] = -1; free_tokens(toks, tn); return;
    }
    free_tokens(toks, tn);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr,"Usage: %s program.casm\n", argv[0]);
        return 1;
    }
    // init regs
    regs[0]=0; regs[1]=0; regs[2]=0; regs[3]=0;
    regs[4]=0; regs[5]=STACK_SIZE; // IP and SP
    load_prog(argv[1]);
    index_labels();

    // main loop
    while(regs[4] >= 0 && regs[4] < prog_lines) {
        char *line = program[regs[4]];
        exec_line(line);
        if(regs[4] == -1) break;
        regs[4]++;
    }

    printf("[VM] finished. R1=%ld R2=%ld R3=%ld R4=%ld\n",
           regs[0], regs[1], regs[2], regs[3]);
    if(history_count>0) {
        printf("[VM] history:\n");
        for(int i=0;i<history_count;i++) printf("  %s\n", history[i]);
    }
    return 0;
}
