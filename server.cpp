#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <utmp.h>
#include <cctype>
#include <stack>
#include <set>
#include <map>
#include "Books.h"
#include "Users.h"

#define SIZE 3000
#define PORT 2908

typedef struct thData {
    int idThread;
    int cl;
} thData;

pthread_t th[100];
std::map<int, int> mapThToNr;
std::map<int, int> mapThreadToUser;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER; // variabila mutex ce va fi partajata de threaduri

struct sockaddr_in server;
struct sockaddr_in from;
Users u[100];
Books b[100];
int thNumber = 0;
int sd;
pid_t pid;

void getBooksInOrder(std::vector <Books> &pref)
{
    std::map<int, Books> mmap;
    Books aux_book;
    float vec[5] = {-1, -1, -1, -1, -1};
    for(auto bi : pref)
    {
        if(bi.getRating() >= vec[0])
        {
            mmap[4] = mmap[3];
            vec[4]  = vec[3];
            mmap[3] = mmap[2];
            vec[3]  = vec[2];
            mmap[2] = mmap[1];
            vec[2]  = vec[1];
            mmap[1] = mmap[0];
            vec[1]  = vec[0];
            mmap[0] = bi;
            vec[0]  = bi.getRating(); 
        }
        else if(bi.getRating() >= vec[1])
        {
            mmap[4] = mmap[3];
            vec[4]  = vec[3];
            mmap[3] = mmap[2];
            vec[3]  = vec[2];
            mmap[2] = mmap[1];
            vec[2]  = vec[1];
            mmap[1] = bi;
            vec[1]  = bi.getRating(); 
        }
        else if(bi.getRating() >= vec[2])
        {
            mmap[4] = mmap[3];
            vec[4]  = vec[3];
            mmap[3] = mmap[2];
            vec[3]  = vec[2];
            mmap[2] = bi;
            vec[2]  = bi.getRating(); 
        }
        else if(bi.getRating() >= vec[3])
        {
            mmap[4] = mmap[3];
            vec[4]  = vec[3];
            mmap[3] = bi;
            vec[3]  = bi.getRating(); 
        }
        else if(bi.getRating() >= vec[4])
        {
            mmap[4] = bi;
            vec[4]  = bi.getRating(); 
        }
    }
    pref.clear();
    pref.push_back(mmap[0]);
    pref.push_back(mmap[1]);
    pref.push_back(mmap[2]);
    pref.push_back(mmap[3]);
    pref.push_back(mmap[4]);
}

void InfoAuthor(char *autor, char *utilizator, char rasp[], int &lungRasp, Users U)
{
    std::string Autor(autor);
    if(!Books::getAuthorInfo(Autor, rasp, lungRasp))
    {
        strcpy(rasp, "Autorul nu a fost găsit.");
        lungRasp = strlen(rasp);
    }
}

void InfoBook(char *param, char *utilizator, char rasp[], int &lungRasp, Users U)
{
    char titlu[100], autor[100];
    bzero(&titlu, 100);
    bzero(&autor, 100);
    strncpy(titlu, param, strlen(param) - strlen(strchr(param, '_')));
    strcpy(autor, param + strlen(titlu) + 1);
    std::string Titlu(titlu), Autor(autor);
    bool ok = false;
    for(int i = 0; i < Books::getNumberOfBooks(); i++)
        if(b[i].getAuthorOfBook() == Autor && b[i].getTitleOfBook() == Titlu)
        {
            strcpy(rasp, b[i].getBookInfos().c_str());
            lungRasp = strlen(rasp);
            ok = true;
            break;
        }
    if(!ok)
    {
        strcpy(rasp, "Cartea nu a fost găsită.");
        lungRasp = strlen(rasp);
    }
}

void Clasament(std::vector<Books> &preferinte, Users U)
{
    std::map<std::string, bool> exista;
    for(int i = 0; i < preferinte.size(); i++)
        exista[preferinte[i].getISBNOfBook()] = true;
    if(preferinte.size() > 5)
    {
        std::string subgen_preferat = U.getFavoriteSubgenre();
        std::vector<Books> pref;
        std::map<std::string, bool> existaInPref;
        if(subgen_preferat != "niciunul")
            for(int i = 0; i < Books::getNumberOfBooks(); i++)
            {
                if(exista[b[i].getISBNOfBook()])
                {
                    std::size_t found = b[i].getAllSubgenresOfBook().find(subgen_preferat);
                    if(found != std::string::npos)
                    {
                        pref.push_back(b[i]);
                        existaInPref[b[i].getISBNOfBook()] = true;
                    }
                }
            }
        std::string autor_preferat = U.getFavoriteAuthor();
        if(pref.size() != 5)
        {
            if(autor_preferat != "niciunul")
            {
                if(pref.size() < 5)
                {
                    for(int i = 0; i < Books::getNumberOfBooks() && pref.size() < 5; i++)
                        if(exista[b[i].getISBNOfBook()] && !existaInPref[b[i].getISBNOfBook()] && b[i].getAuthorOfBook() == autor_preferat)
                        {
                            pref.push_back(b[i]);
                            existaInPref[b[i].getISBNOfBook()] = true;
                        }
                }
                else
                {
                    std::vector<Books> preferinte1;
                    std::map<std::string, bool> exista1;
                    for(auto pf : pref)
                    {
                        preferinte1.push_back(pf);
                        exista1[pf.getISBNOfBook()] = true;
                    }
                    pref.clear();
                    existaInPref.clear();
                    for(auto book : preferinte1)
                    {
                        if(exista1[book.getISBNOfBook()] && book.getAuthorOfBook() == autor_preferat)
                        {
                            pref.push_back(book);
                            existaInPref[book.getISBNOfBook()] = true;
                        }
                    }
                    if(pref.size() < 5)
                    {
                        for(auto book : preferinte)
                        {
                            if(existaInPref[book.getISBNOfBook()] == false)
                            {
                                pref.push_back(book);
                                existaInPref[book.getISBNOfBook()] = true;
                            }
                        }
                    }
                }
            }
            if(pref.size() != 5)
            {
                std::vector<Books> similar = U.getBooksOfAnUserWithSimilarTaste(b);
                if(pref.size() < 5)
                {
                    for(auto b : similar)
                    {
                        if(exista[b.getISBNOfBook()] && !existaInPref[b.getISBNOfBook()])
                        {
                            pref.push_back(b);
                            existaInPref[b.getISBNOfBook()] = true;
                        }
                    }
                }
                else
                {
                    std::vector<Books> preferinte1;
                    std::map<std::string, bool> exista1;
                    for(auto pf : pref)
                    {
                        preferinte1.push_back(pf);
                        exista1[pf.getISBNOfBook()] = true;
                    }
                    pref.clear();
                    existaInPref.clear();
                    for(auto book : similar)
                    {
                        if(exista1[book.getISBNOfBook()])
                        {
                            pref.push_back(book);
                            existaInPref[book.getISBNOfBook()] = true;
                        }
                    }
                    if(pref.size() < 5)
                    {
                        for(auto book : preferinte)
                        {
                            if(existaInPref[book.getISBNOfBook()] == false)
                            {
                                pref.push_back(book);
                                existaInPref[book.getISBNOfBook()] = true;
                            }
                        }
                    }
                }
                if(pref.size() != 5)
                {
                    if(pref.size() < 5)
                    {
                        getBooksInOrder(preferinte);
                        for(auto pf : preferinte)
                        {
                            if(pref.size() == 5)
                                break;
                            if(existaInPref[pf.getISBNOfBook()] == false)
                                pref.push_back(pf);
                        }
                    }
                    else
                    {
                        getBooksInOrder(pref);
                    }
                }
            }
        }
        preferinte.clear();
        for(auto pf : pref)
        {
            preferinte.push_back(pf);
        }
    }
}

void Evalueaza(char *param, char *utilizator, char rasp[], int &lungRasp, Users U)
{
    char titlu[100], autor[100], nota[10];
    bzero(&titlu, 100);
    bzero(&autor, 100);
    bzero(&nota, 10);
    strncpy(titlu, param, strlen(param) - strlen(strchr(param, '_')));
    strcpy(nota, param + strlen(param) - strlen(strrchr(param, '_')) + 1);
    strncpy(autor, param + strlen(titlu) + 1, strlen(param) - strlen(strrchr(param, '_')) - strlen(titlu) - 1);
    bool notaIncorecta = false;
    if(strlen(nota) > 3)
        notaIncorecta = true;
    for(int i = 0; i < strlen(nota) && !notaIncorecta; i++)
        if(!(isdigit(nota[i]) || nota[i] == '.' || nota[i] == ','))
            notaIncorecta = true;
    int nrv = 0;
    for(int i = 0; i < strlen(nota) && !notaIncorecta; i++)
    {
        if(nota[i] == '.' || nota[i] == ',')
            nrv++;
        if(nrv > 1)
            notaIncorecta = true;
    }
    if(!notaIncorecta && strlen(nota) == 3 && isdigit(nota[1]))
        notaIncorecta = true;
    if(!notaIncorecta && strlen(nota) == 2 && (nota[0] != '1' || nota[1] > '0'))
        notaIncorecta = true;
    if(!notaIncorecta && nota[0] == '0')
        notaIncorecta = true;
    if(notaIncorecta)
    {
        strcpy(rasp, "Nota este inadecvată.\nO notă reprezintă un număr între 1 și 10 cu cel mult o zecimală.");
        lungRasp = strlen(rasp);
    }
    else
    {
        std::string Titlu(titlu), Autor(autor);
        bool ok = false;
        for(int i = 0; i < Books::getNumberOfBooks(); i++)
        {
            if(b[i].getAuthorOfBook() == Autor && b[i].getTitleOfBook() == Titlu)
            {
                strcpy(rasp, "Cartea a fost evaluată cu succes.");
                lungRasp = strlen(rasp);
                ok = true;
                if(nota[1] == ',')
                    nota[1] = '.';
                U.insertEval(b[i].getTitleOfBook().c_str(), b[i].getAuthorOfBook().c_str(), nota, b[i].getGenreOfBook().c_str(), b[i].getAllSubgenresOfBook().c_str(), b[i].getYearOfBook().c_str());
                b[i].insertEval(i, nota);
                break;
            }
        }
        if(!ok)
        {
            strcpy(rasp, "Cartea nu a fost găsită.");
            lungRasp = strlen(rasp);
        }
    }
}

void Descarca(char *param, char *utilizator, char rasp[], int &lungRasp, Users U)
{
    char titlu[100], autor[100];
    bzero(&titlu, 100);
    bzero(&autor, 100);
    strncpy(titlu, param, strlen(param) - strlen(strchr(param, '_')));
    strcpy(autor, param + strlen(titlu) + 1);
    std::string Titlu(titlu), Autor(autor);
    bool ok = false;
    for(int i = 0; i < Books::getNumberOfBooks(); i++)
    {
        if(b[i].getAuthorOfBook() == Autor && b[i].getTitleOfBook() == Titlu)
        {
            strcpy(rasp, b[i].getURLOfBook().c_str());
            strcat(rasp, "\n");
            strcat(rasp, titlu);
            strcat(rasp, "-");
            strcat(rasp, autor);
            strcat(rasp, ".pdf\nCartea a fost descărcată cu succes.");
            lungRasp = strlen(rasp);
            ok = true;
            U.insertDownload(b[i].getTitleOfBook().c_str(), b[i].getAuthorOfBook().c_str(), b[i].getGenreOfBook().c_str(), b[i].getAllSubgenresOfBook().c_str(), b[i].getYearOfBook().c_str());
            break;
        }
    }
    if(!ok)
    {
        strcpy(rasp, "Cartea nu a fost găsită.");
        lungRasp = strlen(rasp);
    }
}

void RECOMANDARE(const char *criteriu, char *param, char *utilizator, char rasp[], int &lungRasp, Users U)
{
    std::vector<Books> preferinte;
    int nrCartiGasite = 0;
    bzero(rasp, SIZE);
    lungRasp = 0;
    bool criteriuValid = true;
    if(!strcmp(param, "nimic"))
    {
        for(int i = 0; i < Books::getNumberOfBooks(); i++)
        {
            preferinte.push_back(b[i]);
            nrCartiGasite++;
        }
    }
    else
    {
        std::string cautare(param);
        if(!strcmp(criteriu, "title"))
        {
            for(int i = 0; i < Books::getNumberOfBooks(); i++)
            {
                if(strstr(b[i].getTitleOfBook().c_str(), param))
                {
                    preferinte.push_back(b[i]);
                    nrCartiGasite++;
                }
            }
        }
        else
        {
            if(!strcmp(criteriu, "author"))
            {
                for(int i = 0; i < Books::getNumberOfBooks(); i++)
                {
                    if(b[i].getAuthorOfBook() == cautare)
                    {
                        preferinte.push_back(b[i]);
                        nrCartiGasite++;
                    }
                }
            }
            else
            {
                if(!strcmp(criteriu, "year"))
                {
                    for(int i = 0; i < Books::getNumberOfBooks(); i++)
                    {
                        if(b[i].getYearOfBook() == cautare)
                        {
                            preferinte.push_back(b[i]);
                            nrCartiGasite++;
                        }
                    }
                }
                else
                {
                    if(!strcmp(criteriu, "rating"))
                    {
                        for(int i = 0; i < Books::getNumberOfBooks(); i++)
                        {
                            if(b[i].getRatingOfBook() == cautare)
                            {
                                preferinte.push_back(b[i]);
                                nrCartiGasite++;
                            }
                        }
                    }
                    else
                    {
                        if(!strcmp(criteriu, "isbn"))
                        {
                            for(int i = 0; i < Books::getNumberOfBooks(); i++)
                            {
                                if(b[i].getISBNOfBook() == cautare)
                                {
                                    preferinte.push_back(b[i]);
                                    nrCartiGasite++;
                                }
                            }
                        }
                        else
                        {
                            if(!strcmp(criteriu, "genre"))
                            {
                                for(int i = 0; i < Books::getNumberOfBooks()- 3; i++)
                                {
                                    if(b[i].getGenreOfBook() == cautare)
                                    {
                                        preferinte.push_back(b[i]);
                                        nrCartiGasite++;
                                    }
                                }
                            }
                            else
                            {
                                if(!strcmp(criteriu, "subgenre"))
                                {
                                    for(int i = 0; i < Books::getNumberOfBooks() - 1; i++)
                                    {
                                        std::size_t found = b[i].getAllSubgenresOfBook().find(cautare);
                                        if(found != std::string::npos)
                                        {
                                            preferinte.push_back(b[i]);
                                            nrCartiGasite++;
                                        }
                                    }
                                }
                                else
                                {
                                    criteriuValid = false;
                                    strcpy(rasp, "Introduceți un criteriu valid!");
                                    lungRasp = strlen(rasp);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(nrCartiGasite == 0 && criteriuValid)
    {
        strcpy(rasp, "Nu s-a găsit o astfel de carte.");
        lungRasp = strlen(rasp);
    }
    else if(criteriuValid)
    {
        if(strcmp(criteriu, "nimic"))
            U.insertCautare(criteriu, param);
        if(nrCartiGasite > 5)
            Clasament(preferinte, U);
        int i = 0;
        strcpy(rasp, "Rezultatul căutării este:");
        for(auto book : preferinte)
        {
            strcat(rasp, "\n");
            i++;
            strcat(rasp, "[");
            char c[2];
            c[0] = char(i + '0');
            c[1] = '\0';
            strcat(rasp, c);
            strcat(rasp, "] ");
            strcat(rasp, book.getTitleOfBook().c_str());
            strcat(rasp, ", de ");
            strcat(rasp, book.getAuthorOfBook().c_str());
            strcat(rasp, ".");
        }
        lungRasp = strlen(rasp);
    }
}

void procesare(char *msg, char user[], char rasp[], int &lungRasp, int id, Users &U)
{
    lungRasp = 1;
    if(!strcmp(msg, "logout"))
    {
        if(strlen(user) < 3)
        {
            strcpy(rasp, "Userul nu este logat!");
            lungRasp = strlen(rasp);
        }
        else
        {
            bzero(user, strlen(user));
            strcpy(rasp, "Userul s-a delogat cu succes.");
            lungRasp = strlen(rasp);
        }
    }
    else
    {
        if(!strcmp(msg, "man"))
        {
            int manual = open("files/man_client.txt", O_RDONLY);
            lungRasp = read(manual, rasp, SIZE);
            while(rasp[strlen(rasp) - 1] != '"')
                rasp[strlen(rasp) - 1] = '\0';
        }
        else
        {
            if(!strcmp(msg, "recommend-books"))
            {
                if(strlen(user) < 3)
                {
                    strcpy(rasp, "Userul nu este logat!");
                    lungRasp = strlen(rasp);
                }
                else
                {
                    char criteriu[] = "nimic", param[] = "nimic";
                    RECOMANDARE(criteriu, param, user, rasp, lungRasp, U);
                }
            }
            else
            {
                if(!strcmp(msg, "info-genres"))
                {
                    if(strlen(user) < 3)
                    {
                        strcpy(rasp, "Userul nu este logat!");
                        lungRasp = strlen(rasp);
                    }
                    else
                        Books::printGenresAll(rasp, lungRasp);
                }
                else
                {
                    if(!strstr(msg, " : "))
                    {
                        strcpy(rasp, "Comandă inexistentă!");
                        lungRasp = strlen(rasp);
                    }
                    else
                    {
                        unsigned int j = 0;
                        char cmd[20], *param;
                        param = (char*)malloc(strlen(msg) * sizeof(char));
                        while(msg[j] != ' ')
                        {
                            cmd[j] = msg[j];
                            j++;
                        }
                        cmd[j] = '\0';
                        strcpy(param, msg + strlen(cmd) + 3);
                        if(!strcmp(cmd, "login"))
                        {
                            if(strlen(user) > 2)
                            {
                                strcpy(rasp, "Sunteți deja logat! - ");
                                strcat(rasp, user);
                                lungRasp = strlen(rasp);
                            }
                            else
                            {
                                if(strlen(param) < 3)
                                {
                                    strcpy(rasp, "Introduceți un nume de utilizator cu cel puțin 3 caractere!");
                                    lungRasp = strlen(rasp);
                                }
                                else
                                {
                                    if(!Users::Check(param))
                                    {
                                        int index = Users::insertUser(param);
                                        u[index].setThreadId(id);
                                        u[index].initializeVariables(index);
                                        u[index].setUsername(param);
                                        U = u[index];
                                    }
                                    else
                                    {
                                        for(int i = 0; i < Users::getNumberOfUsers(); i++)
                                            if(u[i].getUsername() == std::string(param))
                                            {
                                                U = u[i];
                                                break;
                                            }
                                    }
                                    strcpy(user, param);
                                    strcpy(rasp, "Utilizatorul s-a logat cu succes.");
                                    lungRasp = strlen(rasp);
                                }
                            }
                        }
                        else
                        {
                            if(cmd[0] == 's' && cmd[1] == 'e' && cmd[2] == 'a' && cmd[3] == 'r' && cmd[4] == 'c' && cmd[5] == 'h' && cmd[6] == '-')
                            {
                                char *criteriu;
                                criteriu = (char*)malloc((strlen(cmd) - strlen("search-")) * sizeof(char));
                                strcpy(criteriu, cmd + strlen("search-"));
                                if(strlen(user) < 3)
                                {
                                    strcpy(rasp, "Userul nu este logat!");
                                    lungRasp = strlen(rasp);
                                }
                                else
                                {
                                    RECOMANDARE(criteriu, param, user, rasp, lungRasp, U);
                                }
                            }
                            else
                            {
                                if(!strcmp(cmd, "download-book"))
                                {
                                    if(strlen(user) < 3)
                                    {
                                        strcpy(rasp, "Userul nu este logat!");
                                        lungRasp = strlen(rasp);
                                    }
                                    else
                                    {
                                        Descarca(param, user, rasp, lungRasp, U);
                                    }
                                }
                                else
                                {
                                    if(!strcmp(cmd, "eval-book"))
                                    {
                                        if(strlen(user) < 3)
                                        {
                                            strcpy(rasp, "Userul nu este logat!");
                                            lungRasp = strlen(rasp);
                                        }
                                        else
                                        {
                                            Evalueaza(param, user, rasp, lungRasp, U);
                                        }
                                    }
                                    else
                                    {
                                        if(!strcmp(cmd, "info-book"))
                                        {
                                            if(strlen(user) < 3)
                                            {
                                                strcpy(rasp, "Userul nu este logat!");
                                                lungRasp = strlen(rasp);
                                            }
                                            else
                                            {
                                                InfoBook(param, user, rasp, lungRasp, U);
                                            }
                                        }
                                        else
                                        {
                                            if(!strcmp(cmd, "info-author"))
                                            {
                                                if(strlen(user) < 3)
                                                {
                                                    strcpy(rasp, "Userul nu este logat!");
                                                    lungRasp = strlen(rasp);
                                                }
                                                else
                                                {
                                                    InfoAuthor(param, user, rasp, lungRasp, U);
                                                }
                                            }
                                            else
                                            {
                                                strcpy(rasp, "Comandă inexistentă!");
                                                lungRasp = strlen(rasp);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int raspuns(void *arg, char user[], Users &U)
{
    int i = 0;
    char msg[SIZE], rasp[SIZE];
	struct thData tdL;
	tdL = *((struct thData*) arg);
    int lungime;
    int totalCitit = 0;
    char aux[SIZE];
    bzero(&aux, sizeof(aux));
	if((read(tdL.cl, &lungime, sizeof(lungime))) <= 0)
	{
		printf("[Thread %d]\n",tdL.idThread);
		perror("Eroare la read() de la client ");
        return -1;
	}
    bzero(&msg, sizeof(msg));
    while(lungime > totalCitit)
    {
        int citit = 0;
        char aux[SIZE];
        bzero(&aux, sizeof(aux));
        if((citit = read(tdL.cl, msg, sizeof(msg))) <= 0)
        {
            printf("[Thread %d]\n",tdL.idThread);
            perror("Eroare la read() de la client.\n");
            return -1;
        }
        totalCitit += citit;
        strcat(msg, aux);
    }
	printf("[Thread %d]Mesajul a fost receptionat... %s\n",tdL.idThread, msg);
    msg[strlen(msg) - 1] = '\0';
    if(!strcmp(msg, "quit"))
        return -1;
    pthread_mutex_lock(&mlock);
    procesare(msg, user, rasp, lungime, tdL.idThread, U);
    pthread_mutex_unlock(&mlock);
    if(write(tdL.cl, &lungime, sizeof(lungime)) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
        return -1;
	}
    if(write(tdL.cl, rasp, lungime) <= 0)
	{
		printf("[Thread %d] ", tdL.idThread);
		perror("[Thread]Eroare la write() catre client.\n");
        return -1;
	}
	else
		printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    bzero(&rasp, sizeof(rasp));
    return 0;
}

void initializeData()
{
    Users::checkXmlFile("files/users.xml");
    Users::traverseDocument();
    for(int i = 0; i < 100; i++)
    {
        u[i].setThreadId(-1);
        u[i].initializeVariables(i);
    }
    Books::checkXmlFile("files/books.xml");
    Books::traverseDocument();
    for(int i = 0; i < Books::getNumberOfBooks(); i++)
    {
        b[i].setThreadId(-1);
        b[i].initializeVariables(i);
    }
    Books::setGenresInfo("ficțiune");
    Books::setGenresInfo("non-ficțiune");
    Books::setGenresInfo("poezie");
    Books::setGenresInfo("dramă");
    Books::setAuthors();
    for(int i = 0; i < 100; i++)
    {
        mapThToNr[i] = -1;
    }
    mapThToNr[-1] = -10;
}

static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Waiting for the message...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    char user[100];
    Users U;
    bzero(&user, sizeof(user));
    while(1)
        if(raspuns((struct thData*) arg, user, U) == -1)
        {
            close((intptr_t)arg);
            printf("Client with Thread %d *Quitted* the server...\n\n", tdL.idThread);
            return (NULL);
        }
}

int main()
{
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int on = 1;
    /* setam optiunea de a reutiliza portul */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));
    /* umplem structura folosita de server */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    /* atasam socketul */
    if(bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }
    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if(listen(sd, 5) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    initializeData();
    while(1)
    {
        int client;
        thData *td;
        socklen_t length = sizeof(from);
        printf("[server]We're waiting at the port %d...\n", PORT);
        fflush(stdout);
        /* acceptam conexiunea din partea unui client */
        if((client = accept(sd, (struct sockaddr *) &from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = thNumber++;
        td->cl = client;
        pthread_create(&th[thNumber], NULL, &treat, td);
    }
    return 0;
}