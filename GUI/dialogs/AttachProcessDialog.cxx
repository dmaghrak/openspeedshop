#include "AttachProcessDialog.hxx"

#include "debug.hxx"

#include <qvariant.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlistview.h>

AttachProcessDialog::AttachProcessDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
  nprintf(DEBUG_CONST_DESTRUCT) ("AttachProcessDialog::AttachProcessDialog() constructor called.\n");
  
  plo = NULL;
  if ( !name ) setName( "AttachProcessDialog" );

  setSizeGripEnabled( TRUE );
  AttachProcessDialogLayout = new QVBoxLayout( this, 11, 6, "AttachProcessDialogLayout"); 

  attachHostLabel = new QLabel( this, "attachHostLabel" );
  AttachProcessDialogLayout->addWidget( attachHostLabel );
  attachHostComboBox = new QComboBox( this, "attachHostComboBox");
  attachHostComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)0, 0, 0, attachHostComboBox->sizePolicy().hasHeightForWidth() ) );
  attachHostComboBox->setEditable(TRUE);
  AttachProcessDialogLayout->addWidget( attachHostComboBox );

  availableProcessListView = new QListView( this, "availableProcessListView" );
  availableProcessListView->addColumn( tr( "Processes belonging to '%s':" ) );
  availableProcessListView->setSelectionMode( QListView::Single );
  availableProcessListView->setAllColumnsShowFocus( FALSE );
  availableProcessListView->setShowSortIndicator( FALSE );
  AttachProcessDialogLayout->addWidget( availableProcessListView );


  Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

  buttonHelp = new QPushButton( this, "buttonHelp" );
  buttonHelp->setAutoDefault( TRUE );
  Layout1->addWidget( buttonHelp );
  Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout1->addItem( Horizontal_Spacing2 );

  buttonOk = new QPushButton( this, "buttonOk" );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  Layout1->addWidget( buttonOk );

  buttonCancel = new QPushButton( this, "buttonCancel" );
  buttonCancel->setAutoDefault( TRUE );
  Layout1->addWidget( buttonCancel );
  AttachProcessDialogLayout->addLayout( Layout1 );
  languageChange();
  resize( QSize(511, 282).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );

  // signals and slots connections
  connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( attachHostComboBox, SIGNAL( activated(const QString &) ), this, SLOT( attachHostComboBoxActivated() ) );

  updateAttachableProcessList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
AttachProcessDialog::~AttachProcessDialog()
{
  // no need to delete child widgets, Qt does it all for us
  nprintf(DEBUG_CONST_DESTRUCT) ("AttachProcessDialog::AttachProcessDialog() destructor called.\n");
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AttachProcessDialog::languageChange()
{
  setCaption( tr( "AttachProcessDialog" ) );
  buttonHelp->setText( tr( "&Help" ) );
  buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
  buttonOk->setText( tr( "&OK" ) );
  buttonOk->setAccel( QKeySequence( QString::null ) );
  buttonCancel->setText( tr( "&Cancel" ) );
  buttonCancel->setAccel( QKeySequence( QString::null ) );
  attachHostLabel->setText( tr("What be your host?") );
  attachHostComboBox->insertItem( "localhost" );
  attachHostComboBox->insertItem( "clink.americas.sgi.com" );
  attachHostComboBox->insertItem( "hope.americas.sgi.com" );
  attachHostComboBox->insertItem( "hope1.americas.sgi.com" );
  attachHostComboBox->insertItem( "hope2.americas.sgi.com" );
}

QString
AttachProcessDialog::selectedProcesses()
{
//  QString ret_value = attachHostComboBox->currentText();

  QListViewItem *selectedItem = availableProcessListView->selectedItem();
  if( selectedItem )
  {
//    printf("Got an ITEM!\n");
    QString ret_value = selectedItem->text(0);
    return( ret_value );
  } else
  {
//    printf("NO ITEMS SELECTED\n");
    return( NULL );
  }
}



void
AttachProcessDialog::updateAttachableProcessList()
{
  char *host = (char *)attachHostComboBox->currentText().ascii();
  ProcessEntry *pe = NULL;
  char entry_buffer[1024];

  if( plo )
  {
    delete(plo);
  }
//  printf("look up processes on host=(%s)\n", host);
  plo = new ProcessListObject(host);

  availableProcessListView->clear();
  QListViewItem *item_2 = new QListViewItem( availableProcessListView, 0 );
  item_2->setOpen( TRUE );
  item_2->setText( 0, tr( "hostname" ) );

  ProcessEntryList::Iterator it;
  for( it = plo->processEntryList.begin();
       it != plo->processEntryList.end();
       ++it )
  {
    pe = (ProcessEntry *)*it;
//    printf("%-20s %-10d %-20s\n", pe->host_name, pe->pid, pe->process_name);
    sprintf(entry_buffer, "%-20s %-10d %-20s\n", pe->host_name, pe->pid, pe->process_name);
    QListViewItem *item = new QListViewItem( item_2, 0 );
    item->setText( 0, tr(entry_buffer) );
  }
}

void AttachProcessDialog::attachHostComboBoxActivated()
{
//  printf("attachHostComboBoxActivated\n");
    updateAttachableProcessList();
}
