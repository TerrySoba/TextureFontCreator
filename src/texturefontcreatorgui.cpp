#include "texturefontcreatorgui.h"

#include <QFileDialog>
#include "FreeTypeRender.h"

#include <string>
#include "character_sets.h"
#include <QSettings>
#include <QGraphicsItem>

namespace { // anonymous namespace

std::u8string toU8String(const QString& str) {
    return std::u8string(reinterpret_cast<const char8_t*>(str.toUtf8().constData()));
}

} // anonymous namespace

static QWidget* createAboutDialog()
{
    QFile file(":/about.ui");
    if (!file.open(QFile::ReadOnly))
    {
        throw std::runtime_error("Could not open UI file");
    }

    QUiLoader loader;
    QWidget* aboutDialog = loader.load(&file);
    file.close();

    return aboutDialog;
}

TextureFontCreatorGUI::TextureFontCreatorGUI(QWidget *parent)
    : QMainWindow(parent),
      m_pixmapItem(nullptr),
      m_aboutDialog(nullptr)
{
    m_ui.setupUi(this);

    m_aboutDialog = createAboutDialog();

    m_ui.previewGraphicsView->setScene(&m_scene);

    connect(m_ui.fontPathBrowseButton, SIGNAL(clicked()), this, SLOT(browseFontPath()));
    connect(m_ui.updatePreviewButton, SIGNAL(clicked()), this, SLOT(updatePreview()));
    connect(m_ui.actionSave_Texture_Font, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(m_ui.actionSave_JSON_File, SIGNAL(triggered(bool)), this, SLOT(saveAsJson()));
    connect(m_ui.actionSave_Simple_Font, SIGNAL(triggered(bool)), this, SLOT(saveAsSimple()));

    connect(m_ui.actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAboutDialog()));

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

void TextureFontCreatorGUI::showAboutDialog()
{
    if (m_aboutDialog)
    {
        m_aboutDialog->show();
    }
}

std::shared_ptr<TextureFontCreator> TextureFontCreatorGUI::createTextureFont() {
    // create string with selected charsets
    std::u8string charSets;
    if (m_ui.ASCII_checkBox->isChecked()) {
        charSets += CHAR_SET_ASCII;
    }
    if (m_ui.ISO_8859_15_checkBox->isChecked()) {
        charSets += CHAR_SET_ISO_8859_15_danish;
    }
    if (m_ui.customCharacterSetGroupBox->isChecked()) {
        auto utf8_text =
                toU8String(m_ui.customCharacterSetTextEdit->document()->toPlainText());
        charSets += utf8_text;
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

    auto creator = std::make_shared<TextureFontCreator>(toU8String(m_ui.fontPathEdit->text()),
                m_ui.fontInputSizeSpinBox->value(), m_ui.powerOfTwoCheckBox->isChecked(),
                charSets.c_str(), m_ui.antialisedCheckBox->isChecked(), m_ui.hintedCheckBox->isChecked());

    return creator;
}

void TextureFontCreatorGUI::updatePreview() {

    std::shared_ptr<TextureFontCreator> creator = createTextureFont();

    std::shared_ptr<GrayImage> img = creator->getImage();
    std::shared_ptr<QImage> image = img->getQImage();
    m_pixmap = QPixmap::fromImage(*image);
    

    m_ui.imageSizeLabel->setText(QString("%1x%2 (%3)").arg(m_pixmap.width()).arg(m_pixmap.height()).arg(m_pixmap.width() * m_pixmap.height()));

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
    QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font", m_lastSavePath, "Binary Texture Fonts (*.ytf);;All Files (*)");
    if (!filename.isEmpty()) {
        std::shared_ptr<TextureFontCreator> creator = createTextureFont();
        creator->writeToFile(toU8String(filename));
        m_lastSavePath = filename;
    }
}

void TextureFontCreatorGUI::saveAsJson() {
    QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font JSON", m_lastSavePath, "JSON Texture Fonts (*.json);;All Files (*)");
    if (!filename.isEmpty()) {
        std::shared_ptr<TextureFontCreator> creator = createTextureFont();

        
        creator->writeToJsonFile(toU8String(filename));
        m_lastSavePath = filename;
    }
}

void TextureFontCreatorGUI::saveAsSimple() {
    QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font Simple", m_lastSavePath, "Simple Texture Fonts (*.stf);;All Files (*)");
    if (!filename.isEmpty()) {
        std::shared_ptr<TextureFontCreator> creator = createTextureFont();
        creator->writeToSimpleFile(toU8String(filename));
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
