// ========================================
// 完整版自动更新器 - 修复函数声明歧义
// ========================================

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDir>
#include <QProcess>
#include <QTime>
#include <QEventLoop>
#include <QDebug>

// ========================================
// 更新 Worker
// ========================================
class UpdateWorker : public QObject {
    Q_OBJECT
signals:
    void progressUpdated(int value);
    void statusUpdated(QString message);
    void noUpdateNeeded();
    void finished();
    void errorOccurred(QString message);

public:
    void doUpdate(const QString &versionUrl) {
        // ---- 获取版本信息 ----
        QNetworkAccessManager manager;
        QNetworkReply *versionReply = manager.get(QNetworkRequest(QUrl(versionUrl)));

        QEventLoop loop;
        connect(versionReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (versionReply->error() != QNetworkReply::NoError) {
            emit errorOccurred("获取版本信息失败：" + versionReply->errorString());
            versionReply->deleteLater();
            return;
        }

        QByteArray data = versionReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString serverVersion = obj["version"].toString();
        QString downloadUrl = obj["download_url"].toString();
        QString expectedMd5 = obj["md5"].toString();

        versionReply->deleteLater();

        QString localVersion = "1.0.0";

        if (serverVersion == localVersion) {
            emit noUpdateNeeded();
            return;
        }

        emit statusUpdated("发现新版本 " + serverVersion + "，开始下载...");

        // ---- 断点续传下载 ----
        QString tempFile = QCoreApplication::applicationDirPath() + "/update_temp.dat";
        QFile file(tempFile);
        qDebug() << "📁 文件保存路径：" << tempFile;  // ← 加这行

        qint64 existingSize = 0;
        if (file.exists()) {
            existingSize = file.size();
            emit statusUpdated(QString("继续下载（已下载 %1 MB）...").arg(existingSize / 1024.0 / 1024.0, 0, 'f', 2));
        } else {
            file.open(QIODevice::WriteOnly);
            file.close();
        }

        QNetworkRequest request{QUrl(downloadUrl)};
        if (existingSize > 0) {
            QString rangeHeader = QString("bytes=%1-").arg(existingSize);
            request.setRawHeader("Range", rangeHeader.toUtf8());
        }

        QNetworkReply *downloadReply = manager.get(request);
        file.open(QIODevice::Append);

        // ---- 添加事件循环 ----
        QEventLoop downloadLoop;

        connect(downloadReply, &QNetworkReply::downloadProgress, [&](qint64 received, qint64 total) {
            qDebug() << "下载进度：" << received << "/" << total;
            if (total > 0) {
                qint64 totalWithResume = total + existingSize;
                qint64 receivedWithResume = received + existingSize;
                int percent = (int)((receivedWithResume * 100) / totalWithResume);
                emit progressUpdated(percent);
            }
        });

        connect(downloadReply, &QNetworkReply::readyRead, [&]() {
            qDebug() << "收到数据：" << downloadReply->bytesAvailable() << "字节";
            file.write(downloadReply->readAll());
        });

        connect(downloadReply, &QNetworkReply::finished, &downloadLoop, &QEventLoop::quit);
        connect(downloadReply, &QNetworkReply::errorOccurred, &downloadLoop, &QEventLoop::quit);

        downloadLoop.exec();  // 这里会阻塞，直到下载完成
        // -----------------------------

        file.close();

        if (downloadReply->error() != QNetworkReply::NoError) {
            emit errorOccurred("下载失败：" + downloadReply->errorString());
            downloadReply->deleteLater();
            return;
        }

        emit statusUpdated("下载完成，正在校验文件...");

        // ---- MD5 校验 ----
        QFile checkFile(tempFile);
        if (!checkFile.open(QIODevice::ReadOnly)) {
            emit errorOccurred("无法打开下载文件进行校验");
            downloadReply->deleteLater();
            return;
        }

        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(&checkFile);
        checkFile.close();

        QString actualMd5 = hash.result().toHex();

        if (actualMd5 != expectedMd5) {
            emit errorOccurred("文件校验失败！\n期望 MD5: " + expectedMd5 + "\n实际 MD5: " + actualMd5);
            downloadReply->deleteLater();
            return;
        }

        emit statusUpdated("✅ 文件校验通过！");

        // ---- 替换文件 ----
        QString currentAppPath = QCoreApplication::applicationDirPath() + "/MyApp.exe";
        QString backupPath = currentAppPath + ".bak";

        // 1. 备份旧文件（如果存在）
        if (QFile::exists(currentAppPath)) {
            QFile::remove(backupPath);
            if (!QFile::rename(currentAppPath, backupPath)) {
                emit errorOccurred("备份旧文件失败！");
                downloadReply->deleteLater();
                return;
            }
            qDebug() << "✅ 已备份旧文件到：" << backupPath;
        } else {
            qDebug() << "📁 首次安装，无需备份";
        }

        // 2. 复制新文件
        if (!QFile::copy(tempFile, currentAppPath)) {
            emit errorOccurred("替换文件失败！");
            // 如果有备份，尝试恢复
            if (QFile::exists(backupPath)) {
                QFile::rename(backupPath, currentAppPath);
                qDebug() << "🔄 已恢复旧文件";
            }
            downloadReply->deleteLater();
            return;
        }

        qDebug() << "✅ 新文件已安装到：" << currentAppPath;

        // 3. 删除临时文件
        QFile::remove(tempFile);

        emit statusUpdated("✅ 已完成！正在重启应用...");
        emit finished();
        downloadReply->deleteLater();
    }
};

// ========================================
// 主窗口
// ========================================
class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("自动更新器 v1.0");
        resize(450, 200);

        QPushButton *btn = new QPushButton("检查更新", this);
        QProgressBar *bar = new QProgressBar(this);
        bar->setRange(0, 100);
        bar->setValue(0);

        QLabel *statusLabel = new QLabel("点击「检查更新」", this);
        statusLabel->setAlignment(Qt::AlignCenter);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(statusLabel);
        layout->addWidget(bar);
        layout->addWidget(btn);
        setLayout(layout);

        connect(btn, &QPushButton::clicked, this, [=]() {
            btn->setEnabled(false);
            btn->setText("更新中...");
            statusLabel->setText("正在检查版本...");
            bar->setValue(0);

            QThread *thread = new QThread;
            UpdateWorker *worker = new UpdateWorker;
            worker->moveToThread(thread);

            // TODO: 替换成你自己的服务器地址
            QString versionUrl = "http://localhost:8000/version.json";

            connect(thread, &QThread::started, [=]() {
                worker->doUpdate(versionUrl);
            });

            connect(worker, &UpdateWorker::progressUpdated, bar, &QProgressBar::setValue);
            connect(worker, &UpdateWorker::statusUpdated, statusLabel, &QLabel::setText);

            connect(worker, &UpdateWorker::noUpdateNeeded, this, [=]() {
                statusLabel->setText("✅ 已是最新版本");
                btn->setEnabled(true);
                btn->setText("检查更新");
                thread->quit();
                worker->deleteLater();
                thread->deleteLater();
            });

            connect(worker, &UpdateWorker::finished, this, [=]() {
                statusLabel->setText("✅ 更新完成，正在重启...");
                QMessageBox::information(this, "提示", "更新成功，应用即将重启！");
                QProcess::startDetached(QCoreApplication::applicationDirPath() + "/MyApp.exe");
                QApplication::quit();
            });

            connect(worker, &UpdateWorker::errorOccurred, this, [=](QString msg) {
                statusLabel->setText("❌ " + msg);
                btn->setEnabled(true);
                btn->setText("重试");
                QMessageBox::critical(this, "错误", msg);
                thread->quit();
                worker->deleteLater();
                thread->deleteLater();
            });

            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            thread->start();
        });
    }
};

// ========================================
// 程序入口
// ========================================
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "main.moc"
