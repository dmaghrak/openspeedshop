////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////
  

#include "ManageCollectorsDialog.hxx"

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
#include <qinputdialog.h>

#include "SS_Input_Manager.hxx"

ManageCollectorsDialog::ManageCollectorsDialog( QWidget* parent, const char* name, bool modal, WFlags fl, int exp_id )
    : QDialog( parent, name, modal, fl )
{
  nprintf(DEBUG_CONST_DESTRUCT) ("ManageCollectorsDialog::ManageCollectorsDialog() constructor called.\n");
  
  mw = (OpenSpeedshop *)parent;
  cli = mw->cli;
  clo = NULL;
  expID = exp_id;
  if ( !name ) setName( "ManageCollectorsDialog" );

  setSizeGripEnabled( TRUE );
  ManageCollectorsDialogLayout = new QVBoxLayout( this, 11, 6, "ManageCollectorsDialogLayout"); 

  availableCollectorsLabel = new QLabel( this, "availableCollectorsLabel" );
  ManageCollectorsDialogLayout->addWidget( availableCollectorsLabel );
  availableCollectorsComboBox = new QComboBox( this, "availableCollectorsComboBox");
  availableCollectorsComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)0, 0, 0, availableCollectorsComboBox->sizePolicy().hasHeightForWidth() ) );
  availableCollectorsComboBox->setEditable(TRUE);
  ManageCollectorsDialogLayout->addWidget( availableCollectorsComboBox );

  attachCollectorsListView = new QListView( this, "attachCollectorsListView" );
  attachCollectorsListView->addColumn( 
    tr( QString("Collectors belonging to experiment: '%1':").arg(expID) ) );
  attachCollectorsListView->addColumn( tr( QString("Name") ) );
  attachCollectorsListView->setSelectionMode( QListView::Single );
  attachCollectorsListView->setAllColumnsShowFocus( TRUE );
  attachCollectorsListView->setShowSortIndicator( TRUE );
  attachCollectorsListView->setRootIsDecorated(TRUE);

  ManageCollectorsDialogLayout->addWidget( attachCollectorsListView );


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
  ManageCollectorsDialogLayout->addLayout( Layout1 );
  languageChange();
  resize( QSize(511, 282).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );

  // signals and slots connections
  connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( availableCollectorsComboBox, SIGNAL( activated(const QString &) ), this, SLOT( availableCollectorsComboBoxActivated() ) );

  connect(attachCollectorsListView, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint& , int ) ), this, SLOT( contextMenuRequested( QListViewItem *, const QPoint &, int ) ) );

  updateAttachedCollectorsList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ManageCollectorsDialog::~ManageCollectorsDialog()
{
  // no need to delete child widgets, Qt does it all for us
  nprintf(DEBUG_CONST_DESTRUCT) ("ManageCollectorsDialog::ManageCollectorsDialog() destructor called.\n");
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ManageCollectorsDialog::languageChange()
{
  setCaption( tr( "ManageCollectorsDialog" ) );
  buttonHelp->setText( tr( "&Help" ) );
  buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
  buttonOk->setText( tr( "&OK" ) );
  buttonOk->setAccel( QKeySequence( QString::null ) );
  buttonCancel->setText( tr( "&Cancel" ) );
  buttonCancel->setAccel( QKeySequence( QString::null ) );
  availableCollectorsLabel->setText( tr("Available Collectors:") );
  QString command;
  command = QString("listTypes all");
// printf("command=(%s)\n", command.ascii() );
  std::list<std::string> list_of_collectors;
  if( !cli->getStringListValueFromCLI( (char *)command.ascii(), 
         &list_of_collectors, FALSE ) )
  {
    printf("Unable to run %s command.\n", command.ascii() );
  }

  if( list_of_collectors.size() > 0 ) 
  {
    for( std::list<std::string>::const_iterator it = list_of_collectors.begin();         it != list_of_collectors.end(); it++ )
    {
      std::string collector_name = *it;
      availableCollectorsComboBox->insertItem( collector_name.c_str() );
    }
  }
}

QString
ManageCollectorsDialog::selectedCollectors()
{
  QListViewItem *selectedItem = attachCollectorsListView->selectedItem();
  if( selectedItem )
  {
    printf("Got an ITEM!\n");
    QString ret_value = selectedItem->text(0);
    return( ret_value );
  } else
  {
    printf("NO ITEMS SELECTED\n");
    return( NULL );
  }
}



void
ManageCollectorsDialog::updateAttachedCollectorsList()
{
  CollectorEntry *ce = NULL;
  char entry_buffer[1024];
  if( clo )
  {
    delete(clo);
  }

  clo = new CollectorListObject(expID);

  attachCollectorsListView->clear();
  CollectorEntryList::Iterator it;
  for( it = clo->collectorEntryList.begin();
       it != clo->collectorEntryList.end();
       ++it )
  {
    ce = (CollectorEntry *)*it;
    QListViewItem *item = new QListViewItem( attachCollectorsListView, ce->name, ce->short_name );
    for( CollectorParameterEntryList::Iterator pit = ce->paramList.begin();
         pit != ce->paramList.end();  pit++)
    {
      CollectorParameterEntry *cpe = (CollectorParameterEntry *)*pit;
      QListViewItem *item2 = new QListViewItem( item, cpe->name, cpe->param_value );
    }
  }
}

void ManageCollectorsDialog::availableCollectorsComboBoxActivated()
{
    updateAttachedCollectorsList();
}

void
ManageCollectorsDialog::contextMenuRequested( QListViewItem *item, const QPoint &pos, int col)
{
//  printf("ManagerCollectorsDialog::createPopupMenu() entered.\n");

  QPopupMenu *popupMenu = new QPopupMenu(this);
 
  if( attachCollectorsListView->selectedItem() )
  {
    if( attachCollectorsListView->selectedItem()->parent() == NULL ) // it's a root node.
    {
      popupMenu->insertItem("Detach...", this, SLOT(detachSelected()) );
      popupMenu->insertItem("Disable...", this, SLOT(disableSelected()) );
    } else
    {
      popupMenu->insertItem("Modify Parameter..", this, SLOT(modifySelected()) );
    }
  } else
  {
    popupMenu->insertItem("Nothing selected.", this, SLOT(nothingSelected()) );
  }


  popupMenu->popup( pos );
}


void
ManageCollectorsDialog::detachSelected()
{
  printf("detachSelected\n");
  QListViewItem *selectedItem = attachCollectorsListView->selectedItem();
  if( selectedItem )
  {
    printf("Got an ITEM!\n");
    QString ret_value = selectedItem->text(0);
    printf("detach = (%s)\n", ret_value.ascii() );

    QString collector_name = selectedItem->text(0);
    QString command;
    command = QString("expDetach -x %1 %2").arg(expID).arg(collector_name);
printf("command=(%s)\n", command.ascii() );
    if( !cli->runSynchronousCLI( (char *)command.ascii() ) )
    {
      printf("Unable to run %s command.\n", command.ascii() );
    }
  }
  updateAttachedCollectorsList();
}

void
ManageCollectorsDialog::disableSelected()
{
  printf("disableSelected\n");
  QListViewItem *selectedItem = attachCollectorsListView->selectedItem();
  if( selectedItem )
  {
    printf("Got an ITEM!\n");
    QString ret_value = selectedItem->text(0);
    printf("disable = (%s)\n", ret_value.ascii() );

    QString collector_name = selectedItem->text(0);
    QString command;
    command = QString("expDisable -x %1 %2").arg(expID).arg(collector_name);
printf("command=(%s)\n", command.ascii() );
    if( !cli->runSynchronousCLI( (char *)command.ascii() ) )
    {
      printf("Unable to run %s command.\n", command.ascii() );
    }

  }
  updateAttachedCollectorsList();
}

void
ManageCollectorsDialog::modifySelected()
{
  printf("modifySelected\n");
  QListViewItem *selectedItem = attachCollectorsListView->selectedItem();
  if( selectedItem )
  {
//    printf("Got an ITEM!\n");
    QString collector_name = selectedItem->parent()->text(0);
    QString param_name = selectedItem->text(0);
    QString param_value = selectedItem->text(1);
//printf("modify = parent()->text(0)=(%s)\n", collector_name.ascii());
//printf("modify = text(0)=(%s)\n", param_name.ascii());
//printf("modify = text(1)=(%s)\n", param_value.ascii());
// Modify the parameter....
    bool ok;
    int res = QInputDialog::getInteger(QString("Set %1 : %2").arg(collector_name).arg(param_name), QString("New Value:"), param_value.toUInt(), 0, 9999999, 10, &ok, this);
printf("res = %d\n", res);
    if( ok )
    {
      printf("user entered a value and pressed OK\n");
      QString command;
      command = QString("expSetParam -x %1 %2::%3=%4").arg(expID).arg(collector_name).arg(param_name).arg(res);
printf("command=(%s)\n", command.ascii() );
      if( !cli->runSynchronousCLI( (char *)command.ascii() ) )
      {
        printf("Unable to run %s command.\n", command.ascii() );
      }
    } else
    {
      printf("user pressed cancel.\n");
    }
    
  }
  updateAttachedCollectorsList();
}

void
ManageCollectorsDialog::nothingSelected()
{
  printf("nothingSelected\n");
}
