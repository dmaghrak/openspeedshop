////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
// Copyright (c) 2006, 2007 Krell Institute All Rights Reserved.
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

// Debug Flag
//#define DEBUG_PCWizard 1
//

#include <stdio.h>
#include "pcSampleWizardPanel.hxx"
#include "PanelContainer.hxx"   // Do not remove
#include "plugin_entry_point.hxx"   // Do not remove
#include "AttachProcessDialog.hxx"
#include "AttachMPProcessDialog.hxx"
#include <qapplication.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfile.h>   // For the file dialog box.
#include <qfiledialog.h>  // For the file dialog box.
#include <qmessagebox.h>
#include <qscrollview.h>
#include <qvbox.h>

#include <qbitmap.h>
#include "rightarrow.xpm"
#include "leftarrow.xpm"

#include "LoadAttachObject.hxx"

#include "pcSampleDescription.hxx"

#include "SS_Input_Manager.hxx"
using namespace OpenSpeedShop::Framework;

pcSampleWizardPanel::pcSampleWizardPanel(PanelContainer *pc, const char *n, ArgumentObject *ao) : Panel(pc, n)
{
  nprintf(DEBUG_CONST_DESTRUCT) ("pcSampleWizardPanel::pcSampleWizardPanel() constructor called\n");
  if ( !getName() )
  {
	setName( "pc Sample" );
  }

  // Clear out the containers for executables and pids
  if( getPanelContainer()->getMainWindow() )
  {
    OpenSpeedshop *mw = getPanelContainer()->getMainWindow();
    if( mw )
    {
      mw->executableName = QString::null;
      mw->pidStr = QString::null;
      mw->parallelPrefixCommandStr = QString::null;
    }
  }

  pcSampleFormLayout = new QVBoxLayout( getBaseWidgetFrame(), 1, 2, getName() );

  mainFrame = new QFrame( getBaseWidgetFrame(), "mainFrame" );
  mainFrame->setMinimumSize( QSize(10,10) );
  mainFrame->setFrameShape( QFrame::StyledPanel );
  mainFrame->setFrameShadow( QFrame::Raised );
  mainFrameLayout = new QVBoxLayout( mainFrame, 11, 6, "mainFrameLayout"); 

  mainWidgetStack = new QWidgetStack( mainFrame, "mainWidgetStack" );
  mainWidgetStack->setMinimumSize( QSize(10,10) );

// Begin: verbose description page
  vDescriptionPageWidget = new QWidget( mainWidgetStack, "vDescriptionPageWidget" );
  vDescriptionPageLayout = new QVBoxLayout( vDescriptionPageWidget, 11, 6, "vDescriptionPageLayout"); 

  vDescriptionPageTitleLabel = new QLabel( vDescriptionPageWidget, "vDescriptionPageTitleLabel" );
  vDescriptionPageTitleLabel->setMinimumSize( QSize(0,0) );
  vDescriptionPageTitleLabel->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed, 0, 0, FALSE ) );
  vDescriptionPageTitleLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
  vDescriptionPageLayout->addWidget( vDescriptionPageTitleLabel );

  vDescriptionPageText = new QTextEdit( vDescriptionPageWidget, "vDescriptionPageText" );
  vDescriptionPageText->setReadOnly(TRUE);
  vDescriptionPageText->setMinimumSize( QSize(10,10) );
  vDescriptionPageText->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, 0, 0, FALSE ) );
  vDescriptionPageText->setMinimumSize( QSize(10,10) );
  vDescriptionPageText->setWordWrap( QTextEdit::WidgetWidth );
  vDescriptionPageLayout->addWidget( vDescriptionPageText );

  vDescriptionPageButtonLayout = new QHBoxLayout( 0, 0, 6, "vDescriptionPageButtonLayout"); 

  vwizardMode = new QCheckBox( vDescriptionPageWidget, "vwizardMode" );
  vwizardMode->setMinimumSize( QSize(10,10) );
  vwizardMode->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  vwizardMode->setChecked( TRUE );
  vDescriptionPageButtonLayout->addWidget( vwizardMode );

  vDescriptionPageButtonSpacer = new QSpacerItem( 1, 10, QSizePolicy::Expanding, QSizePolicy::Fixed );
  vDescriptionPageButtonLayout->addItem( vDescriptionPageButtonSpacer );

  vDescriptionPageIntroButton = new QPushButton( vDescriptionPageWidget, "vDescriptionPageIntroButton" );
  vDescriptionPageIntroButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vDescriptionPageIntroButton->sizePolicy().hasHeightForWidth() ) );
  vDescriptionPageButtonLayout->addWidget( vDescriptionPageIntroButton );

  vDescriptionPageNextButton = new QPushButton( vDescriptionPageWidget, "vDescriptionPageNextButton" );
  vDescriptionPageNextButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vDescriptionPageNextButton->sizePolicy().hasHeightForWidth() ) );
  vDescriptionPageButtonLayout->addWidget( vDescriptionPageNextButton );

  vDescriptionPageFinishButton = new QPushButton( vDescriptionPageWidget, "vDescriptionPageFinishButton" );
  vDescriptionPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vDescriptionPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  vDescriptionPageButtonLayout->addWidget( vDescriptionPageFinishButton );

  vDescriptionPageLayout->addLayout( vDescriptionPageButtonLayout );
  mainWidgetStack->addWidget( vDescriptionPageWidget, 0 );
// End: verbose description page

// Begin: verbose parameter page
  vParameterPageWidget = new QWidget( mainWidgetStack, "vParameterPageWidget" );
  vParameterPageWidget->setMinimumSize( QSize(10,10) );
  vParameterPageLayout = new QVBoxLayout( vParameterPageWidget, 11, 6, "vParameterPageLayout"); 

  vParameterPageDescriptionText = new QTextEdit( vParameterPageWidget, "vParameterPageDescriptionText" );
  vParameterPageDescriptionText->setReadOnly(TRUE);
  vParameterPageDescriptionText->setMinimumSize( QSize(10,10) );

  vParameterPageDescriptionText->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, 0, 0, FALSE ) );
  vParameterPageDescriptionText->setMinimumSize( QSize(10,10) );
  vParameterPageDescriptionText->setWordWrap( QTextEdit::WidgetWidth );
  vParameterPageLayout->addWidget( vParameterPageDescriptionText );


  vParameterPageLine = new QFrame( vParameterPageWidget, "vParameterPageLine" );
  vParameterPageLine->setMinimumSize( QSize(10,10) );
  vParameterPageLine->setFrameShape( QFrame::HLine );
  vParameterPageLine->setFrameShadow( QFrame::Sunken );
  vParameterPageLine->setFrameShape( QFrame::HLine );
  vParameterPageLayout->addWidget( vParameterPageLine );

  vParameterPageParameterLayout = new QVBoxLayout( 0, 0, 6, "vParameterPageParameterLayout"); 

  vParameterPageSampleRateHeaderLabel = new QLabel( vParameterPageWidget, "vParameterPageSampleRateHeaderLabel" );
//  vParameterPageSampleRateHeaderLabel->setMinimumSize( QSize(10,10) );
  vParameterPageSampleRateHeaderLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed, 0, 0, FALSE ) );
  vParameterPageParameterLayout->addWidget( vParameterPageSampleRateHeaderLabel );

  vParameterPageSampleRateLayout = new QHBoxLayout( 0, 0, 6, "vParameterPageSampleRateLayout"); 

  vParameterPageSampleRateLabel = new QLabel( vParameterPageWidget, "vParameterPageSampleRateLabel" );
  vParameterPageSampleRateLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  vParameterPageSampleRateLayout->addWidget( vParameterPageSampleRateLabel );

  vParameterPageSampleRateText = new QLineEdit( vParameterPageWidget, "vParameterPageSampleRateText" );
  vParameterPageSampleRateText->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  vParameterPageSampleRateLayout->addWidget( vParameterPageSampleRateText );

  vParameterPageSpacer = new QSpacerItem( 400, 30, QSizePolicy::Preferred, QSizePolicy::Fixed );
  vParameterPageSampleRateLayout->addItem( vParameterPageSpacer );

  vParameterPageParameterLayout->addLayout( vParameterPageSampleRateLayout );
  vParameterPageLayout->addLayout( vParameterPageParameterLayout );

  vParameterPageButtonLayout = new QHBoxLayout( 0, 0, 6, "vParameterPageButtonLayout"); 

  vParameterPageButtonSpacer = new QSpacerItem( 251, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  vParameterPageButtonLayout->addItem( vParameterPageButtonSpacer );

  vParameterPageResetButton = new QPushButton( vParameterPageWidget, "vParameterPageResetButton" );
  vParameterPageResetButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vParameterPageResetButton->sizePolicy().hasHeightForWidth() ) );
  vParameterPageButtonLayout->addWidget( vParameterPageResetButton );

  vParameterPageBackButton = new QPushButton( vParameterPageWidget, "vParameterPageBackButton" );
  vParameterPageBackButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vParameterPageBackButton->sizePolicy().hasHeightForWidth() ) );
  vParameterPageButtonLayout->addWidget( vParameterPageBackButton );
  vParameterPageNextButton = new QPushButton( vParameterPageWidget, "vParameterPageNextButton" );
  vParameterPageNextButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vParameterPageNextButton->sizePolicy().hasHeightForWidth() ) );
  vParameterPageButtonLayout->addWidget( vParameterPageNextButton );

  vParameterPageFinishButton = new QPushButton( vParameterPageWidget, "vParameterPageFinishButton" );
  vParameterPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vParameterPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  vParameterPageButtonLayout->addWidget( vParameterPageFinishButton );

  vParameterPageLayout->addLayout( vParameterPageButtonLayout );
  mainWidgetStack->addWidget( vParameterPageWidget, 1 );
// End: verbose parameter page

// Begin: verbose summary page
  vSummaryPageWidget = new QWidget( mainWidgetStack, "vSummaryPageWidget" );
  vSummaryPageWidget->setMinimumSize( QSize(10,10) );
  vSummaryPageLayout = new QVBoxLayout( vSummaryPageWidget, 11, 6, "vSummaryPageLayout"); 

  vSummaryPageLabelLayout = new QVBoxLayout( 0, 0, 6, "vSummaryPageLabelLayout"); 
  vSummaryPageFinishLabel = new QTextEdit( vSummaryPageWidget, "vSummaryPageFinishLabel" );
  vSummaryPageFinishLabel->setReadOnly(TRUE);
  vSummaryPageFinishLabel->setMinimumSize( QSize(10,10) );
  vSummaryPageFinishLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred, 0, 0, FALSE ) );
  vSummaryPageLabelLayout->addWidget( vSummaryPageFinishLabel );
  vSummaryPageLayout->addLayout( vSummaryPageLabelLayout );

  vSummaryPageButtonLayout = new QHBoxLayout( 0, 0, 6, "vSummaryPageButtonLayout"); 

  vSummaryPageButtonSpacer = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  vSummaryPageButtonLayout->addItem( vSummaryPageButtonSpacer );
  vSummaryPageBackButton = new QPushButton( vSummaryPageWidget, "vSummaryPageBackButton" );
  vSummaryPageBackButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vSummaryPageBackButton->sizePolicy().hasHeightForWidth() ) );
  vSummaryPageButtonLayout->addWidget( vSummaryPageBackButton );

  vSummaryPageFinishButton = new QPushButton( vSummaryPageWidget, "vSummaryPageFinishButton" );
  vSummaryPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, vSummaryPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  vSummaryPageButtonLayout->addWidget( vSummaryPageFinishButton );
  vSummaryPageLayout->addLayout( vSummaryPageButtonLayout );
  mainWidgetStack->addWidget( vSummaryPageWidget, 3 );
// End: verbose summary page

// The advanced (expert) wording starts here....
// Begin: advance (expert) description page
  eDescriptionPageWidget = new QWidget( mainWidgetStack, "eDescriptionPageWidget" );
  eDescriptionPageLayout = new QVBoxLayout( eDescriptionPageWidget, 11, 6, "eDescriptionPageLayout"); 

  eDescriptionPageTitleLabel = new QLabel( eDescriptionPageWidget, "eDescriptionPageTitleLabel" );
  eDescriptionPageTitleLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, eDescriptionPageTitleLabel->sizePolicy().hasHeightForWidth() ) );
  eDescriptionPageTitleLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
  eDescriptionPageLayout->addWidget( eDescriptionPageTitleLabel );

  eDescriptionPageText = new QLabel( eDescriptionPageWidget, "eDescriptionPageText" );
  eDescriptionPageText->setMinimumSize( QSize(10,10) );
  eDescriptionPageLayout->addWidget( eDescriptionPageText );
  eDescriptionPageButtonLayout = new QHBoxLayout( 0, 0, 6, "eDescriptionPageButtonLayout"); 

  ewizardMode = new QCheckBox( eDescriptionPageWidget, "ewizardMode" );
  ewizardMode->setMinimumSize( QSize(10,10) );
  ewizardMode->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  ewizardMode->setChecked( FALSE );
  eDescriptionPageButtonLayout->addWidget( ewizardMode );


  eDescriptionPageSpacer = new QSpacerItem( 1, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  eDescriptionPageButtonLayout->addItem( eDescriptionPageSpacer );
  eDescriptionPageIntroButton = new QPushButton( eDescriptionPageWidget, "eDescriptionPageIntroButton" );
  eDescriptionPageIntroButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eDescriptionPageIntroButton->sizePolicy().hasHeightForWidth() ) );
  eDescriptionPageButtonLayout->addWidget( eDescriptionPageIntroButton );

  eDescriptionPageNextButton = new QPushButton( eDescriptionPageWidget, "eDescriptionPageNextButton" );
  eDescriptionPageNextButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eDescriptionPageNextButton->sizePolicy().hasHeightForWidth() ) );
  eDescriptionPageButtonLayout->addWidget( eDescriptionPageNextButton );

  eDescriptionPageFinishButton = new QPushButton( eDescriptionPageWidget, "eDescriptionPageFinishButton" );
  eDescriptionPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eDescriptionPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  eDescriptionPageButtonLayout->addWidget( eDescriptionPageFinishButton );

  eDescriptionPageLayout->addLayout( eDescriptionPageButtonLayout );
  mainWidgetStack->addWidget( eDescriptionPageWidget, 4 );
// End: advance (expert) description page

// Begin: advance (expert) parameter page
  eParameterPageWidget = new QWidget( mainWidgetStack, "eParameterPageWidget" );
  eParameterPageWidget->setMinimumSize( QSize(10,10) );
  eParameterPageLayout = new QVBoxLayout( eParameterPageWidget, 11, 6, "eParameterPageLayout"); 

  eParameterPageDescriptionLabel = new QTextEdit( eParameterPageWidget, "eParameterPageDescriptionLabel" );
  eParameterPageDescriptionLabel->setReadOnly(TRUE);
  eParameterPageDescriptionLabel->setMinimumSize( QSize(10,10) );
  eParameterPageDescriptionLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, 0, 0, FALSE ) );
  eParameterPageLayout->addWidget( eParameterPageDescriptionLabel );
  eParameterPageLine = new QFrame( eParameterPageWidget, "eParameterPageLine" );
  eParameterPageLine->setMinimumSize( QSize(10,10) );
  eParameterPageLine->setFrameShape( QFrame::HLine );
  eParameterPageLine->setFrameShadow( QFrame::Sunken );
  eParameterPageLine->setFrameShape( QFrame::HLine );
  eParameterPageLayout->addWidget( eParameterPageLine );

  eParameterPageParameterLayout = new QVBoxLayout( 0, 0, 6, "eParameterPageParameterLayout"); 

  eParameterPageSampleRateHeaderLabel = new QLabel( eParameterPageWidget, "eParameterPageSampleRateHeaderLabel" );
//  eParameterPageSampleRateHeaderLabel->setMinimumSize( QSize(10,10) );
  eParameterPageSampleRateHeaderLabel->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed, 0, 0, FALSE ) );
  eParameterPageParameterLayout->addWidget( eParameterPageSampleRateHeaderLabel );

  eParameterPageSampleRateLayout = new QHBoxLayout( 0, 0, 6, "eParameterPageSampleRateLayout"); 

  eParameterPageSampleRateLabel = new QLabel( eParameterPageWidget, "eParameterPageSampleRateLabel" );
  eParameterPageSampleRateLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  eParameterPageSampleRateLayout->addWidget( eParameterPageSampleRateLabel );

  eParameterPageSampleRateText = new QLineEdit( eParameterPageWidget, "eParameterPageSampleRateText" );
  eParameterPageSampleRateText->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed, 0, 0, FALSE ) );
  eParameterPageSampleRateLayout->addWidget( eParameterPageSampleRateText );

  eParameterPageSpacer = new QSpacerItem( 20, 1, QSizePolicy::Preferred, QSizePolicy::Fixed );
  eParameterPageSampleRateLayout->addItem( eParameterPageSpacer );

  eParameterPageParameterLayout->addLayout( eParameterPageSampleRateLayout );
  eParameterPageLayout->addLayout( eParameterPageParameterLayout );
  eParameterPageButtonLayout = new QHBoxLayout( 0, 0, 6, "eParameterPageButtonLayout"); 

  eParameterPageButtonSpacer = new QSpacerItem( 251, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  eParameterPageButtonLayout->addItem( eParameterPageButtonSpacer );
  eParameterPageBackButton = new QPushButton( eParameterPageWidget, "eParameterPageBackButton" );
  eParameterPageBackButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eParameterPageBackButton->sizePolicy().hasHeightForWidth() ) );
  eParameterPageButtonLayout->addWidget( eParameterPageBackButton );

  eParameterPageResetButton = new QPushButton( eParameterPageWidget, "eParameterPageResetButton" );
  eParameterPageResetButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eParameterPageResetButton->sizePolicy().hasHeightForWidth() ) );
  eParameterPageButtonLayout->addWidget( eParameterPageResetButton );

  eParameterPageNextButton = new QPushButton( eParameterPageWidget, "eParameterPageNextButton" );
  eParameterPageNextButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eParameterPageNextButton->sizePolicy().hasHeightForWidth() ) );
  eParameterPageButtonLayout->addWidget( eParameterPageNextButton );

  eParameterPageFinishButton = new QPushButton( eParameterPageWidget, "eParameterPageFinishButton" );
  eParameterPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eParameterPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  eParameterPageButtonLayout->addWidget( eParameterPageFinishButton );

  eParameterPageLayout->addLayout( eParameterPageButtonLayout );
  mainWidgetStack->addWidget( eParameterPageWidget, 5 );
// End: advanced (exper) parameter page

// Begin: advance (expert) summary page
  eSummaryPageWidget = new QWidget( mainWidgetStack, "eSummaryPageWidget" );
  eSummaryPageWidget->setMinimumSize( QSize(10,10) );
  eSummaryPageLayout = new QVBoxLayout( eSummaryPageWidget, 11, 6, "eSummaryPageLayout"); 

  eSummaryPageFinishLabel = new QTextEdit( eSummaryPageWidget, "eSummaryPageFinishLabel" );
  eSummaryPageFinishLabel->setReadOnly(TRUE);
  eSummaryPageFinishLabel->setMinimumSize( QSize(10,10) );
  eSummaryPageFinishLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, 0, 0, FALSE ) );
  eSummaryPageLayout->addWidget( eSummaryPageFinishLabel );

  eSummaryPageButtonLayout = new QHBoxLayout( 0, 0, 6, "eSummaryPageButtonLayout"); 

  eSummaryPageButtonSpacer = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  eSummaryPageButtonLayout->addItem( eSummaryPageButtonSpacer );
  eSummaryPageBackButton = new QPushButton( eSummaryPageWidget, "eSummaryPageBackButton" );
  eSummaryPageBackButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eSummaryPageBackButton->sizePolicy().hasHeightForWidth() ) );
  eSummaryPageButtonLayout->addWidget( eSummaryPageBackButton );

  eSummaryPageFinishButton = new QPushButton( eSummaryPageWidget, "eSummaryPageFinishButton" );
  eSummaryPageFinishButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, eSummaryPageFinishButton->sizePolicy().hasHeightForWidth() ) );
  eSummaryPageButtonLayout->addWidget( eSummaryPageFinishButton );
  eSummaryPageLayout->addLayout( eSummaryPageButtonLayout );
  mainWidgetStack->addWidget( eSummaryPageWidget, 7 );
  mainFrameLayout->addWidget( mainWidgetStack );
  pcSampleFormLayout->addWidget( mainFrame );
// End: advance (expert) summary page


  languageChange();
  resize( QSize(631, 508).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );

    // signals and slots connections
  connect( eDescriptionPageNextButton, SIGNAL( clicked() ), this,
           SLOT( eDescriptionPageNextButtonSelected() ) );
  connect( eDescriptionPageIntroButton, SIGNAL( clicked() ), this,
           SLOT( eDescriptionPageIntroButtonSelected() ) );
  connect( eParameterPageBackButton, SIGNAL( clicked() ), this,
           SLOT( eParameterPageBackButtonSelected() ) );
  connect( eParameterPageNextButton, SIGNAL( clicked() ), this,
           SLOT( eParameterPageNextButtonSelected() ) );
  connect( eParameterPageResetButton, SIGNAL( clicked() ), this,
           SLOT( eParameterPageResetButtonSelected() ) );
  connect( eParameterPageSampleRateText, SIGNAL( returnPressed() ), this,
           SLOT( eParameterPageSampleRateTextReturnPressed() ) );

  connect( eSummaryPageBackButton, SIGNAL( clicked() ), this,
           SLOT( eSummaryPageBackButtonSelected() ) );
  connect( eSummaryPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( eSummaryPageFinishButtonSelected() ) );
  connect( vDescriptionPageNextButton, SIGNAL( clicked() ), this,
           SLOT( vDescriptionPageNextButtonSelected() ) );
  connect( vDescriptionPageIntroButton, SIGNAL( clicked() ), this,
           SLOT( vDescriptionPageIntroButtonSelected() ) );
  connect( vParameterPageSampleRateText, SIGNAL( returnPressed() ), this,
           SLOT( vParameterPageSampleRateTextReturnPressed() ) );
  connect( vParameterPageBackButton, SIGNAL( clicked() ), this,
           SLOT( vParameterPageBackButtonSelected() ) );
  connect( vParameterPageResetButton, SIGNAL( clicked() ), this,
           SLOT( vParameterPageResetButtonSelected() ) );
  connect( vParameterPageNextButton, SIGNAL( clicked() ), this,
           SLOT( vParameterPageNextButtonSelected() ) );

  connect( vSummaryPageBackButton, SIGNAL( clicked() ), this,
           SLOT( vSummaryPageBackButtonSelected() ) );
  connect( vSummaryPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( vSummaryPageFinishButtonSelected() ) );
  connect( vwizardMode, SIGNAL( clicked() ), this,
           SLOT( vwizardModeSelected() ) );
  connect( ewizardMode, SIGNAL( clicked() ), this,
           SLOT( ewizardModeSelected() ) );

  connect( eDescriptionPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( finishButtonSelected() ) );
  connect( eParameterPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( finishButtonSelected() ) );
  connect( vDescriptionPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( finishButtonSelected() ) );
  connect( vParameterPageFinishButton, SIGNAL( clicked() ), this,
           SLOT( finishButtonSelected() ) );

//
// Setup pcSampleWizard data items
//

// set the load panel associated with this wizard
// to NULL.  The wizard will create a new one along
// the wizard path and then reuse it, if one exists
// at the time it is needed.
  setThisWizardsLoadPanel(NULL);

#if FIX
  if( ao && ao->int_data == 0 )
  {
    // This wizard panel was brought up explicitly.   Don't
    // enable the hook to go back to the IntroWizardPanel.
    vDescriptionPageIntroButton->hide();
    eDescriptionPageIntroButton->hide();
  }
#endif

// This is way ugly and only a temporary hack to get a handle on the 
// parent pcSamplePanel's hook.    
// This should only be > 1 when we're calling this wizard from within
// a pcSamplePanel session to help the user load an executable.
  pcSamplePanel = NULL;
//  if( (int)argument > 1 )
  if( ao && ao->panel_data != NULL )
  {
    pcSamplePanel = (Panel *)ao->panel_data;
  }

// end debug

}


//! Destroys the object and frees any allocated resources
/*! The only thing that needs to be cleaned up is the baseWidgetFrame.
 */
pcSampleWizardPanel::~pcSampleWizardPanel()
{
  nprintf(DEBUG_CONST_DESTRUCT) ("  pcSampleWizardPanel::~pcSampleWizardPanel() destructor called\n");
}

//! Add user panel specific menu items if they have any.
bool
pcSampleWizardPanel::menu(QPopupMenu* contextMenu)
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::menu() requested.\n");

  return( FALSE );
}

//! Save ascii version of this panel.
/*! If the user panel provides save to ascii functionality, their function
     should provide the saving.
 */
void 
pcSampleWizardPanel::save()
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::save() requested.\n");
}

//! Save ascii version of this panel (to a file).
/*! If the user panel provides save to ascii functionality, their function
     should provide the saving.  This callback will invoke a popup prompting
     for a file name.
 */
void 
pcSampleWizardPanel::saveAs()
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::saveAs() requested.\n");
}

//! This function listens for messages.
int 
pcSampleWizardPanel::listener(void *msg)
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::listener() requested.\n");
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::listener() requested., getName()=%s\n", getName());
#endif

  MessageObject *messageObject = (MessageObject *)msg;
  nprintf(DEBUG_PANELS) ("  pcSampleWizardPanel::listener, messageObject->msgType = %s\n", messageObject->msgType.ascii() );
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::listener,  messageObject->msgType = %s\n", messageObject->msgType.ascii() );
#endif
  if( messageObject->msgType == getName() )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    return 1;
  }
  if( messageObject->msgType == "Wizard_Raise_LoadPanel_Back_Page" )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    nprintf(DEBUG_PANELS) ("vDescriptionPageWidget\n");
    if( vwizardMode->isOn() && !ewizardMode->isOn() )
    {// is it verbose?
      mainWidgetStack->raiseWidget(vParameterPageWidget);
    } else {
      mainWidgetStack->raiseWidget(eParameterPageWidget);
    }

    // Hide the load panel as we just came back from it.
    // It may be less confusing to hide it than to leave it up.
    // Another usability study may provide feedback

    QString name = "loadPanel";
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::listener, Wizard_Raise_LoadPanel_Back_Page, try to find panel (%s)\n", name.ascii() );
#endif
//    Panel *loadPanel = getPanelContainer()->findNamedPanel(getPanelContainer()->getMasterPC(), (char *)name.ascii() );
    Panel *loadPanel = getThisWizardsLoadPanel();
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::listener, Wizard_Raise_LoadPanel_Back_Page, loadPanel=0x%x\n", loadPanel);
    if (loadPanel) {
      printf("pcSampleWizardPanel::findAndRaiseLoadPanel, loadPanel->getName()=%s\n", loadPanel->getName() );
    }
#endif
    if( loadPanel ) {
#ifdef DEBUG_PCWizard
      printf("pcSampleWizardPanel::listener, Wizard_Raise_LoadPanel_Back_Page, Found the loadPanel... Try to hide it.\n");
#endif
      loadPanel->getPanelContainer()->hidePanel(loadPanel);
    }

    return 1;
  }
  if( messageObject->msgType == "Wizard_Raise_First_Page" )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    nprintf(DEBUG_PANELS) ("vDescriptionPageWidget\n");
    if( vwizardMode->isOn() && !ewizardMode->isOn() )
    {// is it verbose?
      mainWidgetStack->raiseWidget(vDescriptionPageWidget);
    } else
    {
      mainWidgetStack->raiseWidget(eDescriptionPageWidget);
    }
    return 1;
  }
  if( messageObject->msgType == "Wizard_Raise_Summary_Page" )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::listener, Wizard_Raise_Summary_Page if block \n");
#endif

    vPrepareForSummaryPage();

    return 1;
  }
  return 0;  // 0 means, did not want this message and did not act on anything.
}

#if OLD
//! This function listens for messages.
int 
pcSampleWizardPanel::listener(void *msg)
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::listener() requested.\n");
// printf("pcSampleWizardPanel::listener() requested.\n");

  MessageObject *messageObject = (MessageObject *)msg;
  nprintf(DEBUG_PANELS) ("  messageObject->msgType = %s\n", messageObject->msgType.ascii() );
  if( messageObject->msgType == getName() )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    return 1;
  }
  if( messageObject->msgType == "Wizard_Raise_First_Page" )
  {
    vSummaryPageFinishButton->setEnabled(TRUE);
    eSummaryPageFinishButton->setEnabled(TRUE);
    vSummaryPageBackButton->setEnabled(TRUE);
    eSummaryPageBackButton->setEnabled(TRUE);
    qApp->flushX();
    nprintf(DEBUG_PANELS) ("vDescriptionPageWidget\n");
    if( vwizardMode->isOn() && !ewizardMode->isOn() )
    {// is it verbose?
      mainWidgetStack->raiseWidget(vDescriptionPageWidget);
    } else
    {
      mainWidgetStack->raiseWidget(eDescriptionPageWidget);
    }
    return 1;
  }
  return 0;  // 0 means, did not want this message and did not act on anything.
}

#endif

//! This function broadcasts messages.
int 
pcSampleWizardPanel::broadcast(char *msg)
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::broadcast() requested.\n");
  return 0;
}

void pcSampleWizardPanel::vwizardModeSelected()
{
  QWidget *raisedWidget = mainWidgetStack->visibleWidget();
  wizardModeSelected();
  if( raisedWidget == vDescriptionPageWidget ) {
      nprintf(DEBUG_PANELS) ("vDescriptionPageWidget\n");
      mainWidgetStack->raiseWidget(eDescriptionPageWidget);
  } else if( raisedWidget ==  vParameterPageWidget ) {
      nprintf(DEBUG_PANELS) ("vParameterPageWidget\n");
      mainWidgetStack->raiseWidget(eParameterPageWidget);
  } else if( raisedWidget == vSummaryPageWidget ) {
      nprintf(DEBUG_PANELS) ("vSummaryPageWidget\n");
      mainWidgetStack->raiseWidget(eSummaryPageWidget);
  } else {
      nprintf(DEBUG_PANELS) ("Expert to Verbose: unknown WStackPage\n");
  }
  vwizardMode->setChecked( FALSE );
}

void pcSampleWizardPanel::ewizardModeSelected()
{
  QWidget *raisedWidget = mainWidgetStack->visibleWidget();
  wizardModeSelected();
  if( raisedWidget  == eDescriptionPageWidget ) {
      nprintf(DEBUG_PANELS) ("eDescriptionPageWidget\n");
      mainWidgetStack->raiseWidget(vDescriptionPageWidget);
  } else if( raisedWidget == eParameterPageWidget ) {
      nprintf(DEBUG_PANELS) ("eParameterPageWidget\n");
      mainWidgetStack->raiseWidget(vParameterPageWidget);
  } else if( raisedWidget == eSummaryPageWidget ) {
      nprintf(DEBUG_PANELS) ("eSummaryPageWidget\n");
      mainWidgetStack->raiseWidget(vSummaryPageWidget);
  } else {
      nprintf(DEBUG_PANELS) ("Verbose to Expert: unknown WStackPage\n");
  }
}


void pcSampleWizardPanel::wizardModeSelected()
{
  QWidget *raisedWidget = mainWidgetStack->visibleWidget();
  if( raisedWidget == vDescriptionPageWidget )
  {
    nprintf(DEBUG_PANELS) ("vDescriptionPageWidget\n");
  } else if( raisedWidget ==  vParameterPageWidget )
  {
    nprintf(DEBUG_PANELS) ("vParameterPageWidget\n");
  } else if( raisedWidget == vSummaryPageWidget )
  {
    nprintf(DEBUG_PANELS) ("vSummaryPageWidget\n");
  } else if( raisedWidget  == eDescriptionPageWidget )
  {
    nprintf(DEBUG_PANELS) ("eDescriptionPageWidget\n");
  } else if( raisedWidget == eParameterPageWidget )
  {
    nprintf(DEBUG_PANELS) ("eParameterPageWidget\n");
  } else if( raisedWidget == eSummaryPageWidget )
  {
    nprintf(DEBUG_PANELS) ("eSummaryPageWidget\n");
  }

  ewizardMode->setChecked( FALSE );
}

Panel* pcSampleWizardPanel::findAndRaiseLoadPanel()
{
  // Try to raise the load panel if there is one hidden
#if 1
  Panel *p = getThisWizardsLoadPanel();
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::findAndRaiseLoadPanel, found thisWizardsLoadPanel - now raising, p=%x\n", p);
  if (p) {
    printf("pcSampleWizardPanel::findAndRaiseLoadPanel, p->getName()=%s\n", p->getName() );
  }
#endif
  if (p) {
     p->getPanelContainer()->raisePanel(p);
  } else {
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::findAndRaiseLoadPanel, did not find loadPanel\n");
#endif
  }

#else

  QString name = QString("loadPanel");
  Panel *p = getPanelContainer()->findNamedPanel(getPanelContainer()->getMasterPC(), (char *)name.ascii() );
  if (p) {
#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel::findAndRaiseLoadPanel, found loadPanel - now raising, p=%x\n", p);
     if (p) {
       printf("pcSampleWizardPanel::findAndRaiseLoadPanel, found loadPanel, p->getName()=%s\n", p->getName() );
     }
#endif
     p->getPanelContainer()->raisePanel(p);
  } else {
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::findAndRaiseLoadPanel, did not find loadPanel\n");
#endif
  }
#endif

  return p;
}


void pcSampleWizardPanel::eDescriptionPageNextButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eDescriptionPageNextButtonSelected() \n");
  mainWidgetStack->raiseWidget(eParameterPageWidget);
}

void pcSampleWizardPanel::eDescriptionPageIntroButtonSelected()
{
  getPanelContainer()->hidePanel((Panel *)this);

  nprintf(DEBUG_PANELS) ("eDescriptionPageIntroButtonSelected() \n");

  Panel *p = getPanelContainer()->raiseNamedPanel("Intro Wizard");
  if( !p )
  {
    getPanelContainer()->getMasterPC()->dl_create_and_add_panel("Intro Wizard", getPanelContainer(), NULL );
  }
}

void pcSampleWizardPanel::eParameterPageBackButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eParameterPageBackButtonSelected() \n");
  mainWidgetStack->raiseWidget(eDescriptionPageWidget);
}

void pcSampleWizardPanel::eParameterPageNextButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eParameterPageNextButtonSelected() \n");

  sampleRate = eParameterPageSampleRateText->text();
  vParameterPageSampleRateText->setText(eParameterPageSampleRateText->text());

 // See if loadPanel already exists - if user used the back button
 // we may have hidden the panel and now just need to raise it instead
 // of creating a new one.

#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::eParameterPageNextButtonSelected, calling findAndRaiseLoadPanel()\n");
#endif
  Panel *p = findAndRaiseLoadPanel();

  if (p) {
    // raise the first page of the load panel
    MessageObject *msg = new MessageObject("Wizard_Raise_First_Page");
    p->listener((void *)msg);
    delete msg;

  } else {

#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel, find and raise load panel failed, calling loadNewProgramPanel()\n");
#endif
    OpenSpeedshop *mw = getPanelContainer()->getMainWindow();
    if( mw ) {
     mw->executableName = QString((const char *)0);
     mw->argsStr = QString((const char *)0);
     mw->parallelPrefixCommandStr = QString((const char *)0);

#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel calling mw->loadNewProgramPanel, this=0x%x\n", this );
#endif

     Panel* p = mw->loadNewProgramPanel(getPanelContainer(), getPanelContainer()->getMasterPC(), /* expID */-1, (Panel *) this);
     setThisWizardsLoadPanel(p);
#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel::eParameterPageNextButtonSelected, after calling mw->loadNewProgramPanel, p=0x%x\n", p );
     if (p) {
       printf("pcSampleWizardPanel::eParameterPageNextButtonSelected, p->getName()=%s\n", p->getName() );
     }
#endif

     QString executableNameStr = mw->executableName;
     if( !mw->executableName.isEmpty() ) {
#ifdef DEBUG_PCWizard
      printf("pcSampleWizardPanel, executableName=%s\n", mw->executableName.ascii() );
#endif
     } else {
#ifdef DEBUG_PCWizard
      printf("pcSampleWizardPanel, executableName is empty\n" );
#endif
     } // end if clause for empty executable name
    } // end if clause for mw
  } // end else clause for create new load panel
//jeg  mainWidgetStack->raiseWidget(vSummaryPageWidget);
}

void pcSampleWizardPanel::eParameterPageResetButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eParameterPageResetButtonSelected() \n");
  vParameterPageSampleRateText->setText(QString("%1").arg(original_sampling_rate));
  eParameterPageSampleRateText->setText(QString("%1").arg(original_sampling_rate));
}

void pcSampleWizardPanel::eSummaryPageBackButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eSummaryPageBackButtonSelected() \n");

// JUST RAISE EXISTING PANEL if one is there.

 // See if loadPanel already exists - if user used the back button
 // we may have hidden the panel and now just need to raise it instead
 // of creating a new one.

#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::eSummaryPageBackButtonSelected, calling findAndRaiseLoadPanel()\n");
#endif
  Panel *p = findAndRaiseLoadPanel();

  if (p) {
    // raise the second page of the load panel
    MessageObject *msg = new MessageObject("Wizard_Raise_Second_Page");
    p->listener((void *)msg);
    delete msg;
  } else {
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::eSummaryPageBackButtonSelected, did not find loadPanel\n");
#endif
    p = getPanelContainer()->getMasterPC()->dl_create_and_add_panel("loadPanel", getPanelContainer(), NULL);
    if (p) {
#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel::eSummaryPageBackButtonSelected, found loadPanel, p=%x\n", p);
#endif
     MessageObject *msg = new MessageObject("Wizard_Raise_Second_Page");
     p->listener((void *)msg);
     delete msg;
    }
  }
}


void pcSampleWizardPanel::eSummaryPageFinishButtonSelected()
{
  nprintf(DEBUG_PANELS) ("eSummaryPageFinishButtonSelected() \n");

  vSummaryPageFinishButtonSelected();

}

void pcSampleWizardPanel::vDescriptionPageNextButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vDescriptionPageNextButtonSelected() \n");


  mainWidgetStack->raiseWidget(vParameterPageWidget);
}

void pcSampleWizardPanel::vDescriptionPageIntroButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vDescriptionPageIntroButtonSelected() \n");

  getPanelContainer()->hidePanel((Panel *)this);

  Panel *p = getPanelContainer()->raiseNamedPanel("Intro Wizard");
  if( !p )
  {
    nprintf(DEBUG_PANELS) ("vDescriptionPageIntroButtonSelected() create a new one!\n");
    getPanelContainer()->getMasterPC()->dl_create_and_add_panel("Intro Wizard", getPanelContainer(), NULL);
  }
}

void pcSampleWizardPanel::vParameterPageSampleRateTextReturnPressed()
{
  nprintf(DEBUG_PANELS) ("vParameterPageSampleRateTextReturnPressed() \n");
}

void pcSampleWizardPanel::eParameterPageSampleRateTextReturnPressed()
{
  nprintf(DEBUG_PANELS) ("eParameterPageSampleRateTextReturnPressed() \n");
}

void pcSampleWizardPanel::vParameterPageBackButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vParameterPageBackButtonSelected() \n");

  mainWidgetStack->raiseWidget(vDescriptionPageWidget);
}

void pcSampleWizardPanel::vParameterPageNextButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vParameterPageNextButtonSelected() \n");

  sampleRate = vParameterPageSampleRateText->text();
  eParameterPageSampleRateText->setText(vParameterPageSampleRateText->text());

 // See if loadPanel already exists - if user used the back button
 // we may have hidden the panel and now just need to raise it instead
 // of creating a new one.

#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::vParameterPageNextButtonSelected, calling findAndRaiseLoadPanel()\n");
#endif
  Panel *p = findAndRaiseLoadPanel();

  if (p) {
    // raise the first page of the load panel
    MessageObject *msg = new MessageObject("Wizard_Raise_First_Page");
    p->listener((void *)msg);
    delete msg;
  } else {

#ifdef DEBUG_PCWizard
   printf("pcSampleWizardPanel, calling loadNewProgramPanel()\n");
#endif
   OpenSpeedshop *mw = getPanelContainer()->getMainWindow();
   if( mw ) {
    mw->executableName = QString((const char *)0);
    mw->argsStr = QString((const char *)0);
    mw->parallelPrefixCommandStr = QString((const char *)0);

#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel calling mw->loadNewProgramPanel, this=0x%x\n", this );
#endif

    Panel* p = mw->loadNewProgramPanel(getPanelContainer(), getPanelContainer()->getMasterPC(), /* expID */-1, (Panel *) this);
    setThisWizardsLoadPanel(p);
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::vParameterPageNextButtonSelected, after calling mw->loadNewProgramPanel, p=0x%x\n", p );
    if (p) {
      printf("pcSampleWizardPanel::vParameterPageNextButtonSelected, p->getName()=%s\n", p->getName() );
    }
#endif

    QString executableNameStr = mw->executableName;
    if( !mw->executableName.isEmpty() ) {
#ifdef DEBUG_PCWizard
      printf("pcSampleWizardPanel, executableName=%s\n", mw->executableName.ascii() );
#endif
    } else {
#ifdef DEBUG_PCWizard
      printf("pcSampleWizardPanel, executableName is empty\n" );
#endif
    }
   }
  }
//jeg  mainWidgetStack->raiseWidget(vSummaryPageWidget);
}

void pcSampleWizardPanel::vParameterPageResetButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vParameterPageResetButtonSelected() \n");
  vParameterPageSampleRateText->setText(QString("%1").arg(original_sampling_rate));
  eParameterPageSampleRateText->setText(QString("%1").arg(original_sampling_rate));
}

void pcSampleWizardPanel::vPrepareForSummaryPage()
{
  nprintf(DEBUG_PANELS) ("pcSampleWizardPanel::vPrepareForSummaryPage() \n");
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::vPrepareForSummaryPage() \n");
#endif

  char buffer[2048];

  OpenSpeedshop *mw = getPanelContainer()->getMainWindow();
  if( !mw )
  {
    return;
  } 

  if( !mw->pidStr.isEmpty() )
  {
    QString host_name = mw->pidStr.section(' ', 0, 0, QString::SectionSkipEmpty);
    QString pid_name = mw->pidStr.section(' ', 1, 1, QString::SectionSkipEmpty);
    QString prog_name = mw->pidStr.section(' ', 2, 2, QString::SectionSkipEmpty);
    sprintf(buffer, "<p align=\"left\">You've selected a pc Sample experiment for process \"%s\" running on host \"%s\".<br>Furthermore, you've chosen a sampling rate of \"%s\".<br><br>To complete the experiment setup select the \"Finish\" button.<br><br>After selecting the \"Finish\" button an experiment \"pcSample\" panel will be raised to allow you to futher control the experiment, or click the \"RUN\" button to run the experiment as created by the wizard process.<br><br>Press the \"Back\" button to go back to the previous page.</p>", mw->pidStr.ascii(), mw->hostStr.ascii(), vParameterPageSampleRateText->text().ascii() );
  } else if( !mw->executableName.isEmpty() ) {
    QString host_name = mw->pidStr.section(' ', 0, 0, QString::SectionSkipEmpty);
    QString pid_name = mw->pidStr.section(' ', 1, 1, QString::SectionSkipEmpty);
    QString prog_name = mw->pidStr.section(' ', 2, 2, QString::SectionSkipEmpty);
    if( mw->parallelPrefixCommandStr.isEmpty() || mw->parallelPrefixCommandStr.isNull() ) {
        sprintf(buffer, "<p align=\"left\">You've selected a pc Sample experiment for command/executable \"%s\" to be run on host \"%s\".<br>Furthermore, you've chosen a sampling rate of \"%s\".<br><br>To complete the experiment setup select the \"Finish\" button.<br><br>After selecting the \"Finish\" button an experiment \"pcSample\" panel will be raised to allow you to futher control the experiment, or click the \"RUN\" button to run the experiment as created by the wizard process.<br><br>Press the \"Back\" button to go back to the previous page.</p>", mw->executableName.ascii(), mw->hostStr.ascii(), vParameterPageSampleRateText->text().ascii() );
    } else {
        sprintf(buffer, "<p align=\"left\">You've selected a pc Sample experiment for command/executable <br>\"%s %s\" to be run on host<br>\"%s\".<br>Furthermore, you've chosen a sampling rate of \"%s\".<br><br>To complete the experiment setup select the \"Finish\" button.<br><br>After selecting the \"Finish\" button an experiment \"pcSample\" panel will be raised to allow you to futher control the experiment, or click the \"RUN\" button to run the experiment as created by the wizard process.<br><br>Press the \"Back\" button to go back to the previous page.</p>", mw->parallelPrefixCommandStr.ascii(), mw->executableName.ascii(), mw->hostStr.ascii(), vParameterPageSampleRateText->text().ascii() );
    }
  }

  vSummaryPageFinishLabel->setText( tr( buffer ) );
  mainWidgetStack->raiseWidget(2);
  mainWidgetStack->raiseWidget(vSummaryPageWidget);

  QString name = "loadPanel";
#ifdef DEBUG_PCWizard
  printf("try to find panel (%s)\n", name.ascii() );
#endif
//  Panel *loadPanel = getPanelContainer()->findNamedPanel(getPanelContainer()->getMasterPC(), (char *)name.ascii() );
  Panel *loadPanel = getThisWizardsLoadPanel();
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::vPrepareForSummaryPage(), loadPanel=0x%x\n", loadPanel);
#endif
  if( loadPanel ) {
#ifdef DEBUG_PCWizard
     printf("Found the loadPanel... Try to hide it.\n");
#endif
     loadPanel->getPanelContainer()->hidePanel(loadPanel);
  }

}

// End verbose AttachOrLoad callbacks


void pcSampleWizardPanel::vSummaryPageBackButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vSummaryPageBackButtonSelected() \n");
#ifdef DEBUG_PCWizard
  printf("vSummaryPageBackButtonSelected() \n");
#endif

// RAISE EXISTING PANEL if one is there and request the second page of the wizard
// be focused/raised.

  Panel *p = findAndRaiseLoadPanel();

  if (p) {

#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel::vSummaryPageBackButtonSelected, found loadPanel, p=%x\n", p);
#endif

    // raise the second page of the load panel
    MessageObject *msg = new MessageObject("Wizard_Raise_Second_Page");
    p->listener((void *)msg);
    delete msg;
  } else {
#ifdef DEBUG_PCWizard
    printf("pcSampleWizardPanel::vSummaryPageBackButtonSelected, did not find loadPanel\n");
#endif
    p = getPanelContainer()->getMasterPC()->dl_create_and_add_panel("loadPanel", getPanelContainer(), NULL);
    if (p) {
#ifdef DEBUG_PCWizard
     printf("pcSampleWizardPanel::vSummaryPageBackButtonSelected, found loadPanel, p=%x\n", p);
#endif
     MessageObject *msg = new MessageObject("Wizard_Raise_Second_Page");
     p->listener((void *)msg);
     delete msg;
    }
  }
}

void pcSampleWizardPanel::finishButtonSelected()
{
  nprintf(DEBUG_PANELS) ("finishButtonSelected() \n");
 
#ifdef DEBUG_PCWizard
  printf("finishButtonSelected() \n");
#endif

  OpenSpeedshop *mw = getPanelContainer()->getMainWindow();
  if( mw->executableName.isEmpty() && mw->pidStr.isEmpty() )
  {
#ifdef DEBUG_PCWizard
    printf("finishButtonSelected(),vwizardMode->isOn()=%d\n",vwizardMode->isOn() );
#endif
    if( vwizardMode->isOn() ) {
      vSummaryPageFinishButtonSelected();
    } else {
      vSummaryPageFinishButtonSelected();
    }
  } else {
    
#ifdef DEBUG_PCWizard
    printf("finishButtonSelected(),else clause call vSummaryPageFinishButtonSelected()\n" );
#endif
    vSummaryPageFinishButtonSelected();
  }
}


void pcSampleWizardPanel::vSummaryPageFinishButtonSelected()
{
  nprintf(DEBUG_PANELS) ("vSummaryPageFinishButtonSelected() \n");
#ifdef DEBUG_PCWizard
  printf("vSummaryPageFinishButtonSelected() \n");
#endif

  Panel *p = pcSamplePanel;
  if( getPanelContainer()->getMainWindow() ) { 

    OpenSpeedshop *mw = getPanelContainer()->getMainWindow();

    if( mw ) {

      LoadAttachObject *lao = NULL;
      ParamList *paramList = new ParamList();
      paramList->push_back(vParameterPageSampleRateText->text() );
// printf("A: push_back (%s)\n", vParameterPageSampleRateText->text().ascii() );

      if( !mw->executableName.isEmpty() ) {

//printf("executable name was specified.\n");

        lao = new LoadAttachObject(mw->executableName, (char *)NULL, mw->parallelPrefixCommandStr, paramList, TRUE);

      } else if( !mw->pidStr.isEmpty() ) {

// printf("pid was specified.\n");
        lao = new LoadAttachObject((char *)NULL, mw->pidStr, (char *)NULL, paramList, TRUE);

      } else {

// printf("Warning: No attach or load paramaters available.\n");

      }

      if( lao != NULL ) {
        vSummaryPageFinishButton->setEnabled(FALSE);
        eSummaryPageFinishButton->setEnabled(FALSE);
        vSummaryPageBackButton->setEnabled(FALSE);
        eSummaryPageBackButton->setEnabled(FALSE);
        qApp->flushX();

        if( !p ) {

          ArgumentObject *ao = new ArgumentObject("ArgumentObject", -1 );
          ao->lao = lao;
          p = getPanelContainer()->getMasterPC()->dl_create_and_add_panel("pc Sampling", getPanelContainer(), ao);
          delete ao;

        } else { 
          p->listener((void *)lao);

        }

//        getPanelContainer()->hidePanel((Panel *)this);
// The receiving routine should delete this...
// delete paramList;
      } // (lao != NULL)
    } // (mw) 
  } // (getPanelContainer)

}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void
pcSampleWizardPanel::languageChange()
{
  unsigned int sampling_rate = 100;
#ifdef DEBUG_PCWizard
  printf("pcSampleWizardPanel::languageChange() entered\n");
#endif

  setCaption( tr( "pc Sample Wizard Panel" ) );
  vDescriptionPageTitleLabel->setText( tr( "<h1>pc Sample Wizard</h1>" ) );
  vDescriptionPageText->setText( tr( vpcSampleDescription ) );
  vDescriptionPageIntroButton->setText( tr( "< Back" ) );
  QToolTip::add( vDescriptionPageIntroButton, tr( "Takes you back to the second page of the Intro Wizard so you can make a different selection." ) );
  vDescriptionPageNextButton->setText( tr( "> Next" ) );
  QToolTip::add( vDescriptionPageNextButton, tr( "Advance to the next wizard page." ) );
  vDescriptionPageFinishButton->setText( tr( ">> Finish" ) );
  QToolTip::add( vDescriptionPageFinishButton, tr( "Not active, present in this page for consistent use of buttons." ) );
  vDescriptionPageFinishButton->setEnabled(FALSE);

  vParameterPageDescriptionText->setText( tr( QString("The following options (paramaters) are available to adjust.   These are the options the collector has exported.<br><br>\n"
"The larger the number used for the sample rate, the more\n"
"pc Sample detail will be shown.   However, the trade off will be slower\n"
"performance and a larger data file.<br><br>\n"
"It may take a little experimenting to find the right setting for your \n"
"particular executable.   We suggest starting with the default setting\n"
"of %1.").arg(sampling_rate) ) );
  vParameterPageSampleRateHeaderLabel->setText( tr( "You can set the following option(s):" ) );
  vParameterPageSampleRateLabel->setText( tr( "pc Sample rate (in samples/second):" ) );
  vParameterPageSampleRateText->setText( tr( QString("%1").arg(sampling_rate) ) );
  QToolTip::add( vParameterPageSampleRateText, tr( QString("The number of times to sample per second.   (Default %1.)").arg(sampling_rate) ) );
  vParameterPageBackButton->setText( tr( "< Back" ) );
  QToolTip::add( vParameterPageBackButton, tr( "Takes you back one page." ) );
  vParameterPageResetButton->setText( tr( "Reset" ) );
  QToolTip::add( vParameterPageResetButton, tr( "Reset the values to the default setings." ) );
  vParameterPageNextButton->setText( tr( "> Next" ) );
  QToolTip::add( vParameterPageNextButton, tr( "Advance to the next wizard page." ) );
  vParameterPageFinishButton->setText( tr( ">> Finish" ) );
  QToolTip::add( vParameterPageFinishButton, tr( "Not active, present in this page for consistent use of buttons." ) );
  vParameterPageFinishButton->setEnabled(FALSE);

  vSummaryPageFinishLabel->setText( tr( "No summary available.\n" ) );

  vSummaryPageBackButton->setText( tr( "< Back" ) );
  QToolTip::add( vSummaryPageBackButton, tr( "Takes you back one page." ) );
  vSummaryPageFinishButton->setText( tr( "Finish..." ) );
  QToolTip::add( vSummaryPageFinishButton, tr( "Finishes loading the wizard information and brings up a \"pcSample\" panel" ) );
  eDescriptionPageTitleLabel->setText( tr( "<h1>pc Sample Wizard</h1>" ) );
  eDescriptionPageText->setText( tr( epcSampleDescription ) );
  eDescriptionPageIntroButton->setText( tr( "< Back" ) );
  eDescriptionPageNextButton->setText( tr( "> Next" ) );
  QToolTip::add( eDescriptionPageNextButton, tr( "Advance to the next wizard page." ) );
  eDescriptionPageFinishButton->setText( tr( ">> Finish" ) );
  QToolTip::add( eDescriptionPageFinishButton, tr( "Not active, present in this page for consistent use of buttons." ) );
  eDescriptionPageFinishButton->setEnabled(FALSE);

  eParameterPageDescriptionLabel->setText( tr( "The following options (paramaters) are available to adjust.     <br>These are the options the collector has exported." ) );
  eParameterPageSampleRateHeaderLabel->setText( tr( "You can set the following option(s):" ) );
  eParameterPageSampleRateLabel->setText( tr( "pc Sample rate (in samples/second):" ) );
  eParameterPageSampleRateText->setText( tr( QString("%1").arg(sampling_rate) ) );
  QToolTip::add( eParameterPageSampleRateText, tr( QString("The number of times to sample per second.   (Default %1.)").arg(sampling_rate) ) );
  eParameterPageBackButton->setText( tr( "< Back" ) );
  QToolTip::add( eParameterPageBackButton, tr( "Takes you back one page." ) );
  eParameterPageResetButton->setText( tr( "Reset" ) );
  QToolTip::add( eParameterPageResetButton, tr( "Reset the values to the default setings." ) );
  eParameterPageNextButton->setText( tr( "> Next" ) );
  QToolTip::add( eParameterPageNextButton, tr( "Advance to the next wizard page." ) );
  eParameterPageFinishButton->setText( tr( ">> Finish" ) );
  QToolTip::add( eParameterPageFinishButton, tr( "Not active, present in this page for consistent use of buttons." ) );
  eParameterPageFinishButton->setEnabled(FALSE);

  eSummaryPageFinishLabel->setText( tr( "No summary yet available.") );
  eSummaryPageBackButton->setText( tr( "< Back" ) );
  QToolTip::add( eSummaryPageBackButton, tr( "Takes you back one page." ) );
  eSummaryPageFinishButton->setText( tr( "Finish..." ) );
  QToolTip::add( eSummaryPageFinishButton, tr( "Finishes loading the wizard information and brings up a \"pc Sample\" panel" ) );
  vwizardMode->setText( tr( "Verbose Wizard Mode" ) );
  ewizardMode->setText( tr( "Verbose Wizard Mode" ) );

  // Look up default metrics.   There's only one in this case.
  // Get list of all the collectors from the FrameWork.
  // To do this, we need to create a dummy experiment.
  try {
    char *temp_name = tmpnam(NULL);
//    static std::string tmpdb = std::string(temp_name);
    std::string tmpdb = std::string(temp_name);
    OpenSpeedShop::Framework::Experiment::create (tmpdb);
    OpenSpeedShop::Framework::Experiment dummy_experiment(tmpdb);

    // Is there a pcsamp experiment type?
    bool found_one = FALSE;
    std::set<Metadata> collectortypes = Collector::getAvailable();
    for( std::set<Metadata>::const_iterator mi = collectortypes.begin();
         mi != collectortypes.end(); mi++ )
    {
      if( mi->getUniqueId() == "pcsamp" )
      {
        found_one = TRUE;
      }
    }
    if( found_one == FALSE )
    {
      return;
    }

    Collector pcSampleCollector = dummy_experiment.createCollector("pcsamp");

    Metadata cm = pcSampleCollector.getMetadata();
      std::set<Metadata> md =pcSampleCollector.getParameters();
      std::set<Metadata>::const_iterator mi;
      for (mi = md.begin(); mi != md.end(); mi++) {
        Metadata m = *mi;
//        printf("%s::%s\n", cm.getUniqueId().c_str(), m.getUniqueId().c_str() );
//        printf("%s::%s\n", cm.getShortName().c_str(), m.getShortName().c_str() );
//        printf("%s::%s\n", cm.getDescription().c_str(), m.getDescription().c_str() );
      }
      pcSampleCollector.getParameterValue("sampling_rate", sampling_rate);
      original_sampling_rate = sampling_rate;
// printf("sampling_rate=%d\n", sampling_rate);
//    pcSampleCollector.setParameterValue("sampling_rate", (unsigned)100);
// printf("Initialize the text fields... (%d)\n", sampling_rate);
    vParameterPageSampleRateText->setText(QString("%1").arg(sampling_rate));
    eParameterPageSampleRateText->setText(QString("%1").arg(sampling_rate));

    if( temp_name )
    {
      (void) remove( temp_name );
    }

  }
  catch(const std::exception& error)
  {
    return;
  }
}

