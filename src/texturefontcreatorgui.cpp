#include "texturefontcreatorgui.h"

#include <QFileDialog>
#include "FreeTypeRender.h"

#include <string>
#include "character_sets.h"
#include <QSettings>
#include <QGraphicsItem>



TextureFontCreatorGUI::TextureFontCreatorGUI(QWidget *parent)
    : QMainWindow(parent),
      pixmapItem(NULL)
{
	ui.setupUi(this);

	ui.previewGraphicsView->setScene(&scene);

	connect(ui.fontPathBrowseButton, SIGNAL(clicked()), this, SLOT(browseFontPath()));
	connect(ui.updatePreviewButton, SIGNAL(clicked()), this, SLOT(updatePreview()));
    connect(ui.actionSave_Texture_Font, SIGNAL(triggered(bool)), this, SLOT(saveAs()));

	// load configuration
    QSettings settings("TextureFontCreator", "TextureFontCreator");
	ui.fontPathEdit->setText(settings.value("lastOpenedFont", "").toString());
	ui.fontInputSizeSpinBox->setValue(settings.value("fontSize", 20.0).toDouble());
	ui.powerOfTwoCheckBox->setChecked(settings.value("powerOfTwo", true).toBool());

	ui.ASCII_checkBox->setChecked(settings.value("asciiCheckbox", true).toBool());
	ui.ISO_8859_15_checkBox->setChecked(settings.value("ISO_8859_15Checkbox", false).toBool());
	ui.japaneseKatakanaCheckbox->setChecked(settings.value("japaneseKatakana", false).toBool());

	ui.japaneseHiraganaCheckbox->setChecked(settings.value("japaneseHiragana", false).toBool());
	ui.japaneseKatakanaCheckbox->setChecked(settings.value("japaneseKatakana", false).toBool());
	ui.japaneseKanjiCheckbox->setChecked(settings.value("japaneseKanji", false).toBool());

	ui.customCharacterSetGroupBox->setChecked(settings.value("customCharacterset", false).toBool());
	ui.customCharacterSetTextEdit->setText(settings.value("customCharactersetString", "").toString());
}

std::shared_ptr<TextureFontCreator> TextureFontCreatorGUI::createTextureFont() {
	// create string with selected charsets
	std::string charSets = "";
	if (ui.ASCII_checkBox->isChecked()) {
		charSets += CHAR_SET_ASCII;
	}
	if (ui.ISO_8859_15_checkBox->isChecked()) {
		charSets += CHAR_SET_ISO_8859_15_danish;
	}
	if (ui.customCharacterSetGroupBox->isChecked()) {
		QByteArray utf8_text =
				ui.customCharacterSetTextEdit->document()->toPlainText().toUtf8();
		charSets += utf8_text.constData();
	}
	if (ui.japaneseHiraganaCheckbox->isChecked()) {
		charSets += JAPANESE_HIRAGANA;
	}
	if (ui.japaneseKatakanaCheckbox->isChecked()) {
		charSets += JAPANESE_KATAKANA;
	}
	if (ui.japaneseKanjiCheckbox->isChecked()) {
		charSets += JAPANESE_JOYO_KANJI;
	}

	std::shared_ptr<TextureFontCreator> creator(new TextureFontCreator(ui.fontPathEdit->text().toStdString().c_str(),
				ui.fontInputSizeSpinBox->value(), ui.powerOfTwoCheckBox->isChecked(),
				charSets.c_str()));

//	std::shared_ptr<TextureFontCreator> creator(new TextureFontCreator(ui.fontPathEdit->text().toStdString().c_str(),
//			ui.fontInputSizeSpinBox->value(), ui.textureSizeSpinBox->value(),
//			charSets.c_str()));

	return creator;
}

void TextureFontCreatorGUI::updatePreview() {

	std::shared_ptr<TextureFontCreator> creator = createTextureFont();

	std::shared_ptr<GrayImage> img = creator->getImage();
	std::shared_ptr<QImage> image = img->getQImage();
	pixmap = QPixmap::fromImage(*image);
	// ui.previewLabel->setPixmap(pixmap);
	if (pixmapItem) {
		scene.removeItem(pixmapItem);
		delete pixmapItem;
	}
	pixmapItem = scene.addPixmap(pixmap);

	ui.previewGraphicsView->ensureVisible(pixmapItem);

	ui.fontNameLabel->setText(creator->getFontName().c_str());
}



void TextureFontCreatorGUI::browseFontPath() {
	QString filename = QFileDialog::getOpenFileName( this, "Select Input Font", ui.fontPathEdit->text(), "Fonts (*.ttf *.TTF *.otf *.OTF *.ttc *.TTC *.pfb *.PFB);;All Files (*)");
	if (!filename.isEmpty()) {
		ui.fontPathEdit->setText(filename);
	}
}

void TextureFontCreatorGUI::saveAs() {
	QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font", lastSavePath, "Texture Fonts (*.ytf);;All Files (*)");
	if (!filename.isEmpty()) {
		// std::cout << "Saved file to " << filename.toLocal8Bit().constData() << std::endl;
		std::shared_ptr<TextureFontCreator> creator = createTextureFont();
		creator->writeToFile(filename.toLocal8Bit().constData());
		lastSavePath = filename;
	}
}


TextureFontCreatorGUI::~TextureFontCreatorGUI()
{
    QSettings settings("TextureFontCreator", "TextureFontCreator");
	settings.setValue("lastOpenedFont", ui.fontPathEdit->text());
	settings.setValue("fontSize", ui.fontInputSizeSpinBox->value());
	settings.setValue("powerOfTwo", ui.powerOfTwoCheckBox->isChecked());

	settings.setValue("asciiCheckbox", ui.ASCII_checkBox->isChecked());
	settings.setValue("ISO_8859_15Checkbox", ui.ISO_8859_15_checkBox->isChecked());

	settings.setValue("japaneseHiragana", ui.japaneseHiraganaCheckbox->isChecked());
	settings.setValue("japaneseKatakana", ui.japaneseKatakanaCheckbox->isChecked());
	settings.setValue("japaneseKanji", ui.japaneseKanjiCheckbox->isChecked());

	settings.setValue("customCharacterset", ui.customCharacterSetGroupBox->isChecked());
	settings.setValue("customCharactersetString", ui.customCharacterSetTextEdit->document()->toPlainText());

	delete pixmapItem;
}
