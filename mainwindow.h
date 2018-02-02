#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openDirectory();
    void rename();

private slots:
    void oldEdited();
    void newEdited();

private:
    Ui::MainWindow *ui;

    QLabel *oldLabel;
    QLabel *newLabel;
    QLabel *spaceLabel;
    QLineEdit *oldText;
    QLineEdit *newText;

    QString workDirectory;

    void processDir(const QString &filePath);
    void insertText(const QString &text);
};

#endif // MAINWINDOW_H
