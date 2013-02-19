/*
 *@project memmgrstate
 *@file mainwindow.cpp
 *@brief Implementation of MainWindow class: a GUI and daemon-polling.
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->GetData, SIGNAL(clicked()), this, SLOT(start_polling()));
    connect(ui->Quit, SIGNAL(clicked()), this, SLOT(quit_daemon()));
    this->labels.resize(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start_polling()
{
    int s, len;
    struct sockaddr_un remote;

    s = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    std::string path_to_socket = ui->socket_path->text().toStdString();
    strcpy(remote.sun_path, path_to_socket.c_str());
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    ::connect(s, (struct sockaddr *)&remote, len);

    unsigned int* buffer = new unsigned int[4];

    //Authentication
    std::string passwd = ui->password->text().toStdString();

    buffer[0] = socket_authenticate;

    send(s, buffer, 16, 0);

    for(unsigned int i = 0; i <  passwd.length(); i++)
    {
        buffer[0] = socket_passwd_byte;
        buffer[1] = passwd[i];
        send(s, buffer, 16, 0);
    }

    buffer[0] = socket_passwd_end;
    send(s, buffer, 16, 0);

    read(s, buffer, 16);

    if(buffer[0] != socket_not_authenticated)
    {
        buffer[0] = socket_memory_map_requested;
        send(s, buffer, 16, 0);

        read(s, buffer, 16);
        if(buffer[0] == socket_memory_map_sent)
        {
            for(unsigned int i = 0; i < labels.size(); i++)
            {
                ui->scheme->removeWidget((QWidget*)&labels[i]);
                delete labels[i];
            }

            labels.clear();

            bool ok = true;
            while(ok)
            {
                read(s, buffer, 16);
                QString text = QString::number(buffer[1]);
                QLabel* q = new QLabel(text);
                labels.push_back(q);
                if(buffer[2])
                {
                    labels.last()->setStyleSheet("QLabel { background-color: white; border-style: outset;border-width: 2px; border-color: black;}");
                }
                else
                {
                    labels.last()->setStyleSheet("QLabel { background-color: blue; border-style: outset;border-width: 2px; border-color: black; }");
                }
                labels.last()->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                QSizePolicy p(QSizePolicy::Preferred,QSizePolicy::Preferred);
                p.setHorizontalStretch(buffer[1]);
                labels.last()->setSizePolicy(p);
                ok = buffer[3];
            }

            for(unsigned int i = 0; i < labels.size(); i++)
            {
                ui->scheme->addWidget((QWidget*)labels[i]);
            }
        }

        buffer[0] = socket_connection_closed;
        send(s, buffer, 16, 0);
        read(s, buffer, 16);
        if(buffer[0] == socket_connection_closed)
        {

        }
    }


    ::close(s);
}

void MainWindow::quit_daemon()
{
    int s, len;
    struct sockaddr_un remote;

    s = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, "/home/radiosterne/tmp/memmgrd_socket");
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    int a = ::connect(s, (struct sockaddr *)&remote, len);

    unsigned int* buffer = new unsigned int[4];

    //Authentication
    std::string passwd = ui->password->text().toStdString();

    buffer[0] = socket_authenticate;

    send(s, buffer, 16, 0);

    for(unsigned int i = 0; i <  passwd.length(); i++)
    {
        buffer[0] = socket_passwd_byte;
        buffer[1] = passwd[i];
        send(s, buffer, 16, 0);
    }

    buffer[0] = socket_passwd_end;
    send(s, buffer, 16, 0);

    read(s, buffer, 16);

    if(buffer[0] != socket_not_authenticated)
    {
        buffer[0] = socket_quit;
        send(s, buffer, 16, 0);

        buffer[0] = socket_connection_closed;
        send(s, buffer, 16, 0);
        read(s, buffer, 16);
        if(buffer[0] == socket_connection_closed)
        {

        }
    }


    ::close(s);
}
