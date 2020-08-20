/*
  Jorge Rodríguez,
  Roi Santos Ríos, roi.santos.rios@udc.es
*/
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include <sys/mman.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/resource.h>
#include "HeadLinkedList.h"
#include "list.h"
#include "listPR.h"
#define ANSI_COLOR_BLUE    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define MAXLINE 2048
#define MAXCOM 50
#define MAX_HOST_NAME 20
#define MAX_PATH_LENGTH 4096
#define LEERCOMPLETO ((ssize_t)-1)

struct COMMANDS {
  char *name;
  void (*funcion) (/*char *tr[], int i, list *hist*/);
};

int isFile(const char* name) {
    DIR* directory = opendir(name);
    if(directory != NULL) {
     closedir(directory);
     return 0; }

    if(errno == ENOTDIR) {
     return 1; }
    return -1;
}

void remove_file(const char *path) {
  int status;
  const char* local_file = path;
  char* ts2 = strdup(local_file);
  char* filename = basename(ts2);
  status = remove(filename);

  if (status == 0)
    printf("%s file deleted successfully.\n", filename);
  else
  {
    printf("Unable to delete the file\n");
    perror("Following error occurred");
  }
}

int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;
   if (d)
   {
      struct dirent *p;
      r = 0;
      while (!r && (p=readdir(d)))
      {
          int r2 = -1;
          char *buf;
          size_t len;
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;
          len = path_len + strlen(p->d_name) + 2;
          buf = malloc(len);
          if (buf)
          {
             struct stat statbuf;
             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf))
             {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = remove_directory(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);
   }
   if (!r)
      r = rmdir(path);
   return r;
}

int TrocearCadena(char *cadena, char *trozos[]) {
  int i = 1;

  if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
    return 0;
  while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
    i++;
  return(i);
}

char TipoFichero (mode_t m) {
  switch (m&S_IFMT) {
    case S_IFSOCK: return 's';
    case S_IFLNK: return 'l';
    case S_IFREG: return '-';
    case S_IFBLK: return 'b';
    case S_IFDIR: return 'd';
    case S_IFCHR: return 'c';
    case S_IFIFO: return 'p';
    default: return '?';
  }
}

char * ConvierteModo2 (mode_t m) {
static char permisos[12];
strcpy (permisos,"---------- ");
permisos[0]=TipoFichero(m);
if (m&S_IRUSR) permisos[1]='r';
if (m&S_IWUSR) permisos[2]='w';
if (m&S_IXUSR) permisos[3]='x';
if (m&S_IRGRP) permisos[4]='r';
if (m&S_IWGRP) permisos[5]='w';
if (m&S_IXGRP) permisos[6]='x';
if (m&S_IROTH) permisos[7]='r';
if (m&S_IWOTH) permisos[8]='w';
if (m&S_IXOTH) permisos[9]='x';
if (m&S_ISUID) permisos[3]='s';
if (m&S_ISGID) permisos[6]='s';
if (m&S_ISVTX) permisos[9]='t';
return (permisos);
}

void info(char *tr, struct stat stats) {
    struct tm dt;
    struct passwd *pwd;
    struct group *grp;
    struct stat stats_link;

    //Inode number
    printf("%ld ", stats.st_ino);

    // File permissions
    printf("%s", ConvierteModo2(stats.st_mode));

      //Number of hard links
    printf(" %ld", stats.st_nlink);

    //Name of user
    pwd = getpwuid(stats.st_uid);
    printf(" %s", pwd->pw_name);

    //Name of group
    grp = getgrgid(stats.st_gid);
    printf(" %s", grp->gr_name);

    // File size
    printf(" %ld", stats.st_size);

    // File modification time
    dt = *(gmtime(&stats.st_mtime));
    printf(" %d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900,
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);
    //File or directory name
    if (TipoFichero(stats.st_mode) == 'l'){
        stat(tr, &stats_link);
        char *linkName = malloc(stats_link.st_size + 1);
        stats_link.st_size = readlink(tr, linkName, stats_link.st_size + 1);
      printf(" %s -> %s\n",tr,linkName);
    } else
      printf(" %s\n",tr);
}

//LISTAS
void list1(const char *name) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            lstat(entry->d_name, &stats);
            printf("%s %ld\n", entry->d_name, stats.st_size);
            chdir(cwd);
        }
    }
    closedir(dir);
}

void listR(const char *name, int indent) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            char path[1024];
            lstat(entry->d_name, &stats);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s%s %ld\n", indent, "", entry->d_name, stats.st_size);
            chdir(cwd);
            listR(path, indent + 2);
        }
    }
    closedir(dir);
}

void listV(const char *name) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            lstat(entry->d_name, &stats);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (strncmp(entry->d_name, ".", 1)!=0)
              printf("%s %ld\n", entry->d_name, stats.st_size);
          chdir(cwd);
        }
    }
    closedir(dir);
}

void listL(const char *name) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
      return;

    while ((entry = readdir(dir)) != NULL) {
      stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            lstat(entry->d_name, &stats);
              info(entry->d_name, stats);
              chdir(cwd);
        }
    }
    closedir(dir);
}

void listLV(const char *name) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
      stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            lstat(entry->d_name, &stats);
              if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                  continue;
              if (strncmp(entry->d_name, ".", 1)!=0)
                info(entry->d_name, stats);
              chdir(cwd);
        }
    }
    closedir(dir);
}

void listLR(const char *name, int indent) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            char path[1024];
            lstat(entry->d_name, &stats);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s", indent, "");
            info(entry->d_name, stats);
            chdir(cwd);
            listLR(path, indent + 2);
        }
    }
    closedir(dir);
}

void listRV(const char *name, int indent) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            char path[1024];
            lstat(entry->d_name, &stats);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (strncmp(entry->d_name, ".", 1)!=0) {
              snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
              printf("%*s%s %ld\n", indent, "", entry->d_name, stats.st_size);
            }
            chdir(cwd);
            listRV(path, indent + 2);
        }
    }
    closedir(dir);
}

void listRVL(const char *name, int indent) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat, stats;
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, sizeof(cwd));

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        stat(name, &path_stat);
        if (S_ISDIR(path_stat.st_mode) == 1) {
            chdir(name);
            char path[1024];
            lstat(entry->d_name, &stats);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s", indent, "");
            info(entry->d_name, stats);
            chdir(cwd);
            listRVL(path, indent + 2);
        }
    }
  closedir(dir);
}

void cmdListar(char *tr[], int i) {

  struct stat stats;
  int a = 0;
  int f = 1;
  int j;

  while (f<=i-1){
    if (strcmp(tr[f], "-r")==0){
      a = a + 100;
    } else if (strcmp(tr[f], "-v")==0) {
      a = a + 10;
    } else if (strcmp(tr[f], "-l")==0) {
      a = a + 1;
    }
    f++;
  }

  switch (a) {

    case 0: //listar
    if (!tr[1]){
      list1(".");
    } else {
      j = 1;
       while (j <= i-1){
        if (stat(tr[j], &stats)==0){
          if (S_ISDIR(stats.st_mode)){
            list1(tr[j]);
          } else
            printf("%s %ld\n", tr[j], stats.st_size);
        } else {perror(tr[j]);}
          j++;
      }
    }
    break;

    case 1: //listar -l
      if (!tr[2]) {
        listL(".");
      } else {
        j = 2;
         while (j <= i-1){
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listL(tr[j]);
            } else
              info(tr[j],stats);
            } else {perror(tr[j]);}
              j++;
        }
      }
    break;

    case 10: //listar -v
      if (!tr[2]) {
        listV(".");
      } else {
        j = 2;
         while (j <= i-1){
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listV(tr[j]);
              j++;
            } else
              printf("%s %ld\n", tr[j], stats.st_size);
            } else {perror(tr[j]);}
              j++;
        }
      }
    break;

    case 11: //listar -v -l
      if (!tr[3]) {
        listLV(".");
      } else {
        j = 3;
         while (j <= i-1){
           if (stat(tr[j], &stats)==0){
             if (S_ISDIR(stats.st_mode)){
               listLV(tr[j]);
             } else
               info(tr[j],stats);
             } else {perror(tr[j]);}
               j++;
        }
      }
    break;

    case 100: //listar -r
      if (!tr[2]){
        listR(".",0);
      } else {
        j = 2;
        while (j <= i-1) {
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listR(tr[j],0);
            } else
              printf("%s %ld\n", tr[j], stats.st_size);
            } else {perror(tr[j]);}
              j++;
         }
       }
    break;

    case 101: //listar -r -l
      if (!tr[3]){
        listLR(".",0);
      } else {
        j = 3;
        while (j <= i-1) {
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listLR(tr[j],0);
            } else
              info(tr[j],stats);
            } else {perror(tr[j]);}
              j++;
         }
       }
    break;

    case 110: //listar -r -v
      if (!tr[3]){
        listRV(".",0);
      } else {
        j = 3;
        while (j <= i-1) {
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listRV(tr[j],0);
            } else
              printf("%s %ld\n", tr[j], stats.st_size);
            } else {perror(tr[j]);}
              j++;
         }
       }
    break;

    case 111: //listar -r -v -l
      if (!tr[4]){
        listRVL(".",0);
      } else {
        j = 4;
        while (j <= i-1) {
          if (stat(tr[j], &stats)==0){
            if (S_ISDIR(stats.st_mode)){
              listRVL(tr[j],0);
            } else
              info(tr[j],stats);
            } else {perror(tr[j]);}
              j++;
         }
       }
    break;

    default:
      return;
    break;
  }
}

void cmdInfo(char *tr[], int i) {
    int j = 1;
    struct stat stats;

    while (j<=i-1) {
      if (lstat(tr[j], &stats) == 0)
        info(tr[j], stats);
      else
        perror(tr[j]);
      j++;
    }
}

void cmdCrear(char *tr[], int i) {
  char cwd[256];
  if (i==1)
    printf("%s",getcwd(cwd, sizeof(cwd)));
  else {
    if (strcmp(tr[1], "-d") == 0) {
      if (tr[2] != NULL) {
        mkdir(tr[2],0700);
        printf("Directory %s%s%s successfully created", ANSI_COLOR_BLUE ,tr[2], ANSI_COLOR_RESET);
      }

      else {
        struct dirent *de;
        DIR *dr = opendir(".");
        if (dr == NULL)
            printf("Could not open current directory" );
        while ((de = readdir(dr)) != NULL) {
          if ((strcmp(de->d_name, ".")!=0) && strcmp(de->d_name, "..")!=0)
            printf("%s\n", de->d_name);
        }
        closedir(dr); }
    }
    else {
      FILE *fp;
      fp = fopen(tr[1], "w");
      free(fp); }
  }
}

void cmdBorrar(char *tr[], int i) {
  char cwd[256];
  int r;
  if (i==1)
    printf("%s",getcwd(cwd, sizeof(cwd)));
  else {
    if (strcmp(tr[1], "-r") == 0) {
      if (tr[2] != NULL) {
        if (isFile(tr[2])==0) {
          if (access(tr[2], F_OK) == 0)
            r = remove_directory(tr[2]);
              if (r == 0)
                printf("%s%s%s directory deleted successfully.",ANSI_COLOR_BLUE ,tr[2], ANSI_COLOR_RESET);
              else
                printf("%s%s%s directory could not be deleted.",ANSI_COLOR_BLUE ,tr[2], ANSI_COLOR_RESET);}
        else
          remove_file(tr[2]); }
    }
    else {
      r = rmdir(tr[1]);
      if (r == 0)
        printf("Directory %s%s%s deleted successfully.",ANSI_COLOR_BLUE ,tr[1], ANSI_COLOR_RESET);
      else
        printf("Directory %s%s%s could not be deleted.",ANSI_COLOR_BLUE ,tr[1], ANSI_COLOR_RESET);
    }
  }
}

void cmdAutores(char *tr[], int i) {
    if (i==1)
      printf("Jorge Rodríguez, jorge.rodriguez\nRoi Santos, roi.santos.rios");
    else {
      if (strcmp(tr[1], "-l") == 0)
        printf("jorge.rodriguez, roi.santos.rios");
      else if (strcmp(tr[1], "-n") == 0)
        printf("Jorge Rodríguez, Roi Santos");
    }

}

void cmdPid(char *tr[], int i) {
    if (i==1)
      printf("%d",getpid());
    else {
      if (strcmp(tr[1], "-p") == 0)
        printf("%d",getppid());
    }

}

void cmdDir(char *tr[], int i) {
    char cwd[256];
    char dir[] = "/";
    if (i==1)
      printf("%s",getcwd(cwd, sizeof(cwd)));
    else {
      if (strcmp(tr[1], "direct") == 0) {
        chdir(dir);
        printf("Your current directory has changed to: %s", getcwd(cwd, sizeof(cwd)));}
    }

}

void cmdFecha() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("fecha: %d-%d-%d %d:%d:%d",tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void cmdHora() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("hora: %d:%d:%d",tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void cmdEnd(char *tr[], int i, memList mem, procList proc, list hist) {
    //destroyList(hist);
    //mDestroyList(mem);
    //pDestroyList(&proc);
    exit(0);
}

void cmdHist(char *tr[], int i, memList mem, procList proc, list hist) {
  if (i==1)
    printList(hist);
  else {
    if (strcmp(tr[1], "-c") == 0) {
      destroyList(hist);
      //list hist = CreateList();
      }
    }
}

/*-----------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------*/


void * ObtenerMemoriaShmget (key_t clave, size_t tam, memList mem){
  void * p;
  int aux,id,flags=0777;
  struct shmid_ds s;
  if (tam) {  /*si tam no es 0 la crea en modo exclusivo */
    flags=flags | IPC_CREAT | IPC_EXCL;}
    /*si tam es 0 intenta acceder a una ya creada*/
  if (clave==IPC_PRIVATE){ /*no nos vale*/
    errno=EINVAL;
    return NULL;
    }
  if ((id=shmget(clave, tam, flags))==-1)
    return (NULL);

  if ((p=shmat(id,NULL,0))==(void*) -1) {
    aux=errno;   /*si se ha creado y no se puede mapear*/
    if (tam) {    /*se borra */
      shmctl(id,IPC_RMID,NULL);}
    errno=aux;
    return (NULL);
    }

  shmctl (id,IPC_STAT,&s);
  mInsert(mem, p, s.shm_segsz, "shared", clave, "");
  return (p);
}

void Cmd_AsignarCreateShared (char *arg[], memList mem){
  key_t k;
  size_t tam=0;
  void *p;
  if (arg[0]==NULL || arg[1]==NULL) {
    mPrintListParam(mem, "shared");
    return;
  }

  k=(key_t) atoi(arg[0]);
  if (arg[1]!=NULL){
    tam=(size_t) atoll(arg[1]);
  }

  if ((p=ObtenerMemoriaShmget(k,tam, mem))==NULL)
    perror ("Imposible obtener memoria shmget");
  else
  printf ("Memoria de shmget de clave %d asignada en %p\n",k,p);
}

void Cmd_AsignarShared (char *arg[], memList mem){
  key_t k;
  size_t tam=0;
  void *p;
  if (arg[0]==NULL) {
    mPrintListParam(mem, "shared");
    return;
  }

  k=(key_t) atoi(arg[0]);
  if ((p=ObtenerMemoriaShmget(k,tam, mem))==NULL)
    perror ("Imposible obtener memoria shmget");
  else
  printf ("Memoria de shmget de clave %d asignada en %p\n",k,p);
}

void * MmapFichero (char * fichero, int protection, memList mem)  {
  int df, map=MAP_PRIVATE,modo=O_RDONLY;
  struct stat s;
  void *p;

  if (protection&PROT_WRITE)
    modo=O_RDWR;

  if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
    return NULL;

  if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED) {
    return NULL; }
    mInsert(mem, p, s.st_size, "mmap", 0, fichero);
  return p;}

void Cmd_AsignarMmap (char *arg[], memList mem){
  char *perm;
  void *p;
  int protection=0;

  if (arg[0]==NULL) {
    mPrintListParam(mem, "mmap");
    return;
  }

  if ((perm=arg[1])!=NULL && strlen(perm)<4) {
    if (strchr(perm,'r')!= NULL)
      protection|=PROT_READ;
    if (strchr(perm,'w')!=NULL)
      protection|=PROT_WRITE;
    if (strchr(perm,'x')!=NULL)
      protection|=PROT_EXEC;
  }

  if ((p=MmapFichero(arg[0],protection, mem))==NULL)
    perror ("Imposible mapear fichero");
  else
    printf ("fichero %s mapeado en %p\n", arg[0], p);}

ssize_t LeerFichero (char *fich, void *p, ssize_t n)  /*n=-1 indica que se lea todo*/{
  ssize_t  nleidos,tam=n;int df, aux;
  struct stat s;
  if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
    return ((ssize_t)-1);
  if (n==LEERCOMPLETO)
    tam=(ssize_t) s.st_size;
  if ((nleidos=read(df,p, tam))==-1){
    aux=errno;close(df);
    errno=aux;
    return ((ssize_t)-1);
  }
  close (df);
  return (nleidos); }

void cmdBorrakey (char *args[]) {
  key_t clave;
  int id;
  char *key=args[1];
  if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
    printf ("   rmkey  clave_valida\n");
    return;
  }
  if ((id=shmget(clave,0,0666))==-1){
    perror ("shmget: imposible obtener memoria compartida");
    return;
  }
  if (shmctl(id,IPC_RMID,NULL)==-1)
  perror ("shmctl: imposible eliminar memoria compartida\n");
}

void asignarMalloc(char *tr[], memList mem) {

  if (tr[2]!=NULL){
    size_t space = atoi(tr[2]);
    int *ptr = malloc(space);

    if (ptr==NULL)
      perror("Memory not allocated\n");
    else {
      printf("allocated %d at %p\n", atoi(tr[2]), ptr);
      mInsert(mem, ptr, space, "malloc", 0, "");
    }
  } else {
    mPrintListParam(mem, "malloc");
    }
}

void cmdAsignar(char* tr[], int i, memList mem) {
  if (tr[1] == NULL){
    mPrintList(mem);
  } else if (strcmp(tr[1], "-createshared") == 0) {
    Cmd_AsignarCreateShared(tr+2, mem);
  } else if (strcmp(tr[1], "-shared") == 0) {
    Cmd_AsignarShared(tr+2, mem);
  } else if (strcmp(tr[1], "-malloc") == 0) {
      asignarMalloc(tr, mem);
  } else if (strcmp(tr[1], "-mmap") == 0)
      Cmd_AsignarMmap(tr+2, mem);
}

void cmdVolcar(char* tr[], int i, memList mem) {
  int length, a;
  char length2[5];

  if (i == 2)
    length = 25;
  else {
    strcpy(length2,tr[2]);
    length = atoi(length2);
  }
  unsigned char* p = (unsigned char*) strtoul(tr[1],NULL,16);
  int e;
  for (a=1; a < length+1; a++) {
    printf("%-3x", *(p + a - 1));
    if (((a % 25 == 0) || (a == length)) && (a != 0)) {
      printf("\n");
      int n;
      if ((a == length) && (length % 25 != 0))
        n = length % 25;
      else
        n = 25;
      for (e = 0; e < n; e++) {
        int w = a-n+e;
        if (isprint((char) *(p + w)) != 0)
          printf( "%-3c", *(p + w));
        else printf("%-3s", " ");
        if (e == 24)
          printf("\n");
      }
    }
  }
}

void cmdLlenar(char* tr[], int i, memList mem) {
  int cont;
  cont = atoi(tr[2]);
  int byte = strtol(tr[3],NULL,16);
  unsigned char* p = (unsigned char*) strtoul(tr[1],NULL,16);

  if (memset(p,byte,cont)==NULL)
    printf("Could not fill the specified memory address");
  else
    printf("Memory address %p filled with %d bytes of %c", p, cont, byte);
}

void recursiva (int n) {
  char automatico[2048];
  static char estatico[2048];
  printf ("parametro n:%d en %p\n",n,&n);
  printf ("array estatico en:%p \n",estatico);
  printf ("array automatico en %p\n",automatico);
  n--;
  if (n>0)
    recursiva(n);
}

void cmdRecursiva (char *tr[], int i, memList mem) {
  char n2[2048];
  strcpy(n2,tr[1]);
  int n = atoi(n2);
  recursiva(n);
}

void Rfich (char *path, void *p, ssize_t i, struct stat stats) {
	int file;

	 if ((file=open(path, O_RDONLY)) == -1)
			printf("Could not open the specified file\n");

  if ((i==-1) || (i > stats.st_size))
    		i=(ssize_t) stats.st_size;

	if (read(file,p, i)==-1) {
		close(file);
		printf("Could not write %ld bytes of file %s%s%s in %p\n", i, ANSI_COLOR_BLUE , path , ANSI_COLOR_RESET, p);
	}
  else printf("%ld bytes of file %s%s%s have been written in %p\n", i, ANSI_COLOR_BLUE , path, ANSI_COLOR_RESET, p);
	close (file);
}

void cmdRfich (char* tr[], int n) {
  struct stat stats;
  if (lstat(tr[1], &stats) == 0) {
    if (n==4)
      Rfich(tr[1], (char*)strtoul(tr[2],NULL,16), (ssize_t) atoi(tr[3]), stats);
    else if (n==3)
      Rfich(tr[1], (char*)strtoul(tr[2],NULL,16),(ssize_t) -1, stats);
    else printf("Number of arguments not valid\n");
  }
  else perror(tr[1]);
}

void Wfich (char *path, int perm, void *p, ssize_t i) {
	int file;

	if ((file=open(path,perm,0777)) == -1)
			printf("Could not open the specified file");

	if (write(file,p, i)==-1) {
		close(file);
		printf("Could not write the file %s%s%s with %ld bytes of %p", ANSI_COLOR_BLUE , path, ANSI_COLOR_RESET, i, p);
	    }
  else printf("The file %s%s%s has been written with %ld bytes in %p",  ANSI_COLOR_BLUE , path, ANSI_COLOR_RESET, i, p);
	close (file);
}

void cmdWfich (char* tr[], int n) {
	if (n==4)
		Wfich(tr[1], O_RDONLY | O_EXCL | O_WRONLY | O_CREAT, (char*)strtoul(tr[2],NULL,16), (ssize_t) atoi(tr[3]));
  else if ((n==5) && (strcmp(tr[1], "-o")==0))
      Wfich(tr[2], O_RDONLY | O_WRONLY | O_TRUNC | O_CREAT , (char*)strtoul(tr[3],NULL,16), (ssize_t) atoi(tr[4]));
	else printf("Passed arguments not valid");
}

void desasignarMalloc(memList mem, int s) {
  if (mFindSize(mem,s) != NULL) {
    int* p = mPointer(mFindSize(mem,s));
    printf("block at address %p deallocated (malloc)\n", p);
    free(p);
    mDeleteSize(mem,s);
  } else
    printf("Not found\n");
}

void desasignarShared(memList mem, int clave) {
  if (mFindClave(mem,clave) != NULL) {
    int* p = mPointer(mFindClave(mem,clave));
    printf("block at address %p deallocated (shared)\n", p);
    shmdt(p);
    mDeleteClave(mem,clave);
  } else
    printf("Not found\n");
}

void desasignarMmap(memList mem, char* file) {
  if (mFindFile(mem,file) != NULL) {
    int* p = mPointer(mFindFile(mem,file));
    size_t s = mSize(mFindFile(mem,file));
    printf("block at address %p deallocated (mmap)\n", p);
    munmap(p, s);
    mDeleteFile(mem,file);
  } else
    printf("Not found\n");
}

void desasignarAdr(memList mem, char* ptr) {
  char* p;
  long int adr = strtol(ptr, &p, 16);
  if (mFindAdr(mem,adr) != NULL) {
    char* cmd = mCmd(mFindAdr(mem,adr));
    if (strcmp(cmd, "malloc") == 0)
        desasignarMalloc(mem, mSize(mFindAdr(mem,adr)));
    else if (strcmp(cmd, "mmap") == 0)
        desasignarMmap(mem, mFile(mFindAdr(mem,adr)));
    else if (strcmp(cmd, "shared") == 0)
        desasignarShared(mem, mClave(mFindAdr(mem,adr)));
  } else
    printf("Not found\n");
}

void cmdDesasignar(char* tr[], int i, memList mem) {
  if (tr[1] == NULL){
    mPrintList(mem);
  } else if (strcmp(tr[1], "-malloc") == 0) {
    if (tr[2] == NULL)
      mPrintListParam(mem, "malloc");
    else
      desasignarMalloc(mem, atoi(tr[2]));
  } else if (strcmp(tr[1], "-mmap") == 0) {
    if (tr[2] == NULL)
      mPrintListParam(mem, "mmap");
    else
      desasignarMmap(mem, tr[2]);
  } else if (strcmp(tr[1], "-shared") == 0) {
    if (tr[2] == NULL)
      mPrintListParam(mem, "shared");
    else
      desasignarShared(mem, atoi(tr[2]));
  } else {
    desasignarAdr(mem, tr[1]);
  }
}

void cmdMem (char* tr[], int i, memList mem){
  if (tr[1] == NULL){
  	int a;
    float b;
  	char c;
  	printf("Memory dir of function Cmd_AsignarMmap: %p\n",&Cmd_AsignarMmap );
  	printf("Memory dir of function desasignarAdr: %p\n",&desasignarAdr);
  	printf("Memory dir of function cmdDir: %p\n",&cmdDir);
  	printf("Memory dir of external variable memory list: %p\n",&mem);
  	printf("Memory dir of extrenal variable tr[]: %p\n",&tr);
  	printf("Memory dir of external variablei: %p\n",&i);
  	printf("Memory dir of local variable a: %p\n",&a);
  	printf("Memory dir of local variable b: %p\n",&b);
  	printf("Memory dir of local variable c: %p\n",&c);

  } else if (strcmp(tr[1], "-malloc") == 0) {
      mPrintListParam(mem, "malloc");
  } else if (strcmp(tr[1], "-mmap") == 0) {
      mPrintListParam(mem, "mmap");
  } else if (strcmp(tr[1], "-shared") == 0) {
      mPrintListParam(mem, "shared");
  }
}

/*-----------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------*/

void cmdPriority(char* tr[]) {

  int pid, prior;

  if (tr[1] != NULL) {
    pid = atoi(tr[1]);
    if (tr[2] != NULL){
      prior = atoi(tr[2]);
      if (setpriority(PRIO_PROCESS, pid, prior) != -1)
				printf("New priority of PID %d is %d\n", pid, prior);
			else
				perror("");
    } else {
      prior = getpriority(PRIO_PROCESS, pid);
      if (prior != -1)
				printf("The priority of PID %d is %d\n", pid, prior);
			else
				perror("");
    }
  }
}

void cmdFork(){

  pid_t pid = fork();
  int status;

  if (pid == 0) {
    printf("Executing child process: %d\n", getpid());
  } else if (pid == -1) {
    printf("Could not create child process");
  } else {
    waitpid(pid,&status,0);
  }
}

void doExec (char *tr[]){

  int prio;

  if(tr[0]!=NULL && tr[0][0]=='@'){
    prio=atoi(tr[0]+1);
   if (setpriority(PRIO_PROCESS, getpid(), prio) == -1){
			    perror("ERROR: when changing the priority");
          return;
   }
   tr++;
  }
 if (execvp(tr[0],tr)==-1)
    perror ("Cannot execute");
}

void cmdExec(char *tr[], int i, memList mem, procList proc) {

  doExec (tr+1);
}

int isForeground(char *tr[])
{
   int i;

   for (i=0; tr[i]!=NULL; i++)
      if (!strcmp(tr[i],"&")){
          tr[i]=NULL;
          return 0;
      }
  return 1;
}

void doProcess (char *tr[], int foreground, procList proc)
{
  pid_t pid;
  int fg=foreground && isForeground(tr);
  if ((pid=fork())==0){
      doExec(tr);
      exit (127);
  }
  if (fg)
      waitpid(pid,NULL,0);
  else
      pInsert(proc, pid, *tr);
  }

void cmdSplano (char *tr[], int i, memList mem, procList proc)
{
  doProcess (tr+1,0, proc);
}

void cmdPplano(char *tr[], int i, memList mem, procList proc)
{
  doProcess (tr+1,1, proc);
}

int toForeground(char *tr[])
{
   int i;

   for (i=0; tr[i]!=NULL; i++)
      if (!strcmp(tr[i],"-fg")){
          tr[i]=NULL;
          return 0;
      }
  return 1;
}

void cmdProc(char *tr[], int i, memList mem, procList proc){
  int pos;
  if (i == 3) {
    if(strcmp(tr[1], "-fg") == 0) {
      int pid = atoi(tr[2]);
      if ((pos = pSearchPid(proc, pid)) != -1){
        waitpid(pid,NULL,0);
      }
    }
  } else if (i == 2){
    if(strcmp(tr[1], "-fg") == 0)
      pPrintList(proc);
    else {
      int pid = atoi(tr[1]);
      if ((pos = pSearchPid(proc, pid)) != -1){
        pUpdateNodes(proc);
        printElement(proc, pos);
      }
    }
  } else
    pPrintList(proc);
}

void cmdListarProcs(char *tr[], int i, memList mem, procList proc)
{
  pPrintList(proc);
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
}

void cmdBorrarProcs(char *tr[], int i, memList mem, procList proc){
  if (i>1) {
    if (strcmp(tr[1], "-term") == 0)
      pDeleteTerminated(proc);
    else if (strcmp(tr[1], "-sig") == 0)
      pDeleteSignal(proc);
  } else
    pPrintList(proc);
  //pDeleteSignal(proc);
}
/*pid_t pid = atoi(tr[1]);
kill(pid,9);*/

/*-----------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------*/

struct COMMANDS comandos[] = {
  {"autores", cmdAutores},
  {"pid", cmdPid},
  {"cdir", cmdDir},
  {"fecha", cmdFecha},
  {"hora", cmdHora},
  {"fin", cmdEnd},
  {"end", cmdEnd},
  {"exit", cmdEnd},
  {"hist", cmdHist},
  {"listar", cmdListar},
  {"crear", cmdCrear},
  {"borrar", cmdBorrar},
  {"info", cmdInfo},
  {"asignar", cmdAsignar},
  {"desasignar", cmdDesasignar},
  {"rmkey", cmdBorrakey},
  {"mem", cmdMem},
  {"volcar", cmdVolcar},
  {"llenar", cmdLlenar},
  {"recursiva", cmdRecursiva},
  {"rfich", cmdRfich},
  {"wfich", cmdWfich},
  {"priority", cmdPriority},
  {"fork", cmdFork},
  {"pplano", cmdPplano},
  {"splano", cmdSplano},
  {"exec", cmdExec},
  {"proc", cmdProc},
  {"listarprocs", cmdListarProcs},
  {"borrarprocs", cmdBorrarProcs},
  {NULL, NULL},
};

void ProcessInput (char *in, memList mem, procList proc, list hist) {
  int i;
  int a;
  char * tr[MAXCOM];

  Insert(in, hist);

  a = TrocearCadena(in, tr);
  if (a==0)
    return;

  for (i = 0; comandos[i].name != NULL; i++)
    if (strcmp(tr[0], comandos[i].name) == 0) {
      (*comandos[i].funcion)(tr, a, mem, proc, hist);
      return;
    }
  doProcess(tr,1,proc);

}

int main(int argc, char *argv[]){
  char input [MAXLINE];
  list hist = CreateList();
  memList mem = mCreateList();
  procList proc;
  pCreateList(&proc);

  while (1){
    printf("\n -> ");
    fgets(input,MAXLINE,stdin);
    ProcessInput(input, mem, proc, hist);
  }
}
