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


#ifndef SPLISTVIEW_H
#define SPLISTVIEW_H

class StatsPanelBase;

#include "qlistview.h"
#include <qpopupmenu.h>

#include <qwidget.h>

class SPListView : public QListView
{
public:
  SPListView( StatsPanelBase *rp, QWidget *w, const char *name=0, int flags=0 );

  ~SPListView( );


  StatsPanelBase *resultsPanel;

  void contentsContextMenuEvent( QContextMenuEvent *e );
  QPopupMenu* createPopupMenu( const QPoint & pos );
  QPopupMenu* createPopupMenu( ) { /* obsoleted function. */ return NULL; };

public slots:

private:
};
#endif // SPLISTVIEW_H
