// ========================================
// MyApp.exe - 黑背景 + 白色斜体 HELLO！
// 使用手写字体
// ========================================

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ============================================
    // 创建窗口
    // ============================================
    QWidget window;
    window.setWindowTitle("Welcome");
    window.resize(600, 400);
    window.setStyleSheet("background-color: black;");

    // ============================================
    // 创建文字标签
    // ============================================
    QLabel *label = new QLabel("hello!", &window);

    // ============================================
    // 设置手写字体
    // ============================================
    QFont font;

    // 选择一款可用的手写字体（按优先级尝试）
    QStringList handWritingFonts = {
        "Bradley Hand",      // 美式手写
        "Segoe Script",      // 流畅手写
        "Comic Sans MS",     // 卡通手写
        "Caveat",            // 真实手写（需下载）
        "Dancing Script",    // 优雅手写（需下载）
        "Patrick Hand"       // 学生手写（需下载）
    };

    // 找第一个可用的手写字体
    QString chosenFont;
    QFontDatabase db;
    QStringList availableFonts = db.families();

    for (const QString &fontName : handWritingFonts) {
        if (availableFonts.contains(fontName)) {
            chosenFont = fontName;
            break;
        }
    }

    // 如果都没找到，用系统默认字体 + 斜体模拟手写
    if (chosenFont.isEmpty()) {
        font.setFamily("Arial");
        font.setStyleHint(QFont::Serif);
    } else {
        font.setFamily(chosenFont);
    }

    font.setPointSize(80);      // 大号
    font.setItalic(true);       // 斜体
    font.setBold(true);         // 粗体（更明显）

    label->setFont(font);
    label->setStyleSheet("color: white;");
    label->setAlignment(Qt::AlignCenter);

    // ============================================
    // 布局
    // ============================================
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    window.setLayout(layout);

    window.show();

    return a.exec();
}
