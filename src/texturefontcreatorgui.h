#ifndef TEXTUREFONTCREATORGUI_H
#define TEXTUREFONTCREATORGUI_H

#include <QMainWindow>
#include "ui_texturefontcreatorgui.h"
#include <QPixmap>
#include <QGraphicsScene>
#include <memory>
#include "TextureFontCreator.h"

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

private:
	std::shared_ptr<TextureFontCreator> createTextureFont();

    Ui::TextureFontCreatorGUIClass m_ui;
    QPixmap m_pixmap;
    QString m_lastSavePath;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_pixmapItem;
};

#endif // TEXTUREFONTCREATORGUI_H
