/*
 *@project memmgrstate
 *@file mainwindow.h
 *@brief Definition of MainWindow class: a GUI and daemon-polling.
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <pthread.h>
#include <QLabel>


namespace Ui {
class MainWindow;
}

/*
 *@brief: Main program window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    /*
     *@brief Window UI
     */
    Ui::MainWindow *ui;
    /*
     *@brief An array of labels depicting memory state
     */
    QVector<QLabel*> labels;
private slots:
    /*
     *@brief Polls daemon for new data.
     */
    void start_polling();
    /*
     *@brief Sends daemon a quit request.
     */
    void quit_daemon();
};

//Message codes (copied from main project)
enum socket_message_codes
{
    //Normally daemon-recievable codes
    socket_connection_closed,
    socket_byte_sent,
    socket_byte_requested,
    socket_memory_requested,
    socket_memory_termination,
    socket_authenticate,
    socket_quit,
    socket_memory_map_requested,
    socket_passwd_byte,
    socket_passwd_end,

    //Normally daemon-sendable codes
    socket_memory_allocated,
    socket_memory_map_sent,
    socket_not_authenticated,
    socket_authenticated,
    socket_node_descriptor,
    socket_error
};

#endif // MAINWINDOW_H
