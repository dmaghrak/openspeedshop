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
  

#include <stdio.h> 
#include <unistd.h>

#include <qapplication.h>
#include "openspeedshop.hxx"

#include <qvaluelist.h>

#include "PanelContainer.hxx"

QApplication *qapplication;

#include <qeventloop.h>
#include <qlineedit.h>
QEventLoop *qeventloop;

#include "splash.xpm"
#include <qsplashscreen.h>
#include <qcheckbox.h>

#include <pthread.h>
#define PTMAX 10
pthread_t phandle[PTMAX];
#include "ArgClass.hxx"


extern "C" 
{
  // This routine starts another QApplication gui.  It is called from 
  // gui_init to have the gui started in it's own thread.
  void
  guithreadinit(void *ptr)
  {
    ArgStruct *arg_struct = NULL;
    int argc = 0;
    char **argv = NULL;
    if( ptr != NULL )
    {
      arg_struct = (ArgStruct *)ptr;
      argc = arg_struct->argc;
      argv = arg_struct->argv;
    }
    bool splashFLAG=TRUE;
  
// printf("guithreadinit() \n");
    QString fontname = QString::null;
    for(int i=0;i<argc;i++)
    {
// printf("  argv[%d]=(%s)\n", i, argv[i] );
      QString arg = argv[i];
      if( arg == "-fn" || arg == "--fn" )
      {
        fontname = argv[i+1];
        break;
      }
    }

    qapplication = new QApplication( argc, argv );

    if( argv == NULL )
    {
      argc = 0;
    }

    QString hostStr = QString::null;
    QString executableStr = QString::null;
    QString widStr = QString::null;
    QString argsStr = QString::null;
    QString pidStr = QString::null;
    QString rankStr = QString::null;
    QString expStr = QString::null;
    for(int i=0;i<argc;i++)
    {
      QString arg = argv[i];
      if( arg == "-wid" )
      { // You have a window id from the cli
        for( ;i<argc;)
        {
          widStr += QString(argv[++i]);
          widStr += QString(" ");
        }
      }

    }

    OpenSpeedshop *w;

    w = new OpenSpeedshop(widStr.toInt());

    QPixmap *splash_pixmap = NULL;
    QSplashScreen *splash = NULL;
    if( !w->preferencesDialog->setShowSplashScreenCheckBox->isChecked() )
    {
      splashFLAG = FALSE;
    } 
    if( splashFLAG )
    {
      splash_pixmap = new QPixmap( splash_xpm );
      splash = new QSplashScreen( *splash_pixmap );
      splash->setCaption("splash");
      splash->show();
      splash->message( "Loading plugins" );
      splash->raise();
    }

    // Set the font from the preferences...
    QFont *m_font = NULL;
    if( w->preferencesDialog->preferencesAvailable == TRUE )
    {
      m_font = new QFont( w->preferencesDialog->globalFontFamily,
                      w->preferencesDialog->globalFontPointSize,
                      w->preferencesDialog->globalFontWeight,
                      w->preferencesDialog->globalFontItalic );
    }

    if( m_font != NULL )
    {
      qApp->setFont(*m_font, TRUE);

      delete m_font;
    }


    w->executableName = executableStr;
    w->widStr = widStr;
    w->pidStr = pidStr;
    w->rankStr = rankStr;
    w->expStr = expStr;
    w->hostStr = hostStr;
    w->argsStr = argsStr;

    w->show();

    qapplication->connect( qapplication, SIGNAL( lastWindowClosed() ), qapplication, SLOT( quit() ) );

    if( splashFLAG )
    {
      splash->raise();
      sleep(1);
      splash->raise();
      splash->message( "Plugins loaded..." );
      sleep(1);
      splash->raise();
      sleep(1);
      splash->raise();
      splash->finish(w);
      delete splash;
      delete splash_pixmap;
    }

    qapplication->exec();

    return;
  }

  // This is simple the routine that is called to start the gui it's own 
  // thread.   Called from Start.cxx, when the gui is requested at startup.
  // Otherwise, it can be called from the command line at any time to 
  // initialize the gui.
  int
  gui_init( void *arg_struct )
  {
      int stat = pthread_create(&phandle[0], 0, (void *(*)(void *))guithreadinit,arg_struct);

      return 1;
  }

}
