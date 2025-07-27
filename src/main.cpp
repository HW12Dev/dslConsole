#define WIN32_LEAN_AND_MEAN
#define DIESEL_CONSOLE_PORT 20249

// silly testing
//#define BITSQUID_CONSOLE_PORT 27036

#include <QApplication>
#include <QThread>
#include <QTimer>
#include "main.hpp"

QString INF_COLOUR("#FFFFFF");
QString ECH_COLOUR("");
QString WRN_COLOUR("#FFFF00");
QString ERR_COLOUR("#FF0000");
QString NUL_COLOUR("");


void write_u32(QByteArray& arr, uint32_t val)
{
  arr.append((char*)&val, sizeof(val));
}
uint32_t read_u32(const QByteArray& arr, size_t& offset)
{
  uint32_t val = _byteswap_ulong(*(uint32_t*)(arr.data() + offset));
  offset += sizeof(val);
  return val;
}
std::string read_string(const QByteArray& arr, size_t& offset)
{
  auto size = read_u32(arr, offset);

  if (size > arr.length() || size > 10000) // really bad hack
    return "Malformed message";

  std::string str = std::string(arr.data() + offset, arr.data() + offset + size);
  offset += size;
  return str;
}


void MainWindow::SockConnected() {
  AddToCommandIn("Connected to game socket\n");
}

void MainWindow::SockDisconnected() {
  AddToCommandIn("Game socket disconnected\n");
}

void MainWindow::ReadReady() {
  QByteArray raw_to_write = consoleSock->readAll();

  size_t offset = 0;

  try {

    while (offset < raw_to_write.length() - 1) {

      offset += 8; // packet size?

      std::string log_level = read_string(raw_to_write, offset);
      std::string message = read_string(raw_to_write, offset);

      AddToCommandIn(QString("[") + log_level.c_str() + "] " + message.c_str());
    }
  }
  catch (const char* e) {
    AddToCommandIn(raw_to_write);
  }
}

void MainWindow::SocketStateChanged(SocketType::SocketState state) {
  if (state == SocketType::SocketState::UnconnectedState) {
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
    consoleSock = new SocketType();
    const uint64_t receiveBufferSize = 65536 * 4;
    consoleSock->setReadBufferSize(receiveBufferSize);
    consoleSock->setSocketOption(SocketType::ReceiveBufferSizeSocketOption, receiveBufferSize);

    connect(consoleSock, &SocketType::connected, this, &MainWindow::SockConnected);
    connect(consoleSock, &SocketType::disconnected, this, &MainWindow::SockDisconnected);
    connect(consoleSock, &SocketType::bytesWritten, this, &MainWindow::BytesWritten);
    connect(consoleSock, &SocketType::readyRead, this, &MainWindow::ReadReady);
    connect(consoleSock, &SocketType::stateChanged, this, &MainWindow::SocketStateChanged);
  }
  
  consoleSock->connectToHost("127.0.0.1", DIESEL_CONSOLE_PORT);
  /*if (!consoleSock->waitForConnected(5000)) {
    qDebug() << "Error: " << consoleSock->errorString();
    SetupSocketConnection();
  }*/
}

// dsl::ConsoleProtocol::MESSAGE_TYPE
enum ChunkMessageType : uint32_t {
  TEXT = 0,
  EXECUTE_COMMAND = 1,
  NULL_MESSAGE = 2,
  HASH_PAIR = 3,
  UNFINISHED_MESSAGE = 4
};

// dsl::ConsoleServer::handle_chunk
// dsl::ConsoleIOPort::nibble_chunk

void MainWindow::SubmitCommand() {
  QByteArray cmd;

  auto text = commandEntry->text().toLocal8Bit();

  write_u32(cmd, _byteswap_ulong(EXECUTE_COMMAND)); // chunk type
  write_u32(cmd, _byteswap_ulong(text.length() + sizeof(uint32_t))); // bytes expected: text length + u32 that stores it's actual size
  write_u32(cmd, _byteswap_ulong(text.length()));

  cmd.append(text);


  consoleSock->write(cmd);

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


  /*this->setStyleSheet(QString(
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
  ));*/

  this->SetupSocketConnection();
}

MainWindow::~MainWindow() {}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}