#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <link.h>
#include <dlfcn.h>
#include <android/log.h>
#include <jni.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#define LOG(...) __android_log_print(ANDROID_LOG_INFO,"TernFontProbe",__VA_ARGS__)
typedef int (*Hook)(void*,void*,void**);
typedef int (*Unhook)(void*);
typedef void (*Callback)(const char*,void*);
typedef struct {uint32_t version; Hook hook; Unhook unhook;} API;
static _Atomic unsigned scans, installed, calls;
extern void probe_entry(void);
extern const unsigned char maple_start[], maple_end[];
extern const unsigned char ui_start[], ui_end[];
static uintptr_t target_source, ui_source;
static _Atomic unsigned ui_replacements;
static _Atomic unsigned replacements;
__attribute__((visibility("hidden"))) void select_source(const uintptr_t *source, void *caller, uintptr_t *saved) {
 uintptr_t pair[2]; memcpy(pair,source,sizeof(pair));
 int replace=(pair[0]==target_source && pair[1]==2288420);
 if(replace) {
  pair[0]=(uintptr_t)maple_start; pair[1]=20563672;
  unsigned n=atomic_fetch_add(&replacements,1);
  if(n<12) LOG("MAPLE_REPLACE[%u] caller=%p len=%zu; Regular only",n,caller,(size_t)pair[1]);
 }
 if(pair[0]==ui_source && pair[1]==169056) {
  pair[0]=(uintptr_t)ui_start; pair[1]=128516; replace=2;
  unsigned n=atomic_fetch_add(&ui_replacements,1);
  if(n<12) LOG("UI_REPLACE[%u] caller=%p len=%zu",n,caller,(size_t)pair[1]);
 }
 saved[0]=pair[0]; saved[1]=pair[1]; saved[8]=pair[0];
 unsigned n=atomic_fetch_add(&calls,1);
 if(n<16) LOG("source[%u] ptr=%p len=%zu replaced=%d",n,(void*)pair[0],(size_t)pair[1],replace);
}
static void install_slot(uintptr_t b) {
 unsigned expected=0;
 if(!atomic_compare_exchange_strong(&installed,&expected,1)) return;
 uintptr_t *slot=(uintptr_t*)(b+0x80d4650);
 long ps=sysconf(_SC_PAGESIZE); if(ps<=0) {LOG("invalid page size; skipped");return;}
 uintptr_t page=(uintptr_t)slot & ~((uintptr_t)ps-1);
 FILE *f=fopen("/proc/self/maps","r");
 if(!f) {LOG("cannot inspect protection; skipped");return;}
 char line[1024],perm[5]; unsigned long lo,hi; int prot=-1;
 while(fgets(line,sizeof(line),f)) {
  if(sscanf(line,"%lx-%lx %4s",&lo,&hi,perm)==3 && page>=lo && page+(uintptr_t)ps<=hi) {
   prot=(perm[0]=='r'?PROT_READ:0)|(perm[1]=='w'?PROT_WRITE:0)|(perm[2]=='x'?PROT_EXEC:0);break;
  }
 }
 fclose(f);
 if(prot!=PROT_READ) {LOG("unexpected slot page protection=%d; skipped",prot);return;}
 if(mprotect((void*)page,(size_t)ps,PROT_READ|PROT_WRITE)) {LOG("mprotect RW failed errno=%d; skipped",errno);return;}
 target_source=b+0xd0535b;
 ui_source=b+0xcdbeee;
 uintptr_t old=b+0x7421b08;
 int changed=__atomic_compare_exchange_n(slot,&old,(uintptr_t)probe_entry,0,__ATOMIC_RELEASE,__ATOMIC_RELAXED);
 int restored=mprotect((void*)page,(size_t)ps,prot);
 LOG("slot replacement installed=%d protection_restored=%d errno=%d; MAPLE_AND_UI_SYMBOLS",changed,restored==0,restored?errno:0);
}
static int covered(const struct dl_phdr_info *i, uintptr_t off, size_t n, unsigned flags) {
 for(size_t k=0;k<i->dlpi_phnum;k++) {
  const ElfW(Phdr)*p=&i->dlpi_phdr[k];
  if(p->p_type==PT_LOAD && (p->p_flags&flags)==flags && off>=p->p_vaddr && n<=p->p_memsz && off-p->p_vaddr<=p->p_memsz-n) return 1;
 } return 0;
}
static int inspect(struct dl_phdr_info *i,size_t size,void *why) {
 (void)size;
 const char *s=strrchr(i->dlpi_name,'/'); s=s?s+1:i->dlpi_name;
 if(strcmp(s,"libtern.so")) return 0;
 unsigned count=atomic_fetch_add(&scans,1); if(count>=8) return 1;
 uintptr_t b=i->dlpi_addr;
 if(atomic_load(&installed)) {LOG("%s: duplicate notification; no second install",(char*)why);return 1;}
 const uint32_t code[3]={0xa9400408,0xaa0803e0,0xd65f03c0};
 if(!covered(i,0x7421b08,12,PF_R|PF_X)||!covered(i,0x80d4638,32,PF_R)) {
  LOG("%s: bounds mismatch; no changes",(char*)why); return 1;
 }
 uintptr_t table[4]; memcpy(table,(void*)(b+0x80d4638),sizeof(table));
 int ok=!memcmp((void*)(b+0x7421b08),code,12) && table[0]==0 && table[1]==16 && table[2]==8 && table[3]==b+0x7421b08;
 LOG("%s: base=%p getter=%p slot=%p value=%p validation=%s; REPLACEMENT_PRECHECK",(char*)why,(void*)b,(void*)(b+0x7421b08),(void*)(b+0x80d4650),(void*)table[3],ok?"PASS":"FAIL");
 if(ok && covered(i,0xd0535b,2288420,PF_R) &&
    covered(i,0xcdbeee,169056,PF_R) && !memcmp((void*)(b+0xcdbeee),"\0\1\0\0",4) &&
    (size_t)(ui_end-ui_start)==128516 && !memcmp(ui_start,"\0\1\0\0",4) &&
    (size_t)(maple_end-maple_start)==20563672 && !memcmp(maple_start,"\0\1\0\0",4) &&
    !memcmp((void*)(b+0xd0535b),"\0\1\0\0",4)) install_slot(b);
 else LOG("replacement validation failed; using original fonts");
 return 1;
}
static void loaded(const char *name,void *handle) {
 (void)handle;
 if(!name) return;
 const char *s=strrchr(name,'/'); s=s?s+1:name;
 if(!strcmp(s,"libtern.so")) {LOG("libtern load callback received"); dl_iterate_phdr(inspect,"load_callback");}
}
__attribute__((visibility("default"),used)) Callback native_init(const API *a) {
 LOG("native_init api=%u; Maple and UI symbols test 0.3.2; inline hook API not used",a?a->version:0);
 dl_iterate_phdr(inspect,"native_init_already_loaded");
 return loaded;
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved) {
 (void)vm;(void)reserved; LOG("JNI_OnLoad UI symbols test 0.3.2");
 dl_iterate_phdr(inspect,"JNI_OnLoad_already_loaded");
 return JNI_VERSION_1_6;
}
