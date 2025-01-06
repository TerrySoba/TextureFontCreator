#include "texturefontcreatorgui.h"

#include <QFileDialog>
#include "FreeTypeRender.h"

#include <string>
#include "character_sets.h"
#include <QSettings>
#include <QGraphicsItem>



TextureFontCreatorGUI::TextureFontCreatorGUI(QWidget *parent)
    : QMainWindow(parent),
      m_pixmapItem(NULL)
{
	m_ui.setupUi(this);

	m_ui.previewGraphicsView->setScene(&m_scene);

	connect(m_ui.fontPathBrowseButton, SIGNAL(clicked()), this, SLOT(browseFontPath()));
	connect(m_ui.updatePreviewButton, SIGNAL(clicked()), this, SLOT(updatePreview()));
    connect(m_ui.actionSave_Texture_Font, SIGNAL(triggered(bool)), this, SLOT(saveAs()));

	// load configuration
    QSettings settings("TextureFontCreator", "TextureFontCreator");
	m_ui.fontPathEdit->setText(settings.value("lastOpenedFont", "").toString());
	m_ui.fontInputSizeSpinBox->setValue(settings.value("fontSize", 20.0).toDouble());
	m_ui.powerOfTwoCheckBox->setChecked(settings.value("powerOfTwo", true).toBool());
	m_ui.antialisedCheckBox->setChecked(settings.value("antialised", true).toBool());
	m_ui.hintedCheckBox->setChecked(settings.value("hinted", true).toBool());

	m_ui.ASCII_checkBox->setChecked(settings.value("asciiCheckbox", true).toBool());
	m_ui.ISO_8859_15_checkBox->setChecked(settings.value("ISO_8859_15Checkbox", false).toBool());
	m_ui.japaneseKatakanaCheckbox->setChecked(settings.value("japaneseKatakana", false).toBool());

	m_ui.japaneseHiraganaCheckbox->setChecked(settings.value("japaneseHiragana", false).toBool());
	m_ui.japaneseKatakanaCheckbox->setChecked(settings.value("japaneseKatakana", false).toBool());
	m_ui.japaneseKanjiCheckbox->setChecked(settings.value("japaneseKanji", false).toBool());

	m_ui.customCharacterSetGroupBox->setChecked(settings.value("customCharacterset", false).toBool());
	m_ui.customCharacterSetTextEdit->setText(settings.value("customCharactersetString", "").toString());
}

std::shared_ptr<TextureFontCreator> TextureFontCreatorGUI::createTextureFont() {
	// create string with selected charsets
	std::string charSets = "";
	if (m_ui.ASCII_checkBox->isChecked()) {
		charSets += CHAR_SET_ASCII;
	}
	if (m_ui.ISO_8859_15_checkBox->isChecked()) {
		charSets += CHAR_SET_ISO_8859_15_danish;
	}
	if (m_ui.customCharacterSetGroupBox->isChecked()) {
		QByteArray utf8_text =
				m_ui.customCharacterSetTextEdit->document()->toPlainText().toUtf8();
		charSets += utf8_text.constData();
	}
	if (m_ui.japaneseHiraganaCheckbox->isChecked()) {
		charSets += JAPANESE_HIRAGANA;
	}
	if (m_ui.japaneseKatakanaCheckbox->isChecked()) {
		charSets += JAPANESE_KATAKANA;
	}
	if (m_ui.japaneseKanjiCheckbox->isChecked()) {
		charSets += JAPANESE_JOYO_KANJI;
	}

	auto creator = std::make_shared<TextureFontCreator>(m_ui.fontPathEdit->text().toStdString().c_str(),
				m_ui.fontInputSizeSpinBox->value(), m_ui.powerOfTwoCheckBox->isChecked(),
				charSets.c_str(), m_ui.antialisedCheckBox->isChecked(), m_ui.hintedCheckBox->isChecked());

	return creator;
}

void TextureFontCreatorGUI::updatePreview() {

	std::shared_ptr<TextureFontCreator> creator = createTextureFont();

	std::shared_ptr<GrayImage> img = creator->getImage();
	std::shared_ptr<QImage> image = img->getQImage();
	m_pixmap = QPixmap::fromImage(*image);
	
	// scale pixmap by selected factor
	m_pixmap = m_pixmap.scaled(m_pixmap.width() * m_ui.zoomSlider->value(),
			m_pixmap.height() * m_ui.zoomSlider->value(), Qt::IgnoreAspectRatio, Qt::FastTransformation);

	if (m_pixmapItem) {
		m_scene.removeItem(m_pixmapItem);
		delete m_pixmapItem;
	}
	m_pixmapItem = m_scene.addPixmap(m_pixmap);

	m_ui.previewGraphicsView->ensureVisible(m_pixmapItem);

	m_ui.fontNameLabel->setText(creator->getFontName().c_str());
}



void TextureFontCreatorGUI::browseFontPath() {
	QString filename = QFileDialog::getOpenFileName( this, "Select Input Font", m_ui.fontPathEdit->text(), "Fonts (*.ttf *.TTF *.otf *.OTF *.ttc *.TTC *.pfb *.PFB);;All Files (*)");
	if (!filename.isEmpty()) {
		m_ui.fontPathEdit->setText(filename);
	}
}

void TextureFontCreatorGUI::saveAs() {
	QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font", m_lastSavePath, "Texture Fonts (*.ytf);;All Files (*)");
	if (!filename.isEmpty()) {
		// std::cout << "Saved file to " << filename.toLocal8Bit().constData() << std::endl;
		std::shared_ptr<TextureFontCreator> creator = createTextureFont();
		creator->writeToFile(filename.toLocal8Bit().constData());
		m_lastSavePath = filename;
	}
}


TextureFontCreatorGUI::~TextureFontCreatorGUI()
{
    QSettings settings("TextureFontCreator", "TextureFontCreator");
	settings.setValue("lastOpenedFont", m_ui.fontPathEdit->text());
	settings.setValue("fontSize", m_ui.fontInputSizeSpinBox->value());
	settings.setValue("powerOfTwo", m_ui.powerOfTwoCheckBox->isChecked());
	settings.setValue("antialised", m_ui.antialisedCheckBox->isChecked());
	settings.setValue("hinted", m_ui.hintedCheckBox->isChecked());

	settings.setValue("asciiCheckbox", m_ui.ASCII_checkBox->isChecked());
	settings.setValue("ISO_8859_15Checkbox", m_ui.ISO_8859_15_checkBox->isChecked());

	settings.setValue("japaneseHiragana", m_ui.japaneseHiraganaCheckbox->isChecked());
	settings.setValue("japaneseKatakana", m_ui.japaneseKatakanaCheckbox->isChecked());
	settings.setValue("japaneseKanji", m_ui.japaneseKanjiCheckbox->isChecked());

	settings.setValue("customCharacterset", m_ui.customCharacterSetGroupBox->isChecked());
	settings.setValue("customCharactersetString", m_ui.customCharacterSetTextEdit->document()->toPlainText());

	delete m_pixmapItem;
}
