#ifndef TEXTUREFONTCREATORGUI_H
#define TEXTUREFONTCREATORGUI_H

#include <QMainWindow>
#include "ui_texturefontcreatorgui.h"
#include <QPixmap>
#include <QGraphicsScene>
#include <QUiLoader>

#include <memory>
#include <optional>

#include "TextureFontCreator.h"

struct GuiParameters {
    std::filesystem::path fontPath;
    double fontSize;
    bool powerOfTwo;
    bool antialised;
    bool hinted;
    std::optional<std::u8string> customCharacterSet;
    bool ascii;
    bool iso8859_15;
    bool japaneseHiragana;
    bool japaneseKatakana;
    bool japaneseKanji;
};

class TextureFontCreatorGUI : public QMainWindow
{
    Q_OBJECT

public:
    TextureFontCreatorGUI(QWidget *parent = 0);
    ~TextureFontCreatorGUI();

public slots:
    void browseFontPath();
    void updatePreview();
    void saveAs();
    void saveAsJson();
    void saveAsSimple();

    void showAboutDialog();

private: // methods
    std::optional<GuiParameters> getGuiSettings();

private: // members
    Ui::TextureFontCreatorGUIClass m_ui;
    QPixmap m_pixmap;
    QString m_lastSavePath;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_pixmapItem;

    QWidget* m_aboutDialog;
};

#endif // TEXTUREFONTCREATORGUI_H
