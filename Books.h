#pragma once
#include "files/pugixml.hpp"
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <vector>
#include <cstdio>
#include <ctime>
#include <set>
#include <map>
float strtoi(std::string s)
{
    float nr = 0;
    if(s == "-1")
        return -1;
    if(s[1] == '.')
    {
        nr = (s[0] - '0') * 10 + (s[2] - '0');
        nr = (1.0 * nr) / 10;
        return nr;
    }
    if(s.length() == 2)
        return 10;
    return float(s[0] - '0');
}
std::string itostr(float nr)
{
    char s[10];
    if(float(int(nr)) < nr)
    {
        int n = 0, i = 0;
        while(float(int(nr)) != nr)
        {
            nr = nr * 10;
            n++;
        }
        int number = nr;
        while(number)
        {
            if(i == n)
            {
                s[i]='.';
                i++;
            }
            s[i] = char(number % 10 + '0');
            number = number / 10;
            i++;
        }
        s[i] = '\0';
    }
    else
    {
        int number = nr;
        int i = 0;
        while(number)
        {
            s[i] = char(number % 10 + '0');
            number = number / 10;
            i++;
        }
        s[i] = '\0';
    }
    std::string sir(s);
    reverse(sir.begin(), sir.end());
    return sir;
}
class Books {
private:
    std::string bookInfo;
    std::string title;
    std::string author;
    std::string year;
    std::string genre;
    std::string allSubgenres;
    std::vector <std::string> subgenres;
    std::string ISBN;
    float rating;
    std::string ratingString;
    std::string nrVotes;
    int nrV;
    struct autor {
    std::string nume;
    std::vector<std::string> genuri;
    std::vector<std::string> subgenuri;
    };
    static autor a[42];
    std::string URL;
    static int xmlCheck;
    static pugi::xml_document doc;
    static std::map<int, pugi::xml_node> bookMapIdToNodes;
    static const int numberOfBooks = 58;
    unsigned bookIdInList;
    int threadId = -1;
public:
    static std::vector<std::string> fiction, nonfiction, drama, poetry;
    static void setGenresInfo(const char* nume)
    {
        std::string name(nume);
        auto gen = doc.child("XmlLibs").child("Lib").child("Genuri").children();
        if(name == "ficțiune")
        {
            for(auto obj : gen)
            {
                if(obj.attribute("nume").as_string() == name)
                {
                    auto param = obj.children();
                    for(auto node : param)
                        fiction.push_back(node.attribute("nume").as_string());
                    break;
                }
            }
        }
        else
            if(name == "non-ficțiune")
            {
                for(auto obj : gen)
                    if(obj.attribute("nume").as_string() == name)
                    {
                        auto param = obj.children();
                        for(auto node : param)
                            nonfiction.push_back(node.attribute("nume").as_string());
                        break;
                    }
            }
            else
                if(name == "dramă")
                {
                    for(auto obj : gen)
                        if(obj.attribute("nume").as_string() == name)
                        {
                            auto param = obj.children();
                            for(auto node : param)
                                drama.push_back(node.attribute("nume").as_string());
                            break;
                        }
                }
                else
                {
                    for(auto obj : gen)
                        if(obj.attribute("nume").as_string() == name)
                        {
                            auto param = obj.children();
                            for(auto node : param)
                                poetry.push_back(node.attribute("nume").as_string());
                            break;
                        }
                }
    }
    static void printGenresAll(char rasp[], int &lungRasp)
    {
        strcpy(rasp, "\nFicțiune:\n");
        for(auto vec : fiction)
        {
            strcat(rasp, "    ");
            strcat(rasp, vec.c_str());
            strcat(rasp, "\n");
        }
        strcat(rasp, "Non-ficțiune:\n");
        for(auto vec : nonfiction)
        {
            strcat(rasp, "    ");
            strcat(rasp, vec.c_str());
            strcat(rasp, "\n");
        }
        strcat(rasp, "Poezie:\n");
        for(auto vec : poetry)
        {
            strcat(rasp, "    ");
            strcat(rasp, vec.c_str());
            strcat(rasp, "\n");
        }
        strcat(rasp, "Dramă:");
        for(auto vec : drama)
        {
            strcat(rasp, "\n");
            strcat(rasp, "    ");
            strcat(rasp, vec.c_str());
        }
        lungRasp = strlen(rasp);
    }
    static void checkXmlFile(const char *source)
    {
        pugi::xml_parse_result result = doc.load_file(source);
        if(result)
        {
            std::cout << "XML [" << source << "] parsed successfully!\nThe file is ready for use.\n\n";
            xmlCheck = 1;
        }
        else
        {
            std::cout << result.description();
            std::cout << "XML [" << source << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
            std::cout << "Error description: " << result.description() << "\n";
            std::cout << "Error offset: " << result.offset << " (error at [..." << (source + result.offset) << "]\n\n";
        }
        auto carti = doc.child("XmlLibs").child("Lib").child("Carti").children();
    }
    static void traverseDocument()
    {
        int i = 0;
        auto book = doc.child("XmlLibs").child("Lib").child("Carti").children();
        for(auto &obj : book)
        {
            bookMapIdToNodes[i++] = obj;
        }
    }
    static int getNumberOfBooks()
    {
        return numberOfBooks;
    }
    void setThreadId(int id)
    {
        threadId = id;
    }
    int setBookIdInList(unsigned id)
    {
        if(id <= numberOfBooks)
        {
            this->bookIdInList = id;
            return id;
        }
        return -1;
    }
    void insertEval(int index, char nota[])
    {
        auto book = bookMapIdToNodes[index];
        nrV++;
        nrVotes = itostr(nrV);
        if(rating == -1)
        {
            ratingString = nota;
            rating = strtoi(ratingString);
            nrVotes = itostr(nrV); 
        }
        else
        {
            rating = 1.0 * rating * (nrV - 1) + strtoi(nota);
            rating = float(1.0 * rating / nrV);
            ratingString = itostr(rating);
        }
        book.attribute("rating").set_value(ratingString.c_str());
        book.attribute("NrVoturi").set_value(nrVotes.c_str());
        doc.save_file("files/books.xml");
    }
    void bookInfos()
    {
        bookInfo = this->title + ", de " + this->author + ", anul apariției " + year + ", genul " + genre + ", subgenuri " + allSubgenres + ", ISBN " + ISBN + ", rating " + ratingString + ".";
    }
    std::string getBookInfos()
    {
        return this->bookInfo;
    }
    std::string getTitleOfBook()
    {
        return this->title;
    }
    float getRating()
    {
        return this->rating;
    }
    std::string getAuthorOfBook()
    {
        return this->author;
    }
    std::string getYearOfBook()
    {
        return this->year;
    }
    std::string getRatingOfBook()
    {
        return this->ratingString;
    }
    std::string getISBNOfBook()
    {
        return this->ISBN;
    }
    std::string getGenreOfBook()
    {
        return this->genre;
    }
    std::string getAllSubgenresOfBook()
    {
        return this->allSubgenres;
    }
    std::string getURLOfBook()
    {
        return this->URL;
    }
    void setBookInfos(int index)
    {
        auto book = bookMapIdToNodes[index];
        title = book.attribute("Titlu").as_string();
        author = book.attribute("Autor").as_string();
        year = book.attribute("AnulAparitiei").as_string();
        ISBN = book.attribute("ISBN").as_string();
        ratingString = book.attribute("rating").as_string();
        rating = strtoi(ratingString);
        nrVotes = book.attribute("NrVoturi").as_string();
        nrV = int(strtoi(nrVotes));
        URL = book.attribute("URL").as_string();
        genre = book.attribute("gen").as_string();
        allSubgenres = book.attribute("subgenuri").as_string();
        char c[200];
        strcpy(c, allSubgenres.c_str());
        char *p = strtok(c, ",");
        while(p != NULL)
        {
            subgenres.push_back(std::string(p));
            p = strtok(NULL, ",");
        }
        bookInfos();
    }
    void initializeVariables(int id)
    {
        setBookIdInList(id);
        setBookInfos(id);
    }
    static void setAuthors()
    {
        int i = 0;
        auto autor = doc.child("XmlLibs").child("Lib").child("Autori").children();     
        for(auto obj : autor)
        {
            a[i].nume = obj.attribute("Nume").as_string();
            std::string aux;
            aux = obj.attribute("gen").as_string();
            char c[500];
            strcpy(c, aux.c_str());
            char *p = strtok(c, ",");
            while(p != NULL)
            {
                a[i].genuri.push_back(std::string(p));
                p = strtok(NULL, ",");
            }
            aux = obj.attribute("subgenuri").as_string();
            strcpy(c, aux.c_str());
            char *q = strtok(c, ",");
            while(q != NULL)
            {
                a[i].subgenuri.push_back(std::string(q));
                q = strtok(NULL, ",");
            }
            i++;
        }
    }

    static bool getAuthorInfo(std::string Autor, char rasp[], int &lungRasp)
    {
        bool ok = false;
        for(int i = 0; i < 42 && !ok; i++)
        {
            printf("%s\n", a[i].nume.c_str());
            if(a[i].nume == Autor)
            {
                printf("OK\n");
                strcpy(rasp, "Genurile și subgenurile abordate de autor sunt:\n");
                for(auto gen : a[i].genuri)
                {
                    printf("Am ajuns aici.\n");
                    printf("Autor este: %s\n", Autor.c_str());
                    printf("Gen este: %s\n\n", gen.c_str());
                    strcat(rasp, gen.c_str());
                    strcat(rasp, ":\n");
                    if(gen == "ficțiune")
                    {
                        printf("Am ajuns ȘI aici.\n\n");
                        for(auto subgen : fiction)
                            for(auto subgen_autor : a[i].subgenuri)
                                if(subgen == subgen_autor)
                                {
                                    strcat(rasp, "    ");
                                    strcat(rasp, subgen.c_str());
                                    strcat(rasp, "\n");
                                    break;
                                }
                    }
                    else
                    {
                        if(gen == "non-ficțiune")
                        {
                            for(auto subgen : nonfiction)
                                for(auto subgen_autor : a[i].subgenuri)
                                    if(subgen == subgen_autor)
                                    {
                                        strcat(rasp, "    ");
                                        strcat(rasp, subgen.c_str());
                                        strcat(rasp, "\n");
                                        break;
                                    }
                        }
                        else
                        {
                            if(gen == "poezie")
                            {
                                for(auto subgen : poetry)
                                    for(auto subgen_autor : a[i].subgenuri)
                                        if(subgen == subgen_autor)
                                        {
                                            strcat(rasp, "    ");
                                            strcat(rasp, subgen.c_str());
                                            strcat(rasp, "\n");
                                            break;
                                        }
                            }
                            else
                            {
                                for(auto subgen : drama)
                                    for(auto subgen_autor : a[i].subgenuri)
                                        if(subgen == subgen_autor)
                                        {
                                            strcat(rasp, "    ");
                                            strcat(rasp, subgen.c_str());
                                            strcat(rasp, "\n");
                                            break;
                                        }
                            }
                        }
                    }
                }
                ok = true;
            }
        }
        lungRasp = strlen(rasp);
        rasp[lungRasp - 1] = '\0';
        lungRasp--;
        return ok;
    }
};
std::map<int, pugi::xml_node> Books::bookMapIdToNodes;
pugi::xml_document Books::doc;
int Books::xmlCheck = 1;
Books::autor Books::a[42];
std::vector<std::string> Books::fiction, Books::nonfiction, Books::drama, Books::poetry;