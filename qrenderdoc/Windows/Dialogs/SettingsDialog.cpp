/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "SettingsDialog.h"
#include "Code/Interface/QRDInterface.h"
#include "Code/QRDUtils.h"
#include "Styles/StyleData.h"
#include "Widgets/OrderedListEditor.h"
#include "CaptureDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(ICaptureContext &ctx, QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog), m_Ctx(ctx)
{
  ui->setupUi(this);

  QString styleChooseTooltip = ui->UIStyle->toolTip();

  for(int i = 0; i < StyleData::numAvailable; i++)
    styleChooseTooltip += lit("<br>- ") + StyleData::availStyles[i].styleDescription;

  ui->UIStyle->setToolTip(styleChooseTooltip);
  ui->UIStyle_label->setToolTip(styleChooseTooltip);

  for(int i = 0; i < StyleData::numAvailable; i++)
    ui->UIStyle->addItem(StyleData::availStyles[i].styleName);

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  ui->tabWidget->tabBar()->setVisible(false);

  for(int i = 0; i < ui->tabWidget->count(); i++)
    ui->pages->addItem(ui->tabWidget->tabText(i));

  m_Init = true;

  for(int i = 0; i < (int)TimeUnit::Count; i++)
  {
    ui->EventBrowser_TimeUnit->addItem(UnitSuffix((TimeUnit)i));
  }

  ui->pages->clearSelection();
  ui->pages->setItemSelected(ui->pages->item(0), true);
  ui->tabWidget->setCurrentIndex(0);

  ui->pages->setMinimumWidth(ui->pages->sizeHintForColumn(0));
  ui->pages->adjustSize();

  for(int i = 0; i < StyleData::numAvailable; i++)
  {
    if(StyleData::availStyles[i].styleID == m_Ctx.Config().UIStyle)
    {
      ui->UIStyle->setCurrentIndex(i);
      break;
    }
  }

  ui->saveDirectory->setText(m_Ctx.Config().DefaultCaptureSaveDirectory);
  ui->tempDirectory->setText(m_Ctx.Config().TemporaryCaptureDirectory);

  if(!m_Ctx.Config().SPIRVDisassemblers.isEmpty())
  {
    ui->externalDisassemblerArgs->setText(m_Ctx.Config().SPIRVDisassemblers[0].args);
    ui->externalDisassemblePath->setText(m_Ctx.Config().SPIRVDisassemblers[0].executable);
  }
  ui->Android_SDKPath->setText(m_Ctx.Config().Android_SDKPath);
  ui->Android_JDKPath->setText(m_Ctx.Config().Android_JDKPath);
  ui->Android_MaxConnectTimeout->setValue(m_Ctx.Config().Android_MaxConnectTimeout);
  ui->Android_AutoPushLayerToApp->setChecked(m_Ctx.Config().Android_AutoPushLayerToApp);

  ui->TextureViewer_ResetRange->setChecked(m_Ctx.Config().TextureViewer_ResetRange);
  ui->TextureViewer_PerTexSettings->setChecked(m_Ctx.Config().TextureViewer_PerTexSettings);
  ui->ShaderViewer_FriendlyNaming->setChecked(m_Ctx.Config().ShaderViewer_FriendlyNaming);
  ui->CheckUpdate_AllowChecks->setChecked(m_Ctx.Config().CheckUpdate_AllowChecks);
  ui->Font_PreferMonospaced->setChecked(m_Ctx.Config().Font_PreferMonospaced);

  ui->AlwaysReplayLocally->setChecked(m_Ctx.Config().AlwaysReplayLocally);

  ui->AllowGlobalHook->setChecked(m_Ctx.Config().AllowGlobalHook);

  ui->EventBrowser_TimeUnit->setCurrentIndex((int)m_Ctx.Config().EventBrowser_TimeUnit);
  ui->EventBrowser_AddFake->setChecked(m_Ctx.Config().EventBrowser_AddFake);
  ui->EventBrowser_HideEmpty->setChecked(m_Ctx.Config().EventBrowser_HideEmpty);
  ui->EventBrowser_HideAPICalls->setChecked(m_Ctx.Config().EventBrowser_HideAPICalls);
  ui->EventBrowser_ApplyColors->setChecked(m_Ctx.Config().EventBrowser_ApplyColors);
  ui->EventBrowser_ColorEventRow->setChecked(m_Ctx.Config().EventBrowser_ColorEventRow);

  // disable sub-checkbox
  ui->EventBrowser_ColorEventRow->setEnabled(ui->EventBrowser_ApplyColors->isChecked());

  ui->Comments_ShowOnLoad->setChecked(m_Ctx.Config().Comments_ShowOnLoad);

  ui->Formatter_MinFigures->setValue(m_Ctx.Config().Formatter_MinFigures);
  ui->Formatter_MaxFigures->setValue(m_Ctx.Config().Formatter_MaxFigures);
  ui->Formatter_NegExp->setValue(m_Ctx.Config().Formatter_NegExp);
  ui->Formatter_PosExp->setValue(m_Ctx.Config().Formatter_PosExp);

  if(!RENDERDOC_CanGlobalHook())
  {
    ui->AllowGlobalHook->setEnabled(false);

    QString disabledTooltip = tr("Global hooking is not supported on this platform");
    ui->AllowGlobalHook->setToolTip(disabledTooltip);
    ui->globalHookLabel->setToolTip(disabledTooltip);
  }

  m_Init = false;

  QObject::connect(ui->Formatter_MinFigures, OverloadedSlot<int>::of(&QSpinBox::valueChanged), this,
                   &SettingsDialog::formatter_valueChanged);
  QObject::connect(ui->Formatter_MaxFigures, OverloadedSlot<int>::of(&QSpinBox::valueChanged), this,
                   &SettingsDialog::formatter_valueChanged);
  QObject::connect(ui->Formatter_NegExp, OverloadedSlot<int>::of(&QSpinBox::valueChanged), this,
                   &SettingsDialog::formatter_valueChanged);
  QObject::connect(ui->Formatter_PosExp, OverloadedSlot<int>::of(&QSpinBox::valueChanged), this,
                   &SettingsDialog::formatter_valueChanged);
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

void SettingsDialog::on_pages_itemSelectionChanged()
{
  QList<QListWidgetItem *> sel = ui->pages->selectedItems();

  if(sel.empty())
  {
    ui->pages->setItemSelected(ui->pages->item(ui->tabWidget->currentIndex()), true);
  }
  else
  {
    ui->tabWidget->setCurrentIndex(ui->pages->row(sel[0]));
  }
}

void SettingsDialog::on_okButton_accepted()
{
  setResult(1);
  accept();
}

// general
void SettingsDialog::formatter_valueChanged(int val)
{
  m_Ctx.Config().Formatter_MinFigures = ui->Formatter_MinFigures->value();
  m_Ctx.Config().Formatter_MaxFigures = ui->Formatter_MaxFigures->value();
  m_Ctx.Config().Formatter_NegExp = ui->Formatter_NegExp->value();
  m_Ctx.Config().Formatter_PosExp = ui->Formatter_PosExp->value();

  m_Ctx.Config().SetupFormatting();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_tempDirectory_textEdited(const QString &dir)
{
  if(QDir(dir).exists())
    m_Ctx.Config().TemporaryCaptureDirectory = dir;
  else
    m_Ctx.Config().TemporaryCaptureDirectory = QString();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_saveDirectory_textEdited(const QString &dir)
{
  if(QDir(dir).exists() || dir.isEmpty())
    m_Ctx.Config().DefaultCaptureSaveDirectory = dir;

  m_Ctx.Config().Save();
}

void SettingsDialog::on_browseSaveCaptureDirectory_clicked()
{
  QString dir =
      RDDialog::getExistingDirectory(this, tr("Choose default directory for saving captures"),
                                     m_Ctx.Config().DefaultCaptureSaveDirectory);

  if(!dir.isEmpty())
  {
    m_Ctx.Config().DefaultCaptureSaveDirectory = dir;
    ui->saveDirectory->setText(dir);
  }

  m_Ctx.Config().Save();
}

void SettingsDialog::on_AllowGlobalHook_toggled(bool checked)
{
  m_Ctx.Config().AllowGlobalHook = ui->AllowGlobalHook->isChecked();

  m_Ctx.Config().Save();

  if(m_Ctx.HasCaptureDialog())
    m_Ctx.GetCaptureDialog()->UpdateGlobalHook();
}

void SettingsDialog::on_CheckUpdate_AllowChecks_toggled(bool checked)
{
  m_Ctx.Config().CheckUpdate_AllowChecks = ui->CheckUpdate_AllowChecks->isChecked();

  if(!m_Ctx.Config().CheckUpdate_AllowChecks)
  {
    m_Ctx.Config().CheckUpdate_UpdateAvailable = false;
    m_Ctx.Config().CheckUpdate_UpdateResponse = "";
  }

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Font_PreferMonospaced_toggled(bool checked)
{
  m_Ctx.Config().Font_PreferMonospaced = ui->Font_PreferMonospaced->isChecked();

  m_Ctx.Config().SetupFormatting();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_AlwaysReplayLocally_toggled(bool checked)
{
  m_Ctx.Config().AlwaysReplayLocally = ui->AlwaysReplayLocally->isChecked();

  m_Ctx.Config().Save();
}

// core
void SettingsDialog::on_chooseSearchPaths_clicked()
{
  QDialog listEditor;

  listEditor.setWindowTitle(tr("Shader debug info search paths"));
  listEditor.setWindowFlags(listEditor.windowFlags() & ~Qt::WindowContextHelpButtonHint);

  OrderedListEditor list(tr("Search Path"), BrowseMode::Folder);

  QVBoxLayout layout;
  QDialogButtonBox okCancel;
  okCancel.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  layout.addWidget(&list);
  layout.addWidget(&okCancel);

  QObject::connect(&okCancel, &QDialogButtonBox::accepted, &listEditor, &QDialog::accept);
  QObject::connect(&okCancel, &QDialogButtonBox::rejected, &listEditor, &QDialog::reject);

  listEditor.setLayout(&layout);

  QString setting = m_Ctx.Config().GetConfigSetting("shader.debug.searchPaths");

  list.setItems(setting.split(QLatin1Char(';'), QString::SkipEmptyParts));

  int res = RDDialog::show(&listEditor);

  if(res)
    m_Ctx.Config().SetConfigSetting(lit("shader.debug.searchPaths"),
                                    list.getItems().join(QLatin1Char(';')));
}

// texture viewer
void SettingsDialog::on_TextureViewer_PerTexSettings_toggled(bool checked)
{
  m_Ctx.Config().TextureViewer_PerTexSettings = ui->TextureViewer_PerTexSettings->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_TextureViewer_ResetRange_toggled(bool checked)
{
  m_Ctx.Config().TextureViewer_ResetRange = ui->TextureViewer_ResetRange->isChecked();

  m_Ctx.Config().Save();
}

// shader viewer
void SettingsDialog::on_ShaderViewer_FriendlyNaming_toggled(bool checked)
{
  m_Ctx.Config().ShaderViewer_FriendlyNaming = ui->ShaderViewer_FriendlyNaming->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_browseExtDisasemble_clicked()
{
  QString filePath = RDDialog::getExecutableFileName(this, tr("Locate SPIR-V disassembler"));

  if(!filePath.isEmpty())
  {
    ui->externalDisassemblePath->setText(filePath);
    on_externalDisassemblePath_textEdited(filePath);
  }
}

void SettingsDialog::on_externalDisassemblePath_textEdited(const QString &path)
{
  if(m_Ctx.Config().SPIRVDisassemblers.isEmpty())
  {
    m_Ctx.Config().SPIRVDisassemblers.push_back(SPIRVDisassembler());
    m_Ctx.Config().SPIRVDisassemblers.back().name = lit("Unknown");
  }

  m_Ctx.Config().SPIRVDisassemblers.back().executable = path;

  m_Ctx.Config().Save();
}

void SettingsDialog::on_externalDisassemblerArgs_textEdited(const QString &args)
{
  if(m_Ctx.Config().SPIRVDisassemblers.isEmpty())
  {
    m_Ctx.Config().SPIRVDisassemblers.push_back(SPIRVDisassembler());
    m_Ctx.Config().SPIRVDisassemblers.back().name = lit("Unknown");
  }

  m_Ctx.Config().SPIRVDisassemblers.back().args = args;

  m_Ctx.Config().Save();
}

// event browser
void SettingsDialog::on_EventBrowser_TimeUnit_currentIndexChanged(int index)
{
  if(m_Init)
    return;

  m_Ctx.Config().EventBrowser_TimeUnit = (TimeUnit)qMax(0, ui->EventBrowser_TimeUnit->currentIndex());

  if(m_Ctx.HasEventBrowser())
    m_Ctx.GetEventBrowser()->UpdateDurationColumn();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_EventBrowser_AddFake_toggled(bool checked)
{
  m_Ctx.Config().EventBrowser_AddFake = ui->EventBrowser_AddFake->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_EventBrowser_HideEmpty_toggled(bool checked)
{
  m_Ctx.Config().EventBrowser_HideEmpty = ui->EventBrowser_HideEmpty->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_EventBrowser_HideAPICalls_toggled(bool checked)
{
  m_Ctx.Config().EventBrowser_HideAPICalls = ui->EventBrowser_HideAPICalls->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_EventBrowser_ApplyColors_toggled(bool checked)
{
  m_Ctx.Config().EventBrowser_ApplyColors = ui->EventBrowser_ApplyColors->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_EventBrowser_ColorEventRow_toggled(bool checked)
{
  m_Ctx.Config().EventBrowser_ColorEventRow = ui->EventBrowser_ColorEventRow->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Comments_ShowOnLoad_toggled(bool checked)
{
  m_Ctx.Config().Comments_ShowOnLoad = ui->Comments_ShowOnLoad->isChecked();

  m_Ctx.Config().Save();
}

// android
void SettingsDialog::on_browseTempCaptureDirectory_clicked()
{
  QString dir = RDDialog::getExistingDirectory(this, tr("Choose directory for temporary captures"),
                                               m_Ctx.Config().TemporaryCaptureDirectory);

  if(!dir.isEmpty())
  {
    m_Ctx.Config().TemporaryCaptureDirectory = dir;
    ui->tempDirectory->setText(dir);
  }

  m_Ctx.Config().Save();
}

void SettingsDialog::on_browseAndroidSDKPath_clicked()
{
  QString adb = RDDialog::getExistingDirectory(
      this, tr("Locate SDK root folder (containing build-tools, platform-tools)"),
      QFileInfo(m_Ctx.Config().Android_SDKPath).absoluteDir().path());

  if(!adb.isEmpty())
  {
    ui->Android_SDKPath->setText(adb);
    m_Ctx.Config().Android_SDKPath = adb;
  }

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Android_SDKPath_textEdited(const QString &adb)
{
  if(QFileInfo::exists(adb) || adb.isEmpty())
    m_Ctx.Config().Android_SDKPath = adb;

  m_Ctx.Config().Save();
}

void SettingsDialog::on_browseAndroidJDKPath_clicked()
{
  QString adb =
      RDDialog::getExistingDirectory(this, tr("Locate JDK root folder (containing bin, jre, lib)"),
                                     QFileInfo(m_Ctx.Config().Android_JDKPath).absoluteDir().path());

  if(!adb.isEmpty())
  {
    ui->Android_JDKPath->setText(adb);
    m_Ctx.Config().Android_JDKPath = adb;
  }

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Android_JDKPath_textEdited(const QString &adb)
{
  if(QFileInfo::exists(adb) || adb.isEmpty())
    m_Ctx.Config().Android_JDKPath = adb;

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Android_MaxConnectTimeout_valueChanged(double timeout)
{
  m_Ctx.Config().Android_MaxConnectTimeout = ui->Android_MaxConnectTimeout->value();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_Android_AutoPushLayerToApp_toggled(bool checked)
{
  m_Ctx.Config().Android_AutoPushLayerToApp = ui->Android_AutoPushLayerToApp->isChecked();

  m_Ctx.Config().Save();
}

void SettingsDialog::on_UIStyle_currentIndexChanged(int index)
{
  if(index < 0 || index >= StyleData::numAvailable)
    return;

  // don't do anything until the dialog is initialised and visible
  if(!isVisible())
    return;

  QString oldStyle = m_Ctx.Config().UIStyle;
  QString newStyle = StyleData::availStyles[index].styleID;

  if(oldStyle == newStyle)
    return;

  QMessageBox::StandardButton ret = RDDialog::question(
      this, tr("Switch to new theme?"),
      tr("Would you like to switch to the new theme now?<br><br>Some parts of a theme might "
         "require a full application restart to properly apply."),
      RDDialog::YesNoCancel, QMessageBox::Yes);

  if(ret == QMessageBox::Cancel)
  {
    // change the index back. Since we haven't changed the style yet, this will early out above
    // instead of recursing.

    for(int i = 0; i < StyleData::numAvailable; i++)
    {
      if(StyleData::availStyles[i].styleID == oldStyle)
      {
        ui->UIStyle->setCurrentIndex(i);
        break;
      }
    }

    return;
  }

  // set the style but don't change anything unless the user selected yes.
  m_Ctx.Config().UIStyle = newStyle;

  if(ret == QMessageBox::Yes)
  {
    m_Ctx.Config().SetStyle();

    m_Ctx.Config().SetupFormatting();
  }

  m_Ctx.Config().Save();
}
