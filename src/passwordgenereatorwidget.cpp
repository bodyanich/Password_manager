#include "passwordgenereatorwidget.h"
#include "ui_passwordgenereatorwidget.h"
#include "zxcvbn/zxcvbn.h"
#include <QClipboard>
#include "passwordgeneratorcore.h"
#include "QDebug"

PasswordGenereatorWidget::PasswordGenereatorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordGenereatorWidget),
    m_passwordGenerator(new PasswordGenerator())
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setMaximumHeight(0);
    ui->progressBar->setMaximum(120);

    ui->sliderLength->setMinimum(4);
    ui->spinBoxLength->setMinimum(4);
    pclipboard = QGuiApplication::clipboard();

    connect(ui->buttonCopy, SIGNAL(clicked()), SLOT(copyPassword()));
    connect(ui->buttonGenerate, SIGNAL(clicked()), SLOT(regeneratePassword()));

    connect(ui->sliderLength, SIGNAL(valueChanged(int)), SLOT(passwordLengthChanged(int)));
    connect(ui->spinBoxLength, SIGNAL(valueChanged(int)), SLOT(passwordLengthChanged(int)));

    connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), SLOT(updateGenerator()));
    connect(ui->checkBox, SIGNAL(stateChanged(int)), SLOT(updateGenerator()));
    connect(ui->checkBox_2, SIGNAL(stateChanged(int)), SLOT(updateGenerator()));
    connect(ui->buttonClose, SIGNAL(clicked()), SIGNAL(closed()));
    connect(ui->custom_char_set, SIGNAL(textChanged(QString)), SLOT(updateGenerator()));

    ui->buttonGenerate->setIcon(QIcon(":/Icons/circular-left-arrow.png"));
    ui->buttonGenerate->setIconSize(QSize(24, 24));
    ui->buttonCopy->setIcon(QIcon(":/Icons/copy.png"));
    ui->buttonCopy->setIconSize(QSize(24, 24));

    ui->checkBoxLower->setChecked(true);
    ui->checkBoxUpper->setChecked(true);
    ui->checkBoxNumbers->setChecked(true);
    ui->checkBoxSpecialChars->setChecked(false);
    ui->checkBox->setChecked(true);

    updateGenerator();
}

PasswordGenereatorWidget::~PasswordGenereatorWidget()
{
    delete ui;
}



void PasswordGenereatorWidget::on_lineEdit_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        {
            ui->progressBar->setValue(0);
            ui->progressBar->setMaximumHeight(0);
        }
        else
        {
            auto entropy = ZxcvbnMatch(arg1.toUtf8(), nullptr, nullptr);
            ui->entropyLabel->setText("Entropy = " + QString::number(entropy));
            ui->progressBar->setValue(std::min(int(entropy), ui->progressBar->maximum()));
            ui->progressBar->setMaximumHeight(5);
            if (entropy < 25)
            {
                ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #C43F31}");
                ui->strengthLabel->setText("Strength is bad");
            } else if (entropy < 50)
            {
                ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #DB9837}");
                ui->strengthLabel->setText("Strength is poor");
            } else if (entropy < 80)
            {
                ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #F0C400}");
                ui->strengthLabel->setText("Strength is weak");

            } else if (entropy < 100)
            {
                ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #608A22}");
                ui->strengthLabel->setText("Strength is good");
            }
            else
            {
                ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #1F8023}");
                ui->strengthLabel->setText("Strength is excellent");
            }
        }
}

void PasswordGenereatorWidget::regeneratePassword()
{
    if (m_passwordGenerator->isValid()) {
        ui->lineEdit->setText(m_passwordGenerator->generatePassword());
    }
}

void PasswordGenereatorWidget::copyPassword()
{
    pclipboard->setText(ui->lineEdit->text());
}

void PasswordGenereatorWidget::passwordLengthChanged(int length)
{
    ui->spinBoxLength->blockSignals(true);
    ui->sliderLength->blockSignals(true);

    ui->spinBoxLength->setValue(length);
    ui->sliderLength->setValue(length);

    ui->spinBoxLength->blockSignals(false);
    ui->sliderLength->blockSignals(false);

    updateGenerator();
}

void PasswordGenereatorWidget::updateGenerator()
{
    auto classes = charClasses();
    auto flags = generatorFlags();

    m_passwordGenerator->setLength(ui->spinBoxLength->value());
    m_passwordGenerator->setCharClasses(classes);
    m_passwordGenerator->setFlags(flags);
    m_passwordGenerator->setCustomCharacterSet(ui->custom_char_set->text());

    if (m_passwordGenerator->isValid()) {
        ui->buttonGenerate->setEnabled(true);
    } else {
        ui->buttonGenerate->setEnabled(false);
    }
    regeneratePassword();
}

PasswordGenerator::CharClasses PasswordGenereatorWidget::charClasses()
{
    PasswordGenerator::CharClasses classes;

    if (ui->checkBoxLower->isChecked()) {
        classes |= PasswordGenerator::LowerLetters;
        qDebug() << "lower checked";
    }

    if (ui->checkBoxUpper->isChecked()) {
        classes |= PasswordGenerator::UpperLetters;
        qDebug() << "upper checked";
    }

    if (ui->checkBoxNumbers->isChecked()) {
        classes |= PasswordGenerator::Numbers;
        qDebug() << "numbers checked";
    }

    if (ui->checkBoxExtASCII->isChecked()) {
        classes |= PasswordGenerator::EASCII;
    }
    if (ui->checkBoxSpecialChars->isChecked()) {
        classes |= PasswordGenerator::SpecialCharacters;
    }
    if (ui->checkBoxBraces->isChecked()) {
        classes |= PasswordGenerator::Braces;
    }

    if (ui->checkBoxPunctuation->isChecked()) {
        classes |= PasswordGenerator::Punctuation;
    }

    if (ui->checkBoxQuotes->isChecked()) {
        classes |= PasswordGenerator::Quotes;
    }

    if (ui->checkBoxDashes->isChecked()) {
        classes |= PasswordGenerator::Dashes;
    }

    if (ui->checkBoxMath->isChecked()) {
        classes |= PasswordGenerator::Math;
    }

    return classes;
}

PasswordGenerator::GeneratorFlags PasswordGenereatorWidget::generatorFlags()
{
    PasswordGenerator::GeneratorFlags flags;
    if(ui->checkBox->isChecked())
        flags |= PasswordGenerator::ExcludeLookAlike;

    if(ui->checkBox_2->isChecked())
        flags |= PasswordGenerator::CharFromEveryGroup;

    return flags;
}

