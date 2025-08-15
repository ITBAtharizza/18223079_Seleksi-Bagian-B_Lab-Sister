
#include "./user-shell.h"
#include "./ls.h"
#include "./mkdir.h"
#include "./cd.h"
#include "./rm.h"
#include "./find.h"
#include "cat.h"
#include "cp.h"
#include "mv.h"
#include "util.h"
#include "ps.h"
#include "exec.h"
#include "kill.h"
#include "play.h"
#include "txt.h"
#include "write.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}


struct ShellState state = {
    .current_directory = ROOT_CLUSTER_NUMBER, 
    .current_directory_name = state.curr_dir.table->name,
};
 
void updateDirectoryTable(uint32_t cluster_number) {
    syscall(CHANGE_DIR, (uint32_t)&state.curr_dir, cluster_number, 0x1);
}

void refresh_dir(){
   int8_t ret;
    struct FAT32DriverRequest req={
        .name = "\0\0\0\0\0\0\0\0",
        .buffer_size = 0, 
        .buf = &state.curr_dir,
        .parent_cluster_number = state.current_directory,
    };
    memcpy(req.name, state.current_directory_name, strlen(state.current_directory_name));
    syscall(READ_DIRECTORY, (uint32_t)&req, (uint32_t)&ret, 0); 
}

void run_prompt() {
    char* token = my_strtok(state.prompt_val, ' ');
    if(memcmp(token, "ls", 2) == 0){
        ls();
    }
    else if(memcmp(token, "mkdir", 5) == 0){
        char* arg = my_strtok(NULL, '\0');
        mkdir(arg);
    }
    else if(memcmp(token, "cd", 2) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        cd(arg);
    }
    else if(memcmp(token, "rm", 2) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        remove(arg, state.current_directory);
    } 
    else if(memcmp(token, "cat", 3) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        cat(arg, state.current_directory);
    }
    else if(memcmp(token, "cp", 2) == 0){
        char* arg1 = my_strtok(NULL, ' ');
        char* arg2 = my_strtok(NULL, '\0'); 
        cp(arg1, arg2, state.current_directory);
    }
    else if(memcmp(token, "mv", 2) == 0){
        char* arg1 = my_strtok(NULL, ' ');
        char* arg2 = my_strtok(NULL, '\0'); 
        mv(arg1, arg2, state.current_directory);
    }
    else if(memcmp(token, "find", 4) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        find(arg);
    }
    else if(memcmp(token, "ps", 2) == 0){
        ps();
    }
    else if(memcmp(token, "exec", 4) == 0){
        char* arg = my_strtok(NULL, '\0');
        exec(arg);
    }
    else if(memcmp(token, "kill", 4) == 0){
        char* arg = my_strtok(NULL, '\0');
        kill(arg);
    }
    else if(memcmp(token, "clear", 5) == 0){
        syscall(CLEAR, 0, 0, 0);
    }
    else if (memcmp(token, "play", 4) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        play(arg, state.current_directory);        
    }
    else if(memcmp(token, "txt", 3) == 0){
        char* arg = my_strtok(NULL, '\0'); 
        txt(arg);
    }
    else if(memcmp(token, "write", 5) == 0){
        char* arg1 = my_strtok(NULL, ' '); 
        char* arg2 = my_strtok(NULL, '\0'); 
        writef(arg1, arg2);
    }
    else{
        put_chars("Shell: Perintah tidak ada", BIOS_RED);
        put_char('\n');
        put_chars("List perintah:", BIOS_RED);
        put_char('\n');
        put_chars("cat ", BIOS_RED);
        put_chars("find ", BIOS_RED);
        put_chars("ls ", BIOS_RED);
        put_chars("mv ", BIOS_RED);
        put_chars("cp ", BIOS_RED);
        put_chars("mkdir ", BIOS_RED);
        put_chars("cd ", BIOS_RED);
        put_chars("rm ", BIOS_RED);
        put_chars("ps exec kill clear play", BIOS_RED);
        put_char('\n');
    }
}

void clear_prompt() {
    state.prompt_size = 0;
    memset(state.prompt_val, 0, MAX_PROMPT);
}

//buat nulis di shell
void get_prompt(){
    clear_prompt();
    while(1){
        char c = '\0'; 
        while(c == '\0'){
            syscall(GET_PROMPT, (uint32_t) &c, 0, 0);
        }
        if(c == '\b'){
            if(state.prompt_size> 0){
                state.prompt_val[state.prompt_size--] = c;
                syscall(12, (uint32_t)' ', 0, 0xF);
            }
        }else{
            syscall(PUT_CHAR, (uint32_t)c, BIOS_GREY, 0xF);
            if(c == '\n' || state.prompt_size + 1 >= MAX_PROMPT){
                break;
            }
            state.prompt_val[state.prompt_size++] = c;
        }
    }
    state.prompt_val[state.prompt_size] = '\0';
}

void init(){
    int8_t ret;
    struct FAT32DriverRequest req = {
        .name = "root\0\0\0\0",
        .buffer_size = 0, 
        .buf = &state.curr_dir, 
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    };
    syscall(READ_DIRECTORY, (uint32_t)&req, (uint32_t)&ret, 0);

}

int main(void){
	int8_t ret;
    state.current_directory = ROOT_CLUSTER_NUMBER;
    syscall(ACTIVATE_KEYBOARD, 0, 0, 0);

    char bufer[874] = "[Alstroemeria Records (Bad Apple!!) ft. nomico]\n\n[Verse 1]\nNagareteku toki no naka de demo Kedarusa ga hora guruguru mawatte Watashi kara hanareru kokoro mo Mienaiwa sou shiranai? Jibun kara ugoku koto mo naku Toki no sukima ni nagasare tsuzukete Shiranai wa mawari no koto nado Watashi wa watashi sore dake\n[Refrain]\nYume miteru? Nani mo mitenai? Kataru mo muda na jibun no kotoba Kanashimu nante tsukareru dake yo Nani mo kanjizu sugoseba ii no Tomadou kotoba ataerarete mo Jibun no kokoro tada uwa no sora Moshi watashi kara ugoku no naraba Subete kaeru no nara kuro ni suru\n[Chorus]\nKonna jibun ni mirai wa aru no? Konna sekai ni watashi wa iru no? Ima setsunai no? Ima kanashii no? Jibun no koto mo wakaranai mama Ayumu koto sae tsukareru dake yo Hito no koto nado shiri mo shinaiwa Konna watashi mo kawareru no nara Moshi kawareru no nara shiro ni naru...";
    struct FAT32DriverRequest req2={
        .name = "lirik",
        .ext = "txt",
        .buffer_size = 874, 
        .buf = &bufer,
        .parent_cluster_number = ROOT_CLUSTER_NUMBER
    };
    syscall(WRITE, (uint32_t)&req2, (uint32_t)&ret, 0);

    char bufer2[1200] = "Narrator:    According to all known laws of aviation, there is no way that a bee should be able to fly. Its wings are too small to get its fat little body off the ground. The bee, of course, flies anyway because bees don't care what humans think is impossible.\n     cut to Barry's room, where he's picking out what to wear\nBarry    Yellow, black. Yellow, black. Yellow, black. Yellow, black. Ooh, black and yellow! Yeah, let's shake it up a little.\nMom (Janet Benson)    (calling from downstairs:) Barry! Breakfast is ready!\nBarry:    Coming! (phone rings) Oh, hang on a second. (adjusts his antennas into a headset) Hello?\nAdam Flayman    (on the phone) Barry?\nBarry:    Adam?\nAdam:    Can you believe this is happening?\nBarry:    I can't believe it. I'll pick you up. (hangs up, sharpens his stinger) Lookin' sharp. (flies downstairs)\nMom:    Barry, why don't you use the stairs? Your father paid good money for those.\nBarry:    Sorry. I'm excited.";
    struct FAT32DriverRequest req3={
        .name = "gila",
        .ext = "txt",
        .buffer_size = 1200, 
        .buf = &bufer2,
        .parent_cluster_number = ROOT_CLUSTER_NUMBER
    };
    syscall(WRITE, (uint32_t)&req3, (uint32_t)&ret, 0);   
 
    init();
    while (true){
        put_chars("SeleksiSister", BIOS_LIGHT_BLUE);
        put_chars("-2025", BIOS_LIGHT_GREEN);
        put_chars(":", BIOS_LIGHT_BLUE);
        print_curr_dir(state);
        put_chars("> ", BIOS_GREY);
        get_prompt();
        run_prompt(state.prompt_val);
        refresh_dir();
    }
    return 0;
}