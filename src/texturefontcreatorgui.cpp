#include "texturefontcreatorgui.h"

#include "FreeTypeRender.h"
#include "character_sets.h"

#include <QFileDialog>
#include <QSettings>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QStyle>

#include <string>

namespace { // anonymous namespace

std::u8string toU8String(const QString& str)
{
    return std::u8string(reinterpret_cast<const char8_t*>(str.toUtf8().constData()));
}

QString toQString(const std::u8string& str)
{
    return QString::fromUtf8(reinterpret_cast<const char*>(str.c_str()));
}


std::shared_ptr<TextureFontCreator> createTextureFont(const GuiParameters& parameters)
{
    // create string with selected charsets
    std::u8string charSets;
    if (parameters.ascii) {
        charSets += CHAR_SET_ASCII;
    }
    if (parameters.iso8859_15) {
        charSets += CHAR_SET_ISO_8859_15_danish;
    }
    
    charSets += parameters.customCharacterSet.value_or(u8"");
    
    if (parameters.japaneseHiragana) {
        charSets += JAPANESE_HIRAGANA;
    }
    if (parameters.japaneseKatakana) {
        charSets += JAPANESE_KATAKANA;
    }
    if (parameters.japaneseKanji) {
        charSets += JAPANESE_JOYO_KANJI;
    }

    auto creator = std::make_shared<TextureFontCreator>(
        parameters.fontPath,
        parameters.fontSize,
        parameters.powerOfTwo,
        charSets,
        parameters.antialised,
        parameters.hinted);

    return creator;
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

    // set icons for menu items
    m_ui.actionSave_Texture_Font->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_ui.actionSave_JSON_File->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_ui.actionSave_Simple_Font->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_ui.actionAbout->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    m_ui.actionExit->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));

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

std::optional<GuiParameters> TextureFontCreatorGUI::getGuiSettings()
{
    GuiParameters params;
    params.fontPath = toU8String(m_ui.fontPathEdit->text());
    params.fontSize = m_ui.fontInputSizeSpinBox->value();
    params.powerOfTwo = m_ui.powerOfTwoCheckBox->isChecked();
    params.antialised = m_ui.antialisedCheckBox->isChecked();
    params.hinted = m_ui.hintedCheckBox->isChecked();
    if (m_ui.customCharacterSetGroupBox->isChecked())
    {
        params.customCharacterSet = toU8String(m_ui.customCharacterSetTextEdit->document()->toPlainText());
    }
    params.ascii = m_ui.ASCII_checkBox->isChecked();
    params.iso8859_15 = m_ui.ISO_8859_15_checkBox->isChecked();
    params.japaneseHiragana = m_ui.japaneseHiraganaCheckbox->isChecked();
    params.japaneseKatakana = m_ui.japaneseKatakanaCheckbox->isChecked();
    params.japaneseKanji = m_ui.japaneseKanjiCheckbox->isChecked();

    if (!std::filesystem::exists(params.fontPath))
    {
        // display message box
        QMessageBox::critical(this, "Error", QString("Font file \"%1\" does not exist.").arg(toQString(params.fontPath.u8string())));
        return {};
    }

    return params;
}

void TextureFontCreatorGUI::updatePreview()
{
    auto parameters = getGuiSettings();
    if (!parameters)
    {
        return;
    }

    std::shared_ptr<TextureFontCreator> creator = createTextureFont(*parameters);

    std::shared_ptr<GrayImage> img = creator->getImage();
    std::shared_ptr<QImage> image = img->getQImage();
    m_pixmap = QPixmap::fromImage(*image);

    int width = m_pixmap.width();
    int height = m_pixmap.height();
    int pixelCount = width * height;

    QString sizeText = QString("%1").arg(pixelCount);
    for (int i = sizeText.size() - 3; i > 0; i -= 3) {
        sizeText.insert(i, '.');
    }

    m_ui.imageSizeLabel->setText(QString("%1x%2 (%3 pixels)").arg(width).arg(height).arg(sizeText));

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


    std::u8string sampleText = toU8String(m_ui.textPreviewLineEdit->text());
    std::shared_ptr<GrayImage> sampleImage = creator->renderText(sampleText);
    std::shared_ptr<QImage> sampleQImage = sampleImage->getQImage();

    QPixmap samplePixmap = QPixmap::fromImage(*sampleQImage);
    samplePixmap = samplePixmap.scaled(samplePixmap.width() * m_ui.zoomSlider->value(),
            samplePixmap.height() * m_ui.zoomSlider->value(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    
    m_ui.textPreviewLabel->setPixmap(samplePixmap);
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
        auto parameters = getGuiSettings();
        if (!parameters)
        {
            return;
        }
        std::shared_ptr<TextureFontCreator> creator = createTextureFont(*parameters);
        creator->writeToFile(toU8String(filename));
        m_lastSavePath = filename;
    }
}

void TextureFontCreatorGUI::saveAsJson() {
    QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font JSON", m_lastSavePath, "JSON Texture Fonts (*.json);;All Files (*)");
    if (!filename.isEmpty()) {
        auto parameters = getGuiSettings();
        if (!parameters)
        {
            return;
        }
        std::shared_ptr<TextureFontCreator> creator = createTextureFont(*parameters);
        creator->writeToJsonFile(toU8String(filename));
        m_lastSavePath = filename;
    }
}

void TextureFontCreatorGUI::saveAsSimple() {
    QString filename = QFileDialog::getSaveFileName( this, "Save Texture Font Simple", m_lastSavePath, "Simple Texture Fonts (*.stf);;All Files (*)");
    if (!filename.isEmpty()) {
        auto parameters = getGuiSettings();
        if (!parameters)
        {
            return;
        }
        std::shared_ptr<TextureFontCreator> creator = createTextureFont(*parameters);
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
