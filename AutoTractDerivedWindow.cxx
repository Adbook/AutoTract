#include "AutoTractDerivedWindow.h"
#include <iostream>

/* Management of software tab: https://github.com/marie-cherel2013/NeosegPipeline*/

AutoTractDerivedWindow::AutoTractDerivedWindow()
{
    connect( this->actionSave_Parameter_Configuration, SIGNAL( triggered() ), SLOT( SaveParaConfigFile() ) );
    connect( this->actionLoad_Parameter_Configuration, SIGNAL( triggered() ), SLOT( LoadParaConfigFile() ) );
    connect( this->actionSave_Software_Configuration, SIGNAL( triggered() ), SLOT( SaveSoftConfigFile() ) );
    connect( this->actionLoad_Software_Configuration, SIGNAL( triggered() ), SLOT( LoadSoftConfigFile() ) );

    /*1st tab*/
    // Select Parameters Signal Mapper
    QSignalMapper* selectParameters_signalMapper = new QSignalMapper(this);
    connect(selectParameters_signalMapper, SIGNAL(mapped(QString)), this, SLOT(selectParameters(QString)));

    // Enter Parameters Signal Mapper
    QSignalMapper* enterParameters_signalMapper = new QSignalMapper(this);
    connect(enterParameters_signalMapper, SIGNAL(mapped(QString)), this, SLOT(enterParameters(QString)));

    initializeParametersMap();
    QMap<QString, Parameters>::iterator it_parameters;
    for(it_parameters = m_parameters_map.begin(); it_parameters != m_parameters_map.end(); ++it_parameters)
    {
        QString name = it_parameters.key();
        Parameters parameters = it_parameters.value();

        selectParameters_signalMapper->setMapping(parameters.select_button, name);
        connect(parameters.select_button, SIGNAL(clicked()), selectParameters_signalMapper, SLOT(map()));

        enterParameters_signalMapper->setMapping(parameters.enter_lineEdit, name);
        connect(parameters.enter_lineEdit, SIGNAL(editingFinished()), enterParameters_signalMapper, SLOT(map()));
    }
    connect(output_dir_pushButton, SIGNAL(clicked()), this, SLOT(selectOutputDirectory()));
    connect(para_output_dir_lineEdit, SIGNAL(editingFinished()), this, SLOT(enterOutputDirectory()));

    /*2nd tab*/
    // Select Executable Signal Mapper
    QSignalMapper* selectExecutable_signalMapper = new QSignalMapper(this);
    connect(selectExecutable_signalMapper, SIGNAL(mapped(QString)), this, SLOT(selectExecutable(QString)));

    // Enter Executable Signal Mapper
    QSignalMapper* enterExecutable_signalMapper = new QSignalMapper(this);
    connect(enterExecutable_signalMapper, SIGNAL(mapped(QString)), this, SLOT(enterExecutable(QString)));

    // Reset Executable Signal Mapper
    QSignalMapper* resetExecutable_signalMapper = new QSignalMapper(this);
    connect(resetExecutable_signalMapper, SIGNAL(mapped(QString)), this, SLOT(resetExecutable(QString)));

    initializeExecutablesMap();
    QMap<QString, Executable>::iterator it_exec;
    for(it_exec = m_executables_map.begin(); it_exec != m_executables_map.end(); ++it_exec)
    {
        QString name = it_exec.key();
        Executable executable = it_exec.value();

        selectExecutable_signalMapper->setMapping(executable.select_button, name);
        connect(executable.select_button, SIGNAL(clicked()), selectExecutable_signalMapper, SLOT(map()));

        enterExecutable_signalMapper->setMapping(executable.enter_lineEdit, name);
        connect(executable.enter_lineEdit, SIGNAL(editingFinished()), enterExecutable_signalMapper, SLOT(map()));

        resetExecutable_signalMapper->setMapping(executable.reset_button, name);
        connect(executable.reset_button, SIGNAL(clicked()), resetExecutable_signalMapper, SLOT(map()));
    }

    /*4th tab: Registration*/
    connect(para_registration_type_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeRegistrationType(int)));

    /*4th tab*/
    connect(tracts_dir_pushButton, SIGNAL(clicked()), this, SLOT(selectTractsPopulationDirectory()));
    connect(para_tracts_dir_lineEdit, SIGNAL(editingFinished()), this, SLOT(enterAtlasPopulationDirectory()));
    connect(para_ref_tracts_listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(selectAtlas(QListWidgetItem*)));

    /*7th tab: Execution*/
    connect(runPipeline_pushButton, SIGNAL(clicked()), this, SLOT(runPipeline()));
    connect(stopPipeline_pushButton, SIGNAL(clicked()), this, SLOT(stopPipeline()));
}

void AutoTractDerivedWindow::changeRegistrationType(int index)
{

}

void AutoTractDerivedWindow::runPipeline()
{
    SyncUiToModelStructure();
    m_pipeline = new Pipeline();
    m_pipeline->setPipelineParameters(m_para_m);
    m_pipeline->setPipelineSoftwares(m_soft_m);
    m_pipeline->SetExecutablesMap( m_lookup_executables_map );
    m_pipeline->SetParametersMap( m_lookup_parameters_map );
    initializePipelineLogging();
    m_pipeline->writePipeline();


    m_thread = new MainScriptThread();
    m_thread->setPipeline(m_pipeline);
    m_thread->start();
}

void AutoTractDerivedWindow::stopPipeline()
{
    m_thread->terminate();

    runPipeline_pushButton->setEnabled(true);
    runPipeline_action->setEnabled(true);

    stopPipeline_pushButton->setEnabled(false);
    stopPipeline_action->setEnabled(false);

    //runPipeline_progressBar->hide();
}

void AutoTractDerivedWindow::initializePipelineLogging()
{
    log_textEdit->show();
    log_textEdit->clear();
    log_textEdit->setMaximumBlockCount(500);

    // Log path
    QDir* output_dir = new QDir(m_para_m->getpara_output_dir_lineEdit());
    QFileInfo fi(m_para_m->getpara_output_dir_lineEdit());
    QString base = fi.baseName();
    QString log_path = output_dir->filePath(base + ".log");

    // Log File
    QFile* log_file = new::QFile(log_path);
    log_file->open(QIODevice::ReadWrite);
    m_log_textStream = new::QTextStream(log_file);

    // QFileSystemWatcher
    QFileSystemWatcher* log_watcher = new::QFileSystemWatcher(this);
    log_watcher->addPath(log_path);
    connect(log_watcher, SIGNAL(fileChanged(QString)), this, SLOT(printPipelineLog()));
}

void AutoTractDerivedWindow::printPipelineLog()
{
    QScrollBar *scrollBar = log_textEdit->verticalScrollBar();

    QString line = m_log_textStream->readAll();

    if(scrollBar->value() == scrollBar->maximum())
    {
        log_textEdit->insertPlainText(line);
        scrollBar->setValue(scrollBar->maximum());
    }
    else
    {
        log_textEdit->insertPlainText(line);
    }
}

void AutoTractDerivedWindow::SaveParaConfigFile()
{
    QString m_DialogDir;
    QString filename = QFileDialog::getSaveFileName( this , "Save Parameter Configuration File" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        Save_Parameter_Configuration( filename.toStdString() );
    }

}

void AutoTractDerivedWindow::LoadParaConfigFile()
{
    QString m_DialogDir;
    QString filename = QFileDialog::getOpenFileName( this , "Open Parameter Configuration File" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        Load_Parameter_Configuration( filename.toStdString() );
    }
}

void AutoTractDerivedWindow::SaveSoftConfigFile()
{
    QString m_DialogDir;
    QString filename = QFileDialog::getSaveFileName( this , "Save Software Configuration File" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        Save_Software_Configuration( filename.toStdString() );
    }

}

void AutoTractDerivedWindow::LoadSoftConfigFile()
{
    QString m_DialogDir;
    QString filename = QFileDialog::getOpenFileName( this , "Open Software Configuration File" , m_DialogDir , "XML files (*.xml)" );
    if( filename != "" )
    {
        QFileInfo fi( filename ) ;
        m_DialogDir = fi.dir().absolutePath() ;
        Load_Software_Configuration( filename.toStdString() );
    }

}

void AutoTractDerivedWindow::initializeExecutablesMap()
{
    Executable DTIReg = {DTIReg_pushButton, soft_DTIReg_lineEdit, reset_DTIReg_pushButton};
    m_executables_map.insert("DTIReg", DTIReg);

    Executable fiberprocess = {fiberprocess_pushButton, soft_fiberprocess_lineEdit, reset_fiberprocess_pushButton};
    m_executables_map.insert("fiberprocess", fiberprocess);

    Executable ResampleDTIVolume = {ResampleDTIVolume_pushButton, soft_ResampleDTIVolume_lineEdit, reset_ResampleDTI_Volume_pushButton};
    m_executables_map.insert("ResampleDTIVolume", ResampleDTIVolume);

    Executable ImageMath = {ImageMath_pushButton, soft_ImageMath_lineEdit, reset_ImageMath_pushButton};
    m_executables_map.insert("ImageMath", ImageMath);

    Executable TractographyLabelMapSeeding = {TractographyLabelMapSeeding_pushButton, soft_TractographyLabelMapSeeding_lineEdit, reset_TractographyLabelMapSeeding_pushButton};
    m_executables_map.insert("TractographyLabelMapSeeding", TractographyLabelMapSeeding);

    Executable FiberPostProcess = {FiberPostProcess_pushButton, soft_FiberPostProcess_lineEdit, reset_FiberPostProcess_pushButton};
    m_executables_map.insert("FiberPostProcess", FiberPostProcess);

    Executable polydatatransform = {polydatattransform_pushButton, soft_polydatatransform_lineEdit, reset_polydatatransform_pushButton};
    m_executables_map.insert("polydatatransform", polydatatransform);

    Executable unu = {unu_pushButton, soft_unu_lineEdit, reset_unu_pushButton};
    m_executables_map.insert("unu", unu);

    Executable MDT = {MDT_pushButton, soft_MDT_lineEdit, reset_MDT_pushButton};
    m_executables_map.insert("MDT", MDT);

    Executable python = {python_pushButton, soft_python_lineEdit, reset_python_pushButton};
    m_executables_map.insert("python", python);

    Executable dtiprocess = {dtiprocess_pushButton, soft_dtiprocess_lineEdit, reset_dtiprocess_pushButton};
    m_executables_map.insert("dtiprocess", dtiprocess);
}

void AutoTractDerivedWindow::SetLookupExecutableMap(QMap<QString, QString> lookup_executables_map)
{
    QMap<QString, QString>::iterator i;
    for (i = lookup_executables_map.begin(); i != lookup_executables_map.end(); ++i)
    {
        m_lookup_executables_map[i.key()] =  i.value() ;
    }
}
void AutoTractDerivedWindow::SetLookupParameterMap(QMap<QString, QString> lookup_parameters_map)
{
    QMap<QString, QString>::iterator i;
    for (i = lookup_parameters_map.begin(); i != lookup_parameters_map.end(); ++i)
    {
        m_lookup_parameters_map[i.key()] =  i.value() ;
    }
}

void AutoTractDerivedWindow::selectExecutable(QString executable_name)
{
    Executable executable = m_executables_map[executable_name];
    QString executable_path = (executable.enter_lineEdit)->text();
    QString dir_path = "";

    if(!(executable_path.isEmpty()))
    {
        dir_path = (QFileInfo(executable_path).dir()).absolutePath();
    }

    executable_path = QFileDialog::getOpenFileName(this, tr("Select executable"), dir_path);
    if(!executable_path.isEmpty())
    {
        (executable.enter_lineEdit)->setText(executable_path) ;
    }
}

void AutoTractDerivedWindow::enterExecutable(QString executable_name)
{
    Executable executable = m_executables_map[executable_name];
    QString executable_path = (executable.enter_lineEdit)->text();
    (executable.enter_lineEdit)->setText(executable_path) ;
}

void AutoTractDerivedWindow::resetExecutable(QString executable_name)
{
    Executable executable = m_executables_map[executable_name];
    (executable.enter_lineEdit)->setText(m_lookup_executables_map[executable_name]);
}

void AutoTractDerivedWindow::initializeParametersMap()
{
    Parameters inputDTIatlas_dir = { inputDTIatlas_pushButton, para_inputDTIatlas_lineEdit};
    m_parameters_map.insert("inputDTIatlas_dir", inputDTIatlas_dir);

    Parameters inputWMmask_dir = {inputWMmask_pushButton, para_inputWMmask_lineEdit};
    m_parameters_map.insert("inputWMmask_dir", inputWMmask_dir);

    Parameters inputCSFmask_dir = {inputCSFmask_pushButton, para_inputCSFmask_lineEdit};
    m_parameters_map.insert("inputCSFmask_dir", inputCSFmask_dir);

    Parameters refDTIatlas_dir = {refDTIatlas_pushButton, para_refDTIatlas_lineEdit};
    m_parameters_map.insert("refDTIatlas_dir", refDTIatlas_dir);

    Parameters tracts_dir = {tracts_dir_pushButton, para_tracts_dir_lineEdit};
    m_parameters_map.insert("tracts_dir", tracts_dir);
}

void AutoTractDerivedWindow::selectParameters(QString parameters_name)
{
    Parameters parameters = m_parameters_map[parameters_name];
    QString parameters_path = (parameters.enter_lineEdit)->text();
    QString dir_path = "";

    if(!(parameters_path.isEmpty()))
    {
        dir_path = (QFileInfo(parameters_path).dir()).absolutePath();
    }

    parameters_path = QFileDialog::getOpenFileName(this, tr("Select path"), dir_path);
    if(!parameters_path.isEmpty())
    {
        (parameters.enter_lineEdit)->setText(parameters_path) ;
    }
}

void AutoTractDerivedWindow::enterParameters(QString parameters_name)
{
    Parameters parameters = m_parameters_map[parameters_name];
    QString parameters_path = (parameters.enter_lineEdit)->text();
    (parameters.enter_lineEdit)->setText(parameters_path) ;
}

void AutoTractDerivedWindow::selectTractsPopulationDirectory()
{
    QString tractsPopulationDirectory = QFileDialog::getExistingDirectory (this, tr("Open Directory"), para_tracts_dir_lineEdit->text(), QFileDialog::ShowDirsOnly);
    para_tracts_dir_lineEdit->setText(tractsPopulationDirectory);
    m_atlasPopulationDirectory = tractsPopulationDirectory;
    UpdateAtlasPopulationDirectoryDisplay() ;
}

//***** Select/Unselect Atlas *****//
void AutoTractDerivedWindow::selectAtlas(QListWidgetItem* item)
{
    if(item->checkState())
    {
        m_selectedAtlases << item->text();
    }
    else
    {
        m_selectedAtlases.removeAt(m_selectedAtlases.indexOf(item->text()));
    }
}

void AutoTractDerivedWindow::UpdateAtlasPopulationDirectoryDisplay()
{
    checkAtlases() ;
    displayAtlases() ;
    checkSelectedAtlases() ;
}
//***** Display Atlases *****//
void AutoTractDerivedWindow::displayAtlases()
{
    para_ref_tracts_listWidget->clear();
    QStringList::const_iterator it;
    int count = 0;
    for (it = m_selectedAtlases.constBegin(); it != m_selectedAtlases.constEnd(); ++it)
    {
        QListWidgetItem* item = new QListWidgetItem(*it, para_ref_tracts_listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        item->setCheckState(Qt::Unchecked);count++;
    }
}

void AutoTractDerivedWindow::enterAtlasPopulationDirectory()
{
    QString atlasPopulationDirectory = para_tracts_dir_lineEdit->text();
    if(!atlasPopulationDirectory.isEmpty())
    {
        m_atlasPopulationDirectory =  para_tracts_dir_lineEdit->text() ;
    }
    UpdateAtlasPopulationDirectoryDisplay() ;
}

void AutoTractDerivedWindow::checkSelectedAtlases()
{
    QStringList::const_iterator it;
    for (it = m_selectedAtlases.constBegin(); it != m_selectedAtlases.constEnd(); ++it)
    {
        QList<QListWidgetItem *> items = para_ref_tracts_listWidget->findItems(*it, Qt::MatchExactly);

        QList<QListWidgetItem *>::iterator item;
        for(item=items.begin(); item!=items.end(); ++item)
        {
            (*item)->setCheckState(Qt::Checked);
        }
    }
}

//***** Checking Atlases *****//
void AutoTractDerivedWindow::checkAtlases()
{
    QDir* m_atlasPopulation_dir = new QDir(para_tracts_dir_lineEdit ->text());
    QStringList atlasPopulation_list = m_atlasPopulation_dir->entryList( QDir::Files | QDir::NoSymLinks);

    QStringList::const_iterator it;
    for (it = atlasPopulation_list.constBegin(); it != atlasPopulation_list.constEnd(); ++it)
    {
        if((*it).endsWith(".vtk") || (*it).endsWith(".vtp"))
        {
            m_selectedAtlases << *it;
        }
    }
}

void AutoTractDerivedWindow::selectOutputDirectory()
{
    QString outputDirectory = QFileDialog::getExistingDirectory (this, tr("Open Directory"), para_output_dir_lineEdit->text(), QFileDialog::ShowDirsOnly);
    para_output_dir_lineEdit->setText(outputDirectory);
    m_para_m->setpara_output_dir_lineEdit( outputDirectory );
}

void AutoTractDerivedWindow::enterOutputDirectory()
{
    QString outputDirectory = para_output_dir_lineEdit->text();
    if(!outputDirectory.isEmpty())
    {
        m_para_m->setpara_output_dir_lineEdit( para_output_dir_lineEdit->text() ) ;
    }
}
