#define WIN32_LEAN_AND_MEAN
#define DIESEL_CONSOLE_PORT 20249

#include <QApplication>
#include <QThread>
#include <QTimer>
#include "main.hpp"

QString INF_COLOUR("#FFFFFF");
QString ECH_COLOUR("");
QString WRN_COLOUR("#FFFF00");
QString ERR_COLOUR("#FF0000");
QString NUL_COLOUR("");

void MainWindow::SockConnected() {
  AddToCommandIn("Connected to game socket\n");
}

void MainWindow::SockDisconnected() {
  AddToCommandIn("Game socket disconnected\n");
}

void MainWindow::ReadReady() {
  QString raw_to_write = consoleSock->readAll();

  AddToCommandIn(raw_to_write);
}

void MainWindow::SocketStateChanged(QTcpSocket::SocketState state) {
  if (state == QTcpSocket::SocketState::UnconnectedState) {
    this->SetupSocketConnection();
  }
}

void MainWindow::BytesWritten(qint64 bytes) {

}

void MainWindow::AddToCommandIn(QString to_add) {
  commandIn->moveCursor(QTextCursor::End);
  commandIn->insertPlainText(to_add);
  commandIn->moveCursor(QTextCursor::End);
}

void MainWindow::SetupSocketConnection() {
  if (consoleSock == nullptr) {
    consoleSock = new QTcpSocket();

    connect(consoleSock, &QTcpSocket::connected, this, &MainWindow::SockConnected);
    connect(consoleSock, &QTcpSocket::disconnected, this, &MainWindow::SockDisconnected);
    connect(consoleSock, &QTcpSocket::bytesWritten, this, &MainWindow::BytesWritten);
    connect(consoleSock, &QTcpSocket::readyRead, this, &MainWindow::ReadReady);
    connect(consoleSock, &QTcpSocket::stateChanged, this, &MainWindow::SocketStateChanged);
  }
  
  consoleSock->connectToHost("127.0.0.1", DIESEL_CONSOLE_PORT);
  /*if (!consoleSock->waitForConnected(5000)) {
    qDebug() << "Error: " << consoleSock->errorString();
    SetupSocketConnection();
  }*/
}

void MainWindow::SubmitCommand() {
  consoleSock->write(commandEntry->text().toLocal8Bit());
  commandEntry->setText("");
}

void MainWindow::SaveLog() {
  QFile out_file(save_file_dialog->selectedFiles().first());
  out_file.open(QFile::OpenModeFlag::WriteOnly);
  QTextStream out(&out_file);
  out << this->commandIn->toPlainText();
  out_file.close();
}

void MainWindow::SaveLogDialog() {
  this->save_file_dialog->exec();
}

void MainWindow::Clear() {
  this->commandIn->setText(QString(""));
}

MainWindow::MainWindow() {
  this->resize(QSize(800, 600));

  QWidget* central = new QWidget(this);
  setCentralWidget(central);

  menu_bar = new QMenuBar(this);
  setMenuBar(menu_bar);

  this->save_file_dialog = new QFileDialog(this);
  this->save_file_dialog->setFileMode(QFileDialog::FileMode::AnyFile);
  this->save_file_dialog->setAcceptMode(QFileDialog::AcceptSave);

  connect(save_file_dialog, &QFileDialog::fileSelected, this, &MainWindow::SaveLog);

  file_menu = new QMenu(QString("File"));
  menu_bar->addMenu(file_menu);

  save_menu = file_menu->addAction(QString("Save"));

  clear_menu = file_menu->addAction(QString("Clear"));

  connect(save_menu, &QAction::triggered, this, &MainWindow::SaveLogDialog);
  connect(clear_menu, &QAction::triggered, this, &MainWindow::Clear);


  QVBoxLayout* verticalLayout = new QVBoxLayout(central);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  verticalLayout->setSpacing(0);

  commandIn = new QTextEdit();
  commandIn->setReadOnly(true);

  commandEntry = new QLineEdit();

  connect(commandEntry, &QLineEdit::returnPressed, this, &MainWindow::SubmitCommand);

  verticalLayout->addWidget(commandIn);
  verticalLayout->addWidget(commandEntry);


  this->setStyleSheet(QString(
    "* {"
    "color: #eef3ef"
    "}"
    "QMainWindow, QTextEdit, QMenuBar {"
    "background-color: #1a1c19;"
    "accent-color: #1a1c19;"
    "outline: #1a1c19;"
    "border: #5a5c59;"
    "}"
    "QTextEdit {"
    "background-color: #4a4c49;"
    "}"
    "QLineEdit {"
    "background-color: #2a2c29;"
    "}"
  ));

  this->SetupSocketConnection();
}

MainWindow::~MainWindow() {}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}