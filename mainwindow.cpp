#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QDirIterator>
#include <QStringListIterator>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move(100, 100);

    this->setMinimumWidth(350);
    oldLabel = new QLabel("Replace", this);
    oldLabel->setMargin(10);
    ui->mainToolBar->addWidget(oldLabel);

    oldText = new QLineEdit(this);
    oldText->setMinimumWidth(30);
    ui->mainToolBar->addWidget(oldText);

    newLabel = new QLabel("with", this);
    newLabel->setMargin(10);
    ui->mainToolBar->addWidget(newLabel);

    newText = new QLineEdit(this);
    newText->setMinimumWidth(30);
    ui->mainToolBar->addWidget(newText);

    spaceLabel = new QLabel("  ", this);
    ui->mainToolBar->addWidget(spaceLabel);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openDirectory()));
    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(rename()));
    connect(oldText, SIGNAL(editingFinished()), this, SLOT(oldEdited()));
    connect(newText, SIGNAL(editingFinished()), this, SLOT(newEdited()));
    connect(oldText, SIGNAL(returnPressed()), newText, SLOT(setFocus()));
}

MainWindow::~MainWindow()
{
    delete oldLabel;
    delete newLabel;
    delete oldText;
    delete newText;
    delete spaceLabel;
    delete ui;
}

void MainWindow::openDirectory()
{
    workDirectory = QFileDialog::getExistingDirectory(this, "Select directory...", QDir::homePath());

    QStringList title = this->windowTitle().split(QString(" - "));
    this->setWindowTitle(title.at(0) + " - " + workDirectory);

    oldEdited();
}

void MainWindow::rename()
{
    if(newText->text().isEmpty()) {
        if(QMessageBox::question(this, QApplication::applicationName(),
                "Replace with empty text?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }

    QDir::setCurrent(workDirectory);

    QDirIterator iterator(workDirectory, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QStringList entries;

    while(iterator.hasNext()) {
        entries << iterator.next();
        qDebug() << iterator.filePath();
    }

    entries.sort();

    QStringListIterator li(entries);
    li.toBack();
    while(li.hasPrevious()) {
        processDir(li.previous());
    }

    // Top-Ordner bearbeiten
    processDir(workDirectory);
}

void MainWindow::processDir(const QString &filePath)
{
    insertText(QString("\nProcessing %1/ ...\n\n").arg(filePath));

    QProcess moveProcess;
    moveProcess.setWorkingDirectory(filePath);
    moveProcess.start("bash");
    moveProcess.write(QString("for i in *%1*;").arg(oldText->text()).toLatin1());
    moveProcess.write(QString("do mv \"$i\" \"%1${i#%2}\";").arg(newText->text()).arg(oldText->text()).toLatin1());
    moveProcess.write("done");
    moveProcess.closeWriteChannel();
    moveProcess.waitForFinished();
    moveProcess.close();

    QDir dir(filePath);
    QStringList fileList = dir.entryList();

    insertText(QString("\nEntries in %1:\n\n").arg(filePath));

    foreach (QString str, fileList) {
        insertText(str + "\n");
    }
}

void MainWindow::insertText(const QString &text)
{
    ui->textEdit->insertPlainText(text);
    ui->textEdit->ensureCursorVisible();
    qDebug() << text;
}

void MainWindow::oldEdited()
{
    if(!workDirectory.isEmpty() && !oldText->text().isEmpty()) {
        QProcess find;
        find.setWorkingDirectory(workDirectory);
        find.start(QString("find . -name \"*%1*\"").arg(oldText->text()));
        find.waitForFinished();
        QByteArray entries = find.readAll();
        find.close();

        QStringList list = QString::fromLatin1(entries).split("\n");
        if(!list.isEmpty() && entries.size() > 3) {

            insertText(QString("Files and directories containing %1 in %2:\n\n")
                    .arg(oldText->text()).arg(workDirectory));

            QStringListIterator li(list);
            while(li.hasNext()) {
                insertText(li.next() + "\n");
            }

            newEdited();

        } else {

            insertText("...Didn't find any file\n\n");
            ui->actionRun->setEnabled(false);
        }

    } else {
        ui->actionRun->setEnabled(false);
    }
}

void MainWindow::newEdited()
{
    if(!newText->text().isEmpty()) {

        if(!workDirectory.isEmpty() && !oldText->text().isEmpty()) {

            ui->actionRun->setEnabled(true);
            insertText("\nPress Cmd+R or click the running man\n");
        }
    }

}
