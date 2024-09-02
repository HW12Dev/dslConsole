#pragma once

#include <QMainWindow>

#include <QTcpSocket>
#include <QTextEdit>
#include <QMenuBar>
#include <QLineEdit>
#include <QFileDialog>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
public:
  MainWindow();
  ~MainWindow();

  void AddToCommandIn(QString to_add);

  void SetupSocketConnection();

  void SockConnected();
  void SockDisconnected();
  void BytesWritten(qint64 bytes);
  void ReadReady();
  void SocketStateChanged(QTcpSocket::SocketState state);

  void SubmitCommand();

  void SaveLog();
  void SaveLogDialog();
  void Clear();

  QTextEdit *commandIn;
  QLineEdit *commandEntry;

  QTcpSocket* consoleSock = nullptr;
  QThread* socket_connection_thread;

  QFileDialog* save_file_dialog;

  QMenuBar* menu_bar;
  QMenu* file_menu;
    QAction* save_menu;
    QAction* clear_menu;
};