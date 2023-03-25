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
#include "Books.h"
class Users {
private:    
    std::string username;
    std::vector<std::string> cautari;
    static pugi::xml_document doc;
    static std::map<int, pugi::xml_node> userMapIdToNodes;
    int threadId = -1;
    unsigned userInList;
    static int xmlCheck;
public:
    void setUsername(char param[])
    {
        this->username = std::string(param);
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
        auto users = doc.child("XmlUsers").child("UsersDb").child("Users").children();
    }
    static void traverseDocument()
    {
        int i = 0;
        auto user = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto &obj : user)
            userMapIdToNodes[i++] = obj;
        i = 0;
    }
    static bool Check(char *username)
    {
        std::string nume(username);
        auto user = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto &obj : user)
        {
            if(obj.attribute("Username").as_string() == nume)
                return true;
        }
        return false;
    }
    static int insertUser(char *username)
    {
        auto node = doc.child("XmlUsers").child("UsersDb").child("Users");
        pugi::xml_node param = node.append_child("User");
        param.append_attribute("Username") = username;
        doc.save_file("files/users.xml");
        return getNumberOfUsers() - 1;
    }
    void insertDownload(const char *titlu, const char *autor, const char *gen, const char *subgen, const char *an)
    {
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == this->username)
            {
                bool ok = true;
                auto downloads = user.children();
                for(auto nod : downloads)
                {
                    if(nod.name() == std::string("Download") && nod.attribute("Titlu").value() == std::string(titlu) && nod.attribute("Autor").value() == std::string(autor))
                    {
                        ok = false;
                        break;
                    }
                }
                if(ok)
                {
                    pugi::xml_node param = user.append_child("Download");
                    param.append_attribute("Titlu") = titlu;
                    param.append_attribute("Autor") = autor;
                    param.append_attribute("Gen") = gen;
                    param.append_attribute("Subgen") = subgen;
                    param.append_attribute("An") = an;
                    doc.save_file("files/users.xml");
                }
                break;
            }
        }
    }
    void insertEval(const char *titlu, const char *autor, char nota[], const char *gen, const char *subgen, const char *an)
    {
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == this->username)
            {
                bool ok = true;
                auto eval = user.children();
                for(auto nod : eval)
                {
                    if(nod.name() == std::string("Book") && nod.attribute("Titlu").value() == std::string(titlu) && nod.attribute("Autor").value() == std::string(autor))
                    {
                        nod.attribute("nota").set_value(nota);
                        doc.save_file("files/users.xml");
                        ok = false;
                        break;
                    }
                }
                if(ok)
                {
                    pugi::xml_node param = user.append_child("Book");
                    param.append_attribute("Titlu") = titlu;
                    param.append_attribute("Autor") = autor;
                    param.append_attribute("Gen") = gen;
                    param.append_attribute("Subgen") = subgen;
                    param.append_attribute("An") = an;
                    param.append_attribute("nota") = nota;
                    doc.save_file("files/users.xml");
                }
                break;
            }
        }
    }
    void insertCautare(const char *criteriu, char *parametru)
    {
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == this->username)
            {
                bool ok = true;
                auto caut = user.children();
                for(auto nod : caut)
                {
                    if(nod.name() == std::string("Cautare") && nod.attribute(criteriu).as_string() == std::string(parametru))
                    {
                        ok = false;
                        break;
                    }
                }
                if(ok)
                {
                    pugi::xml_node param = user.append_child("Cautare");
                    param.append_attribute(criteriu) = parametru;
                    doc.save_file("files/users.xml");
                }
                break;
            }
        }
    }
    static int getNumberOfUsers()
    {
        auto user = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        int numberOfUsers = 0;
        for(auto &obj : user)
        {
            numberOfUsers++;
        }
        return numberOfUsers;
    }
    void setThreadId(int id)
    {
        threadId = id;
    }
    void setUserInfos(int index)
    {
        auto user = userMapIdToNodes[index];
        this->username = user.attribute("Username").as_string();
    }
    int setUserIdInList(unsigned id)
    {
        if(id <= getNumberOfUsers())
        {
            this->userInList = id;
            return id;
        }
        return -1;
    }
    std::string getUsername()
    {
        return this->username;
    }
    void setBooksInList(int id)
    {
        auto user = userMapIdToNodes[id].children();
    }
    void initializeVariables(int id)
    {
        setUserIdInList(id);
        setBooksInList(id);
        setUserInfos(id);
    }
    std::string getFavoriteSubgenre()
    {
        std::map<std::string, int> subgenuri;
        std::string pref;
        std::vector<std::string> subgenuriCautate;
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == this->username)
            {
                auto caut = user.children();
                for(auto nod : caut)
                {
                    if(nod.name() == std::string("Cautare"))
                    {
                        {
                            for(auto subgen : Books::fiction)
                            {
                                std::string s1 = nod.attribute("subgenre").as_string();
                                std::size_t found = s1.find(subgen);
                                if(found != std::string::npos)
                                {
                                    if(subgenuri[subgen] == 0)
                                        subgenuriCautate.push_back(subgen);
                                    subgenuri[subgen]++;
                                }
                            }
                        }
                        {
                            for(auto subgen : Books::nonfiction)
                            {
                                std::string s1 = nod.attribute("subgenre").as_string();
                                std::size_t found = s1.find(subgen);
                                if(found != std::string::npos)
                                {
                                    if(subgenuri[subgen] == 0)
                                        subgenuriCautate.push_back(subgen);
                                    subgenuri[subgen]++;
                                }
                            }
                        }
                        {
                            for(auto subgen : Books::poetry)
                            {
                                std::string s1 = nod.attribute("subgenre").as_string();
                                std::size_t found = s1.find(subgen);
                                if(found != std::string::npos)
                                {
                                    if(subgenuri[subgen] == 0)
                                        subgenuriCautate.push_back(subgen);
                                    subgenuri[subgen]++;
                                }
                            }
                        }
                        {
                            for(auto subgen : Books::drama)
                            {
                                std::string s1 = nod.attribute("subgenre").as_string();
                                std::size_t found = s1.find(subgen);
                                if(found != std::string::npos)
                                {
                                    if(subgenuri[subgen] == 0)
                                        subgenuriCautate.push_back(subgen);
                                    subgenuri[subgen]++;
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
        int max = 0, nrApMax = 0;
        for(auto sub : subgenuriCautate)
            if(max < subgenuri[sub])
            {
                max = subgenuri[sub];
                pref = sub;
                nrApMax = 1;
            }
            else
                if(max == subgenuri[sub])
                    nrApMax++;
        if(max == 0 || nrApMax > 1)
            return "niciunul";
        return pref;
    }
    std::string getFavoriteAuthor()
    {
        std::map<std::string, int> autor;
        std::string pref;
        std::vector<std::string> autoriCautati;
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == this->username)
            {
                auto caut = user.children();
                for(auto nod : caut)
                {
                    if(nod.name() == std::string("Cautare"))
                    {
                        if(autor[nod.attribute("author").as_string()] == 0)
                            autoriCautati.push_back(nod.attribute("author").as_string());
                        autor[nod.attribute("author").as_string()]++;
                    }
                }
                break;
            }
        }
        int max = 0, nrApMax = 0;
        for(auto aut : autoriCautati)
            if(max < autor[aut])
            {
                max = autor[aut];
                pref = aut;
                nrApMax = 1;
            }
            else
                if(max == autor[aut])
                    nrApMax++;
        if(max == 0 || nrApMax > 1)
            return "niciunul";
        return pref;
    }
    void Determine(std::string nume_user, std::string &genul, std::string &subgenul, std::string &autorul, std::string &anul)
    {
        std::map<std::string, int> top_autor;
        std::map<std::string, int> top_gen;
        std::map<std::string, int> top_subgen;
        std::map<std::string, int> top_an;
        auto nodes = doc.child("XmlUsers").child("UsersDb").child("Users").children();
        for(auto user : nodes)
        {
            pugi::xml_attribute attr = user.attribute("Username");
            if(attr.value() == nume_user)
            {
                auto caut = user.children();
                for(auto nod : caut)
                {
                    if(nod.name() == std::string("Cautare"))
                    {
                        top_autor[nod.attribute("author").as_string()]++;
                        top_gen[nod.attribute("genre").as_string()]++;
                        top_subgen[nod.attribute("subgenre").as_string()]++;
                        top_an[nod.attribute("year").as_string()]++;
                    }
                    else
                        if(nod.name() == std::string("Book") || nod.name() == std::string("Download"))
                        {
                            top_autor[nod.attribute("Autor").as_string()]++;
                            top_gen[nod.attribute("Gen").as_string()]++;
                            top_an[nod.attribute("An").as_string()]++;
                            char c[500];
                            std::string aux = nod.attribute("Subgen").as_string();
                            strcpy(c, aux.c_str());
                            char *p = strtok(c, ",");
                            while(p != NULL)
                            {
                                top_subgen[std::string(p)]++;
                                p = strtok(NULL, ",");
                            }
                        }
                }
                break;
            }
        }
        for(auto i : top_autor)
        {
            if(i.second > top_autor[autorul])
                autorul = i.first;
        }
        for(auto i : top_gen)
        {
            if(i.second > top_gen[genul])
                genul = i.first;
        }
        for(auto i : top_subgen)
        {
            if(i.second > top_subgen[subgenul])
                subgenul = i.first;
        }
        for(auto i : top_an)
        {
            if(i.second > top_an[anul])
                anul = i.first;
        }
    }

    std::vector<Books> getBooksOfAnUserWithSimilarTaste(Books b[])
    {
        std::vector<Books> vector_final;
        std::map<std::string, bool> mm;
        std::string genul, subgenul, autorul, anul;
        Determine(this->username, genul, subgenul, autorul, anul);
        bool gasitAlt = false, gasitUser = false;
        for(int i = 0; i < getNumberOfUsers(); i++)
        {
            if(userMapIdToNodes[i].attribute("Username").as_string() != this->username)
            {
                std::string g, sb, aut, an;
                Determine(userMapIdToNodes[i].attribute("Username").as_string(), g, sb, aut, an);
                if(autorul == aut && genul == g && subgenul == sb || subgenul == sb && anul == an)
                {
                    auto downloads = userMapIdToNodes[i].children();
                    for(auto x : downloads)
                    {
                        if(x.name() == std::string("Download") || (x.name() == std::string("Book") && x.attribute("nota").as_float() >= 7.5))
                        {
                            std::string title = x.attribute("Titlu").as_string();
                            std::string autor = x.attribute("Autor").as_string();
                            for(int k = 0; k < Books::getNumberOfBooks(); k++)
                            {
                                if(b[k].getAuthorOfBook() == autor && b[k].getTitleOfBook() == title && mm[b[k].getISBNOfBook()] != true)
                                {
                                    mm[b[k].getISBNOfBook()] = true;
                                    vector_final.push_back(b[k]);
                                }
                            }
                        }
                    }
                    break;
                }
            }
        } 
        return vector_final;
    }
};

std::map<int, pugi::xml_node> Users::userMapIdToNodes; 
pugi::xml_document Users::doc;
int Users::xmlCheck = 1;